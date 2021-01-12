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

#include "albumlistview.h"

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

#include "albumdelegate.h"
#include "albumdatamodel.h"
#include "databaseservice.h"
#include "commonservice.h"
#include "musiclistdialog.h"
#include "ac-desktop-define.h"
#include "global.h"
#include "playlistview.h"

DWIDGET_USE_NAMESPACE
// 升序
bool moreThanTimestampASC(AlbumInfo v1, AlbumInfo v2)
{
    return v1.timestamp <= v2.timestamp;
}

bool moreThanTitleASC(const AlbumInfo v1, const AlbumInfo v2)
{
    return v1.pinyinAlbum <= v2.pinyinAlbum;
}
// 降序
bool moreThanTimestampDES(AlbumInfo v1, AlbumInfo v2)
{
    return v1.timestamp >= v2.timestamp;
}

bool moreThanTitleDES(const AlbumInfo v1, const AlbumInfo v2)
{
    return v1.pinyinAlbum >= v2.pinyinAlbum;
}

int calculateAlbumSize(int index, AlbumInfo info)
{
    return index + info.musicinfos.size();
}

// 排序
void AlbumListView::sortList(QList<AlbumInfo> &albumInfos, const DataBaseService::ListSortType &sortType)
{
    switch (sortType) {
    case DataBaseService::SortByAddTimeASC: {
        qSort(albumInfos.begin(), albumInfos.end(), moreThanTimestampASC);
        break;
    }
    case DataBaseService::SortByTitleASC: {
        qSort(albumInfos.begin(), albumInfos.end(), moreThanTitleASC);
        break;
    }
    case DataBaseService::SortByAddTimeDES: {
        qSort(albumInfos.begin(), albumInfos.end(), moreThanTimestampDES);
        break;
    }
    case DataBaseService::SortByTitleDES: {
        qSort(albumInfos.begin(), albumInfos.end(), moreThanTitleDES);
        break;
    }
    default:
        break;
    }
}

