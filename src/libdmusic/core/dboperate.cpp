/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dboperate.h"

#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>

#include "audioanalysis.h"
#include "utils.h"

using namespace DMusic;

DBOperate::DBOperate(QStringList supportedSuffixs, QObject *parent)
    : QObject(parent)
{
    for (QString str : supportedSuffixs) {
        m_supportedSuffixs.append("*." + str);
    }
}

void DBOperate::slotImportMetas(const QStringList &urls, const QSet<QString> &metaHashs, bool importPlay,
                                const QSet<QString> &playMetaHashs, const QSet<QString> &allMetaHashs,
                                const QString &playlistHash, const bool &playFalg)
{
    QString mediaHash;
    // 统计总共需要加载的数量
    QStringList filePaths;
    QStringList allUrls = urls;
    // 添加默认音乐目录
    if (allUrls.isEmpty())
        allUrls.append(DmGlobal::musicPath());
    for (auto &curUrl : allUrls) {
        if (curUrl.isEmpty()) {
            continue;
        }
        QUrl url(curUrl);
        QString filepath = url.toLocalFile().isEmpty() ? curUrl : url.toLocalFile();
        QFileInfo fileInfo(filepath);
        if (fileInfo.isDir()) {
            QDirIterator it(filepath, m_supportedSuffixs,
                            QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                filePaths.append(it.next());
            }
        } else {
            filePaths.append(filepath);
        }
    }
    qDebug() << __FUNCTION__ << "allCount = " << filePaths.size();

    int importedCount = 0, importedFailCount = 0, existCount = 0;
    QSet<QString> allHashs;
    for (auto &filePath : filePaths) {
        QFileInfo fileinfo(filePath);
        while (fileinfo.isSymLink()) {  //to find final target
            fileinfo.setFile(fileinfo.symLinkTarget());
        }
        auto hash = Utils::filePathHash(fileinfo.absoluteFilePath());
        DMusic::MediaMeta mediaMeta;
        mediaMeta.hash = hash;
        QSet<QString> curHashs;
        if (!playlistHash.isEmpty()) {
            if (metaHashs.contains(mediaMeta.hash)) {
                existCount++;
            } else if (!allMetaHashs.contains(mediaMeta.hash)) {
                mediaMeta = AudioAnalysis::creatMediaMeta(filePath);
                if (mediaMeta.length > 0) {
                    AudioAnalysis::parseMetaCover(mediaMeta);
                    curHashs << "all" << playlistHash;
                    allHashs << "all" << playlistHash;
                } else {
                    importedFailCount++;
                }
            } else {
                curHashs << playlistHash;
                allHashs << playlistHash;
            }
        } else {
            if (!allMetaHashs.contains(mediaMeta.hash)) {
                mediaMeta = AudioAnalysis::creatMediaMeta(filePath);
                if (mediaMeta.length > 0) {
                    AudioAnalysis::parseMetaCover(mediaMeta);
                    curHashs << "all";
                    allHashs << "all";
                } else {
                    importedFailCount++;
                }
            } else {
                existCount++;
            }
        }
        // 自动添加到播放列表
        if (mediaMeta.length > 0 && importPlay && !playMetaHashs.contains(mediaMeta.hash)) {
            curHashs << "play";
            allHashs << "play";
        }
        if (!curHashs.isEmpty())
            emit signalAddOneMeta(curHashs.toList(), mediaMeta);
        // 自动播放
        if (mediaHash.isEmpty() && playFalg && mediaMeta.length > 0)
            mediaHash = mediaMeta.hash;

        importedCount++;
    }

    if (importedCount > 0)
        emit signalImportFinished(allHashs.toList(), importedFailCount, importedCount - importedFailCount - existCount, existCount, mediaHash);
}
