/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#include "singerlistview.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QMimeData>

#include <DMenu>
#include <DDialog>
#include <DDesktopServices>
#include <DScrollBar>
#include <DGuiApplicationHelper>

#include "util/pinyinsearch.h"

#include "singerdelegate.h"
#include "singerdatamodel.h"
#include "databaseservice.h"
#include "commonservice.h"
#include "musiclistdialog.h"
#include "global.h"
#include "ac-desktop-define.h"

DWIDGET_USE_NAMESPACE
// 升序
bool moreThanTimestampASC(SingerInfo v1, SingerInfo v2)
{
    return v1.timestamp <= v2.timestamp;
}

bool moreThanSingerASC(SingerInfo v1, SingerInfo v2)
{
    return v1.pinyinSinger <= v2.pinyinSinger;
}
// 降序
bool moreThanTimestampDES(SingerInfo v1, SingerInfo v2)
{
    return v1.timestamp >= v2.timestamp;
}

bool moreThanSingerDES(SingerInfo v1, SingerInfo v2)
{
    return v1.pinyinSinger >= v2.pinyinSinger;
}

int calculateSingerSize(int index, SingerInfo info)
{
    return index + info.musicinfos.size();
}

// 排序
void SingerListView::sortList(QList<SingerInfo> &singerInfos, const DataBaseService::ListSortType &sortType)
{
    switch (sortType) {
    case DataBaseService::SortByAddTimeASC: {
        qSort(singerInfos.begin(), singerInfos.end(), moreThanTimestampASC);
        break;
    }
    case DataBaseService::SortBySingerASC: {
        qSort(singerInfos.begin(), singerInfos.end(), moreThanSingerASC);
        break;
    }
    case DataBaseService::SortByAddTimeDES: {
        qSort(singerInfos.begin(), singerInfos.end(), moreThanTimestampDES);
        break;
    }
    case DataBaseService::SortBySingerDES: {
        qSort(singerInfos.begin(), singerInfos.end(), moreThanSingerDES);
        break;
    }
    default:
        break;
    }
}



