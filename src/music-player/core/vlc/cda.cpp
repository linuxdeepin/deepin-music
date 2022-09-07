// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cda.h"
#include <vlc/vlc.h>
#include <vlc_common.h>
#include <vlc_variables.h>
#include <vlc_access.h>
#include <vlc_stream.h>
#include <vlc_input_item.h>
#include <libvlc_media.h>
#include <vlc_playlist.h>
#include <vlc_interface.h>
#include <vlc_input.h>
#include <libvlc.h>
#include <vlc_plugin.h>
#include <vlc_modules.h>

#include <QDebug>
#include <QDir>
#include <QScopedPointer>
#include <QMap>
#include <QTimer>

#include "ddiskmanager.h"
#include "dblockdevice.h"

#include "vlc/vlcdynamicinstance.h"
#include "util/dbusutils.h"

typedef input_item_t *(*input_item_NewExt_func)(const char *,
                                                const char *,
                                                mtime_t, int,
                                                enum input_item_net_type);
typedef stream_t *(*vlc_stream_NewURL_func)(vlc_object_t *, const char *);
typedef input_item_node_t *(*input_item_node_Create_func)(input_item_t *);
typedef void (*input_item_Release_func)(input_item_t *);
typedef int (*vlc_stream_ReadDir_func)(stream_t *, input_item_node_t *);
typedef void (*input_item_node_Delete_func)(input_item_node_t *);
typedef void (*vlc_stream_Delete_func)(stream_t *);

static input_item_t *inputItem = nullptr;
static stream_t *pStream = nullptr;