AlbumListView::AlbumListView(QString hash, QWidget *parent)
    : DListView(parent), m_hash(hash)
{
    setObjectName("AlbumListView");
    albumModel = new AlbumDataModel(0, 1, this);
    albumDelegate = new AlbumDataDelegate(this);
    setModel(albumModel);
    setItemDelegate(albumDelegate);
    setViewportMargins(0, 0, 8, 0);

    setUniformItemSizes(true);

    setViewModeFlag(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    musciListDialog = new MusicListDialog("album", this);
    AC_SET_OBJECT_NAME(musciListDialog, AC_musicListDialogAlbum);
    AC_SET_ACCESSIBLE_NAME(musciListDialog, AC_musicListDialogAlbum);


    connect(this, &AlbumListView::doubleClicked, this, &AlbumListView::onDoubleClicked);

    setSelectionMode(QListView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(DataBaseService::getInstance(), &DataBaseService::signalCoverUpdate,
            this, &AlbumListView::slotCoverUpdate);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &AlbumListView::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
    // 歌曲删除
    connect(DataBaseService::getInstance(), &DataBaseService::signalRmvSong,
            this, &AlbumListView::slotRemoveSingleSong);
    // 跳转到播放的位置
    connect(CommonService::getInstance(), &CommonService::sigScrollToCurrentPosition,
            this, &AlbumListView::slotScrollToCurrentPosition);
}

AlbumListView::~AlbumListView()
{
}

void AlbumListView::setAlbumListData(QList<AlbumInfo> &&listinfo)
{
    albumModel->clear();
    DataBaseService::ListSortType sortType = getSortType();
    this->setDataBySortType(listinfo, sortType);
}

void AlbumListView::resetAlbumListDataByStr(const QString &searchWord)
{
    QList<AlbumInfo> albumInfoList = DataBaseService::getInstance()->allAlbumInfos();

    this->albumModel->clear();
    DataBaseService::ListSortType sortType = getSortType();

    // 排序
    sortList(albumInfoList, sortType);

    for (AlbumInfo albumInfo : albumInfoList) {
        if (!CommonService::getInstance()->containsStr(searchWord, albumInfo.albumName)) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
        //设置icon
        bool iconExists = false;
        for (int i = 0; i < albumInfo.musicinfos.values().size(); i++) {
            MediaMeta metaBind = albumInfo.musicinfos.values().at(i);
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
        albumModel->appendRow(pItem);
        auto row = albumModel->rowCount() - 1;
        QModelIndex idx = albumModel->index(row, 0, QModelIndex());
        QVariant albumval;
        albumval.setValue(albumInfo);
        albumModel->setData(idx, albumval, Qt::UserRole);
    }
}

void AlbumListView::resetAlbumListDataBySongName(const QList<MediaMeta> &mediaMetas)
{
    QList<AlbumInfo> albumInfoList = DataBaseService::getInstance()->allAlbumInfos();
    this->albumModel->clear();
    DataBaseService::ListSortType sortType = getSortType();

    // 排序
    sortList(albumInfoList, sortType);

    for (AlbumInfo albumInfo : albumInfoList) {
//            static MediaMeta &tmpMeta = albumMeta;
//            bool ret = std::any_of(mediaMetas.begin(), mediaMetas.end(), [](MediaMeta mt) {return mt.hash == tmpMeta.hash;});
        bool isAlbumContainSong = false;
        for (MediaMeta albumMeta : albumInfo.musicinfos.values()) {
            for (MediaMeta listMeta : mediaMetas) {
                if (albumMeta.hash == listMeta.hash) {
                    isAlbumContainSong = true;
                    break;
                }
            }
            if (isAlbumContainSong) {
                break;
            }
        }
        if (isAlbumContainSong) {
            QStandardItem *pItem = new QStandardItem;
            //设置icon
            pItem->setIcon(m_defaultIcon);
            for (int i = 0; i < albumInfo.musicinfos.values().size(); i++) {
                MediaMeta metaBind = albumInfo.musicinfos.values().at(i);
                QString imagesDirPath = Global::cacheDir() + "/images/" + metaBind.hash + ".jpg";
                QFileInfo file(imagesDirPath);
                if (file.exists()) {
                    pItem->setIcon(QIcon(imagesDirPath));
                    break;
                }
            }
            albumModel->appendRow(pItem);
            auto row = albumModel->rowCount() - 1;
            QModelIndex idx = albumModel->index(row, 0, QModelIndex());
            QVariant albumval;
            albumval.setValue(albumInfo);
            albumModel->setData(idx, albumval, Qt::UserRole);
        }
    }
}

void AlbumListView::resetAlbumListDataBySinger(const QList<SingerInfo> &singerInfos)
{
    QList<AlbumInfo> albumInfoList = DataBaseService::getInstance()->allAlbumInfos();

    this->albumModel->clear();
    DataBaseService::ListSortType sortType = getSortType();

    // 排序
    sortList(albumInfoList, sortType);

    for (AlbumInfo albumInfo : albumInfoList) {
//        static AlbumInfo &tmpMeta = albumInfo;
//        bool ret = std::any_of(singerInfos.begin(), singerInfos.end(), [](SingerInfo mt) {return CommonService::getInstance()->containsStr(mt.singerName, tmpMeta.singer);});
        bool isAlbumContainSong = false;
        for (SingerInfo singerInfo : singerInfos) {
            if (CommonService::getInstance()->containsStr(singerInfo.singerName, albumInfo.singer)) {
                isAlbumContainSong = true;
                break;
            }
        }
        if (isAlbumContainSong) {
            QStandardItem *pItem = new QStandardItem;
            //设置icon
            bool iconExists = false;
            for (int i = 0; i < albumInfo.musicinfos.values().size(); i++) {
                MediaMeta metaBind = albumInfo.musicinfos.values().at(i);
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
            albumModel->appendRow(pItem);
            auto row = albumModel->rowCount() - 1;
            QModelIndex idx = albumModel->index(row, 0, QModelIndex());
            QVariant albumval;
            albumval.setValue(albumInfo);
            albumModel->setData(idx, albumval, Qt::UserRole);
        }
    }
}

QList<AlbumInfo> AlbumListView::getAlbumListData() const
{
    QList<AlbumInfo> list;
    for (int i = 0; i < albumModel->rowCount(); i++) {
        QModelIndex idx = albumModel->index(i, 0, QModelIndex());
        AlbumInfo albumTmp = idx.data(Qt::UserRole).value<AlbumInfo>();
        list.append(albumTmp);
    }
    return list;
}

int AlbumListView::getMusicCount()
{
    QList<AlbumInfo> albumInfos = getAlbumListData();
    return std::accumulate(albumInfos.begin(), albumInfos.end(), 0, calculateAlbumSize);
}

void AlbumListView::setViewModeFlag(QListView::ViewMode mode)
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

QListView::ViewMode AlbumListView::getViewMode()
{
    return m_viewModel;
}

MediaMeta AlbumListView::playing() const
{
    return playingMeta;
}

//MediaMeta AlbumListView::hoverin() const
//{
//    return hoverinMeta;
//}

void AlbumListView::setThemeType(int type)
{
    musicTheme = type;
    musciListDialog->setThemeType(type);
}

int AlbumListView::getThemeType() const
{
    return musicTheme;
}

//void AlbumListView::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
//{
////    if (musciListDialog->isVisible())
////        musciListDialog->setPlayPixmap(pixmap, sidebarPixmap);
//    playingPix = pixmap;
//    sidebarPix = sidebarPixmap;
//    update();
//}

QPixmap AlbumListView::getPlayPixmap() const
{
    return playingPix;
}

//QPixmap AlbumListView::getSidebarPixmap() const
//{
//    return sidebarPix;
//}

QPixmap AlbumListView::getPlayPixmap(bool isSelect)
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

void AlbumListView::onDoubleClicked(const QModelIndex &index)
{
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();
    // 修改为二级页面,去掉dialog
//    musciListDialog->flushDialog(albumTmp.musicinfos, true);
//    musciListDialog->exec();
    emit CommonService::getInstance()->signalShowSubSonglist(albumTmp.musicinfos, true);
}

void AlbumListView::slotCoverUpdate(const MediaMeta &meta)
{
    for (int i = 0; i < albumModel->rowCount(); i++) {
        QModelIndex idx = albumModel->index(i, 0, QModelIndex());
        AlbumInfo albumTmp = idx.data(Qt::UserRole).value<AlbumInfo>();

        if (albumTmp.musicinfos.contains(meta.hash)) {
            QStandardItem *item = dynamic_cast<QStandardItem *>(albumModel->item(i, 0));
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

void AlbumListView::slotRemoveSingleSong(const QString &listHash, const QString &musicHash)
{
    if (listHash != "all") {
        return;
    }
    for (int i = 0; i < albumModel->rowCount(); i++) {
        QModelIndex idx = albumModel->index(i, 0, QModelIndex());
        AlbumInfo albumTmp = idx.data(Qt::UserRole).value<AlbumInfo>();

        if (albumTmp.musicinfos.contains(musicHash)) {
            albumTmp.musicinfos.remove(musicHash);

            QVariant albumval;
            albumval.setValue(albumTmp);
            albumModel->setData(idx, albumval, Qt::UserRole);
            break;
        }
    }
}

void AlbumListView::dragEnterEvent(QDragEnterEvent *event)
{
    auto t_formats = event->mimeData()->formats();
    qDebug() << t_formats;
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    }
}

void AlbumListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (/*index.isValid() && */(event->mimeData()->hasFormat("text/uri-list")  || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        DListView::dragMoveEvent(event);
    }
}

void AlbumListView::dropEvent(QDropEvent *event)
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

DataBaseService::ListSortType AlbumListView::getSortType()
{
    DataBaseService::ListSortType type = static_cast<DataBaseService::ListSortType>
                                         (DataBaseService::getInstance()->getPlaylistSortType(m_hash));
    return type;
}

void AlbumListView::setSortType(DataBaseService::ListSortType sortType)
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
    case DataBaseService::SortByTitle: {
        if (getSortType() == DataBaseService::SortByTitleASC) {
            sortType = DataBaseService::SortByTitleDES;
        } else {
            sortType = DataBaseService::SortByTitleASC;
        }
        break;
    }
    default:
        sortType = DataBaseService::SortByAddTimeASC;
        break;
    }

    DataBaseService::getInstance()->updatePlaylistSortType(sortType, m_hash);
    QList<AlbumInfo> albumInfos = getAlbumListData();
    this->setDataBySortType(albumInfos, sortType);
}

void AlbumListView::setDataBySortType(QList<AlbumInfo> &albumInfos, DataBaseService::ListSortType sortType)
{
    // 排序
    sortList(albumInfos, sortType);

    albumModel->removeRows(0, albumModel->rowCount());
    for (AlbumInfo meta : albumInfos) {
        QStandardItem *pItem = new QStandardItem();
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
        albumModel->appendRow(pItem);
        auto row = albumModel->rowCount() - 1;
        QModelIndex idx = albumModel->index(row, 0, QModelIndex());
        QVariant albumval;
        albumval.setValue(meta);
        albumModel->setData(idx, albumval, Qt::UserRole);
    }
}

void AlbumListView::slotScrollToCurrentPosition(QString songlistHash)
{
    qDebug() << __FUNCTION__ << songlistHash;
    // listmode情况下跳转到播放位置
    if (songlistHash == "album" && this->viewMode() == QListView::ListMode) {
        int height = 0;
        QString currentMetaHash = Player::getInstance()->getActiveMeta().hash;
        for (int i = 0; i < albumModel->rowCount(); i++) {
            QModelIndex idx = albumModel->index(i, 0, QModelIndex());
            QSize size = albumDelegate->sizeHint(QStyleOptionViewItem(), idx);
            AlbumInfo albumInfo = idx.data(Qt::UserRole).value<AlbumInfo>();
            if (albumInfo.musicinfos.contains(currentMetaHash)) {
                this->verticalScrollBar()->setValue(height);
                break;
            }
            height += size.height();
        }
    }
}