SingerListView::SingerListView(QString hash, QWidget *parent)
    : DListView(parent)
{
    setObjectName("SingerListView");
    m_hash = hash;
    singerModel = new SingerDataModel(0, 1, this);

    signerDelegate = new SingerDataDelegate(this);
    setModel(singerModel);
    setItemDelegate(signerDelegate);
    setViewportMargins(0, 0, 8, 0);

    setUniformItemSizes(true);

    setViewModeFlag(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    musicListDialog = new MusicListDialog("artist", this);
//    AC_SET_OBJECT_NAME(musicListDialog, AC_musicListDialogSinger);
//    AC_SET_ACCESSIBLE_NAME(musicListDialog, AC_musicListDialogSinger);

    connect(this, &SingerListView::doubleClicked, this, &SingerListView::onDoubleClicked);

    setSelectionMode(QListView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(DataBaseService::getInstance(), &DataBaseService::signalCoverUpdate,
            this, &SingerListView::slotCoverUpdate);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &SingerListView::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());

    // 歌曲删除
    connect(DataBaseService::getInstance(), &DataBaseService::signalRmvSong,
            this, &SingerListView::slotRemoveSingleSong);

    // 跳转到播放的位置
    connect(CommonService::getInstance(), &CommonService::sigScrollToCurrentPosition,
            this, &SingerListView::slotScrollToCurrentPosition);
}

SingerListView::~SingerListView()
{
}

void SingerListView::setSingerListData(QList<SingerInfo> &&listinfo)
{
    setUpdatesEnabled(false);
    singerModel->clear();

    DataBaseService::ListSortType sortType = getSortType();
    this->setDataBySortType(listinfo, sortType);
    setUpdatesEnabled(true);
}

QList<SingerInfo> SingerListView::getSingerListData() const
{
    QList<SingerInfo> list;
    if (singerModel) {
        for (int i = 0; i < singerModel->rowCount(); i++) {
            QModelIndex idx = singerModel->index(i, 0, QModelIndex());
            SingerInfo singerTmp = idx.data(Qt::UserRole).value<SingerInfo>();
            list.append(singerTmp);
        }
    }
    return list;
}

void SingerListView::resetSingerListDataByStr(const QString &searchWord)
{
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    DataBaseService::ListSortType sortType = getSortType();

    // 排序
    sortList(singerInfos, sortType);

    singerModel->clear();
    for (SingerInfo meta : singerInfos) {
        if (!CommonService::getInstance()->containsStr(searchWord, meta.singerName)) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
        //设置icon
        bool iconExists = false;
        for (int i = 0; i < meta.musicinfos.values().size(); i++) {
            MediaMeta metaBind = meta.musicinfos.values().at(i);
            QString imagesDirPath = Global::cacheDir() + "/images/" + metaBind.hash + ".jpg";
            QFileInfo file(imagesDirPath);
            QIcon icon;
            if (file.exists()) {
                pItem->setIcon(QIcon(imagesDirPath));
                iconExists = true;
                break;
            }
        }
        if (!iconExists) {
            pItem->setIcon(m_defaultIcon);
        }
        singerModel->appendRow(pItem);
        auto row = singerModel->rowCount() - 1;
        QModelIndex idx = singerModel->index(row, 0, QModelIndex());
        QVariant singerval;
        singerval.setValue(meta);
        singerModel->setData(idx, singerval, Qt::UserRole);
    }
}

void SingerListView::resetSingerListDataBySongName(const QList<MediaMeta> &mediaMetas)
{
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    DataBaseService::ListSortType sortType = getSortType();

    // 排序
    sortList(singerInfos, sortType);

    singerModel->clear();
    for (SingerInfo singerInfo : singerInfos) {
//        static SingerInfo &tmpMeta = singerInfo;
//        QList<MediaMeta> mediaMetasTemp = mediaMetas;
//        bool ret = std::any_of(mediaMetasTemp.begin(), mediaMetasTemp.end(), [](MediaMeta mt) {
//            return CommonService::getInstance()->containsStr(mt.singer, tmpMeta.singerName);
//        });
        bool isSingerContainSong = false;
        foreach (MediaMeta meta, mediaMetas) {
            if (CommonService::getInstance()->containsStr(meta.singer, singerInfo.singerName)) {
                isSingerContainSong = true;
                break;
            }
        }
        if (isSingerContainSong) {
            QStandardItem *pItem = new QStandardItem;
            //设置icon
            bool iconExists = false;
            for (int i = 0; i < singerInfo.musicinfos.values().size(); i++) {
                MediaMeta metaBind = singerInfo.musicinfos.values().at(i);
                QString imagesDirPath = Global::cacheDir() + "/images/" + metaBind.hash + ".jpg";
                QFileInfo file(imagesDirPath);
                QIcon icon;
                if (file.exists()) {
                    pItem->setIcon(QIcon(imagesDirPath));
                    iconExists = true;
                    break;
                }
            }
            if (!iconExists) {
                pItem->setIcon(m_defaultIcon);
            }
            singerModel->appendRow(pItem);
            auto row = singerModel->rowCount() - 1;
            QModelIndex idx = singerModel->index(row, 0, QModelIndex());
            QVariant singerval;
            singerval.setValue(singerInfo);
            singerModel->setData(idx, singerval, Qt::UserRole);
        }
    }
}

void SingerListView::resetSingerListDataByAlbum(const QList<AlbumInfo> &albumInfos)
{
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    DataBaseService::ListSortType sortType = getSortType();

    // 排序
    sortList(singerInfos, sortType);

    singerModel->clear();
    for (SingerInfo singerInfo : singerInfos) {
//        static SingerInfo &tmpMeta = singerInfo;
//        bool ret = std::any_of(albumInfos.begin(), albumInfos.end(), [](AlbumInfo mt) {return CommonService::getInstance()->containsStr(mt.singer, tmpMeta.singerName);});
        bool isSingerContainSong = false;
        foreach (AlbumInfo albumInfo, albumInfos) {
            if (CommonService::getInstance()->containsStr(albumInfo.singer, singerInfo.singerName)) {
                isSingerContainSong = true;
                break;
            }
        }
        if (isSingerContainSong) {
            QStandardItem *pItem = new QStandardItem;
            //设置icon
            bool iconExists = false;
            for (int i = 0; i < singerInfo.musicinfos.values().size(); i++) {
                MediaMeta metaBind = singerInfo.musicinfos.values().at(i);
                QString imagesDirPath = Global::cacheDir() + "/images/" + metaBind.hash + ".jpg";
                QFileInfo file(imagesDirPath);
                QIcon icon;
                if (file.exists()) {
                    pItem->setIcon(QIcon(imagesDirPath));
                    iconExists = true;
                    break;
                }
            }
            if (!iconExists) {
                pItem->setIcon(m_defaultIcon);
            }
            singerModel->appendRow(pItem);
            auto row = singerModel->rowCount() - 1;
            QModelIndex idx = singerModel->index(row, 0, QModelIndex());
            QVariant singerval;
            singerval.setValue(singerInfo);
            singerModel->setData(idx, singerval, Qt::UserRole);
        }
    }
}

int SingerListView::getMusicCount()
{
    QList<SingerInfo> singerInfos = getSingerListData();
    return std::accumulate(singerInfos.begin(), singerInfos.end(), 0, calculateSingerSize);
}

void SingerListView::setViewModeFlag(QListView::ViewMode mode)
{
    if (mode == QListView::IconMode) {
        setIconSize(QSize(150, 150));
        setGridSize(QSize(-1, -1));
        setViewportMargins(-10, -13, -35, 10);
        setSpacing(20);
    } else {
        setIconSize(QSize(36, 36));
        setGridSize(QSize(-1, -1));
        setViewportMargins(0, 0, 8, 0);
        setSpacing(0);
    }
    setViewMode(mode);
    m_viewModel = mode;
}

QListView::ViewMode SingerListView::getViewMode()
{
    return m_viewModel;
}

void SingerListView::setThemeType(int type)
{
    musicTheme = type;
//    musicListDialog->setThemeType(type);
}

int SingerListView::getThemeType() const
{
    return musicTheme;
}

//void SingerListView::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
//{
////    if (musciListDialog->isVisible())
////        musciListDialog->setPlayPixmap(pixmap, sidebarPixmap);
//    playingPix = pixmap;
//    sidebarPix = sidebarPixmap;
//    update();
//}

QPixmap SingerListView::getPlayPixmap() const
{
    return playingPix;
}

//QPixmap SingerListView::getSidebarPixmap() const
//{
//    return sidebarPix;
//}

QPixmap SingerListView::getPlayPixmap(bool isSelect)
{
    QColor color;
    if (isSelect) {
        color = QColor(Qt::white);
    } else {
        color = QColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
    }

    QImage playingImage = Player::getInstance()->playingIcon().pixmap(QSize(20, 18), QIcon::Active, QIcon::On).toImage();
    for (int i = 0; i < playingImage.width(); i++) {
        for (int j = 0; j < playingImage.height(); j++) {
            if (playingImage.pixelColor(i, j) != QColor(0, 0, 0, 0)) {
                playingImage.setPixelColor(i, j, color);
            }
        }
    }
    QPixmap playingPixmap = QPixmap::fromImage(playingImage);
    update();
    return playingPixmap;
}

DataBaseService::ListSortType SingerListView::getSortType()
{
    DataBaseService::ListSortType type = static_cast<DataBaseService::ListSortType>
                                         (DataBaseService::getInstance()->getPlaylistSortType(m_hash));
    return type;
}

void SingerListView::setSortType(DataBaseService::ListSortType sortType)
{
    // 倒序
    switch (sortType) {
    case DataBaseService::SortByAddTime: {
        if (getSortType() == DataBaseService::SortByAddTimeASC) {
            sortType = DataBaseService::SortByAddTimeDES;
        } else {
            sortType = DataBaseService::SortByAddTimeASC;
        }
        break;
    }
    case DataBaseService::SortBySinger: {
        if (getSortType() == DataBaseService::SortBySingerASC) {
            sortType = DataBaseService::SortBySingerDES;
        } else {
            sortType = DataBaseService::SortBySingerASC;
        }
        break;
    }
    default:
        sortType = DataBaseService::SortByAddTimeASC;
        break;
    }

    DataBaseService::getInstance()->updatePlaylistSortType(sortType, m_hash);
    QList<SingerInfo> singerInfos = getSingerListData();
    this->setDataBySortType(singerInfos, sortType);
}

void SingerListView::setDataBySortType(QList<SingerInfo> &singerInfos, DataBaseService::ListSortType sortType)
{
    // 排序
    sortList(singerInfos, sortType);

    singerModel->clear();
    for (SingerInfo meta : singerInfos) {
        QStandardItem *pItem = new QStandardItem;
        //设置icon
        bool iconExists = false;
        for (int i = 0; i < meta.musicinfos.values().size(); i++) {
            MediaMeta metaBind = meta.musicinfos.values().at(i);
            QString imagesDirPath = Global::cacheDir() + "/images/" + metaBind.hash + ".jpg";
            QFileInfo file(imagesDirPath);
            QIcon icon;
            if (file.exists()) {
                pItem->setIcon(QIcon(imagesDirPath));
                iconExists = true;
                break;
            }
        }
        if (!iconExists) {
            pItem->setIcon(m_defaultIcon);
        }
        singerModel->appendRow(pItem);
        auto row = singerModel->rowCount() - 1;
        QModelIndex idx = singerModel->index(row, 0, QModelIndex());
        QVariant singerval;
        singerval.setValue(meta);
        singerModel->setData(idx, singerval, Qt::UserRole);
    }
}

void SingerListView::slotRemoveSingleSong(const QString &listHash, const QString &musicHash)
{
    if (listHash != "all") {
        return;
    }
    for (int i = 0; i < singerModel->rowCount(); i++) {
        QModelIndex idx = singerModel->index(i, 0, QModelIndex());
        SingerInfo singerTmp = idx.data(Qt::UserRole).value<SingerInfo>();

        if (singerTmp.musicinfos.contains(musicHash)) {
            singerTmp.musicinfos.remove(musicHash);

            QVariant singerVal;
            singerVal.setValue(singerTmp);
            singerModel->setData(idx, singerVal, Qt::UserRole);
            // 记录切换歌单之前播放状态
            Player::PlaybackStatus preStatue = Player::getInstance()->status();
            bool isActive = (musicHash == Player::getInstance()->getActiveMeta().hash);
            // 更新player中缓存的歌曲信息，如果存在正在播放的歌曲，停止播放
            Player::getInstance()->playRmvMeta(QStringList() << musicHash);
            // 如果该专辑内歌曲不存在了，则刷新页面
            if (singerTmp.musicinfos.size() == 0) {
                singerModel->removeRow(i);
                int nextPlayIndex = i;
                if (singerModel->rowCount() > 0 && isActive) {
                    if (nextPlayIndex == singerModel->rowCount()) {
                        nextPlayIndex = 0;
                    }
                    QModelIndex nextModelIndex = singerModel->index(nextPlayIndex, 0, QModelIndex());
                    SingerInfo nextSinger = nextModelIndex.data(Qt::UserRole).value<SingerInfo>();
                    Player::getInstance()->clearPlayList();
                    Player::getInstance()->setPlayList(nextSinger.musicinfos.values());
                    Player::getInstance()->setCurrentPlayListHash("artist", false);
                    emit Player::getInstance()->signalPlayListChanged();
                    if (preStatue == Player::PlaybackStatus::Playing) {
                        Player::getInstance()->playMeta(nextSinger.musicinfos.values().first());
                    } else {
                        Player::getInstance()->setActiveMeta(nextSinger.musicinfos.values().first());
                    }
                }
            }
            break;
        }
    }
}

void SingerListView::slotScrollToCurrentPosition(QString songlistHash)
{
    qDebug() << __FUNCTION__ << songlistHash;
    // listmode情况下跳转到播放位置
    if (songlistHash == "artist" && this->viewMode() == QListView::ListMode) {
        int height = 0;
        QString currentMetaHash = Player::getInstance()->getActiveMeta().hash;
        for (int i = 0; i < singerModel->rowCount(); i++) {
            QModelIndex idx = singerModel->index(i, 0, QModelIndex());
            QSize size = signerDelegate->sizeHint(QStyleOptionViewItem(), idx);
            SingerInfo singerInfo = idx.data(Qt::UserRole).value<SingerInfo>();
            if (singerInfo.musicinfos.contains(currentMetaHash)) {
                this->verticalScrollBar()->setValue(height);
                break;
            }
            height += size.height();
        }
    }
}

void SingerListView::onDoubleClicked(const QModelIndex &index)
{
    SingerInfo signerTmp = index.data(Qt::UserRole).value<SingerInfo>();
    // 原来的弹框修改为显示二级菜单
//    musicListDialog->flushDialog(signerTmp.musicinfos, false);
//    musicListDialog->exec();
    if (m_hash == "artist") {
        emit CommonService::getInstance()->signalShowSubSonglist(signerTmp.musicinfos, SingerType);
    } else if (m_hash == "artistResult") {
        emit CommonService::getInstance()->signalShowSubSonglist(signerTmp.musicinfos, SearchSingerResultType);
    }
}

void SingerListView::slotCoverUpdate(const MediaMeta &meta)
{
    for (int i = 0; i < singerModel->rowCount(); i++) {
        QModelIndex idx = singerModel->index(i, 0, QModelIndex());
        SingerInfo singerInfo = idx.data(Qt::UserRole).value<SingerInfo>();

        if (singerInfo.musicinfos.contains(meta.hash)) {
            QStandardItem *item = dynamic_cast<QStandardItem *>(singerModel->item(i, 0));
            if (item == nullptr) {
                break;
            }
            QString imagesDirPath = Global::cacheDir() + "/images/" + meta.hash + ".jpg";
            QFileInfo file(imagesDirPath);
            QIcon icon;
            if (file.exists()) {
                item->setIcon(QIcon(imagesDirPath));
            } else {
                item->setIcon(m_defaultIcon);
            }
            break;
        }
    }
}

void SingerListView::dragEnterEvent(QDragEnterEvent *event)
{
    auto t_formats = event->mimeData()->formats();
    qDebug() << t_formats;
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    }
}

void SingerListView::dragMoveEvent(QDragMoveEvent *event)
{
    auto index = indexAt(event->pos());
    if (/*index.isValid() && */(event->mimeData()->hasFormat("text/uri-list")  || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        DListView::dragMoveEvent(event);
    }
}

void SingerListView::dropEvent(QDropEvent *event)
{
    if ((!event->mimeData()->hasFormat("text/uri-list") && !event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        return;
    }

    if (event->mimeData()->hasFormat("text/uri-list")) {
        auto urls = event->mimeData()->urls();
        QStringList localpaths;
        for (auto &url : urls) {
            localpaths << url.toLocalFile();
        }

        if (!localpaths.isEmpty()) {
            DataBaseService::getInstance()->importMedias("all", localpaths);
        }
    }

    DListView::dropEvent(event);
}