static void inputItemNewExtFc(const QString &strcda)
{
    inputItem = nullptr;
    input_item_NewExt_func input_item_NewExt_fc = (input_item_NewExt_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("input_item_NewExt");
    inputItem = input_item_NewExt_fc(strcda.toUtf8().data(), "access_demux", 0, ITEM_TYPE_DISC, ITEM_LOCAL);
}

static void vlc_stream_NewURL_fc(libvlc_media_player_t *mediaPlayer, const QString &strcda)
{
    pStream = nullptr;
    vlc_stream_NewURL_func vlc_stream_NewURL_fc = (vlc_stream_NewURL_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("vlc_stream_NewURL");
    pStream = vlc_stream_NewURL_fc((vlc_object_t *)mediaPlayer, strcda.toUtf8().data()); //打开CD，读取流，该操作较耗时
}

QStringList getCDADirectory()
{
    return QStringList() << "cdda:///dev/sr0"; //暂时只考虑sr0,原装光驱
}

QString queryIdTypeFormDbus()
{
    QVariant vartype = DBusUtils::readDBusProperty("org.freedesktop.UDisks2",
                                                   "/org/freedesktop/UDisks2/block_devices/sr0",
                                                   "org.freedesktop.UDisks2.Block",
                                                   "IdType",
                                                   QDBusConnection::systemBus());
    return vartype.isValid() ? vartype.toString() : "";
}

CdaThread::CdaThread(QObject *parent) : QThread(parent), m_cdaStat(CDROM_INVALID)
{

}

QList<MediaMeta> CdaThread::getCdaMetaInfo()
{
    return m_mediaList;
}

void CdaThread::doQuery()
{
    start();
}

input_item_node_t *CdaThread::getInputNode()
{
    input_item_node_Create_func input_item_node_Create_fc = (input_item_node_Create_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("input_item_node_Create");
    input_item_Release_func input_item_Release_fc = (input_item_Release_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("input_item_Release");
    vlc_stream_ReadDir_func vlc_stream_ReadDir_fc = (vlc_stream_ReadDir_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("vlc_stream_ReadDir");
    vlc_stream_Delete_func vlc_stream_Delete_fc = (vlc_stream_Delete_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("vlc_stream_Delete");

    input_item_node_t *p_items = nullptr;
    QStringList strcdalist = getCDADirectory();

    if (strcdalist.isEmpty())
        return p_items;

    QString strcda = strcdalist.at(0);
    inputItemNewExtFc(strcda);
    if (!inputItem) {
        qDebug() << "no cd driver?";
        return p_items;
    }

    Q_ASSERT(m_play_t);

    vlc_stream_NewURL_fc(m_play_t, strcda);
    if (!pStream) {
        qDebug() << "create stream failed";
        return p_items;
    }
    p_items = input_item_node_Create_fc(inputItem);
    input_item_Release_fc(inputItem);
    int ret = vlc_stream_ReadDir_fc(pStream, p_items);//读取CD中的节点信息
    qDebug() << __FUNCTION__ << ":vlc_stream_ReadDir result:" << ret;
    //释放stream流
    vlc_stream_Delete_fc(pStream);
    return p_items;
}

QString CdaThread::GetCdRomString()
{
    QString strcda = "sr0"; //cdrom关键字
    QStringList blDevList = DDiskManager::blockDevices(QVariantMap());//QVariantMap()为默认的参数，无需特定设置
    //find sr0
    foreach (QString tmpstr, blDevList) {
        QString strdev = tmpstr.mid(tmpstr.lastIndexOf("/") + 1, tmpstr.size() - tmpstr.lastIndexOf("/"));
        if (strdev.compare(strcda) == 0) {
            return tmpstr;
        }
    }
    return QString();
}

void CdaThread::setCdaState(CdaThread::CdromState stat)
{
    if (stat != CDROM_MOUNT_WITH_CD) {
        stat = CDROM_INVALID;
    }

    if (m_cdaStat == stat) {
        // 线程中做休眠
        QThread::sleep(1); //状态一致时，统一休眠
        return;
    }
    qDebug() << __FUNCTION__ << "cda state changed:" << stat;
    m_cdaStat = stat;
    /**
     * 状态更改后再发送cda状态
     **/
    emit sigSendCdaStatus(m_cdaStat);
    /**
     * 非CDROM_MOUNT_WITH_CD清空缓存
     **/
    if (stat != CDROM_MOUNT_WITH_CD) {
        m_mediaList.clear();
    }
}

void CdaThread::run()
{
    while (m_needRun) {
        QString strcdrom = GetCdRomString();
        /**
         * 没有cdrom，说明光驱已移出或损坏
         * */
        if (strcdrom.isEmpty()) {
            setCdaState(CDROM_INVALID);
            continue;
        }

        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(strcdrom));
        DBlockDevice *pblk = blk.data();
        if (!pblk) {
            continue;
        }

        qulonglong blocksize = pblk->size();
        /**
         * 过滤空的光盘和文件类型为udf、iso9660格式的光盘
         **/
        if (blocksize == 0
                || pblk->fsType() == DBlockDevice::iso9660
                || queryIdTypeFormDbus().toLower() == "udf") {
            setCdaState(CDROM_MOUNT_WITHOUT_CD);
            continue;
        }

        /**
         * 当状态不一致时，现在去读取节点信息并发送相关信号
         * */
        if (m_cdaStat != CDROM_MOUNT_WITH_CD) {
            /**
             * 当blocksize有效时，可以开始读取cda节点名
             * */
            input_item_node_t *p_items = getInputNode();
            /**
             * 防止线程正在读取cd数据流时，用户弹出cd盘
             * */
            if (!p_items) {
                setCdaState(CDROM_INVALID);
                qCritical() << __FUNCTION__ << "read input_item_node_t failed,maybe caused by rejecting CD";
                continue;
            }

            m_mediaList.clear();
            for (int i = 0; i < p_items->i_children; i++) {
                input_item_node_t *child = p_items->pp_children[i];
                if (child != nullptr) {
                    qDebug() << __FUNCTION__ << "thread id:" << QThread::currentThread() \
                             << "name:" << child->p_item->psz_name << "duration:" << child->p_item->i_duration;
                    //空光盘有可能节点不为空，检测持续时间
                    if (child->p_item->i_duration == 0)
                        continue;
                    MediaMeta meta;
                    // cd的hash值添加编号防止psz_name相同
                    meta.hash = DMusic::filepathHash(QString(child->p_item->psz_name) + QString::number(i + 1));
                    QString strnum = QString::number(i + 1);
                    if (strnum.length() == 1) {
                        strnum.insert(0, '0'); //前置0显示，如01,02，
                    }
                    meta.title = QString("Audio CD - Track %1").arg(strnum); /*child->p_item->psz_name;*/
                    meta.localPath = child->p_item->psz_uri;
                    meta.length = child->p_item->i_duration / 1000; //microseconds to milliseconds
                    meta.track = i + 1; //cd的歌曲索引值，从1开始，用于歌曲切歌
                    meta.filetype = "cdda";
                    meta.mmType = MIMETYPE_CDA; //mimetype用于识别cd
                    m_mediaList << meta;
                }
            }

            input_item_node_Delete_func input_item_node_Delete_fc = (input_item_node_Delete_func)VlcDynamicInstance::VlcFunctionInstance()->resolveSymbol("input_item_node_Delete");
            /**
             * 释放查询的节点input_item_node_t，input_item_node_Delete为遍历释放，无需考虑子节点的释放
             * */
            input_item_node_Delete_fc(p_items);
            /**
             * 发送添加歌单消息
             * */
            if (m_mediaList.size() > 0) {
                setCdaState(CDROM_MOUNT_WITH_CD);
            }
        }
        sleep(1);
    }
}

