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

bool moreThanTimestamp(AlbumInfo v1, AlbumInfo v2)
{
    return v1.timestamp < v2.timestamp;
}

bool moreThanTitle(const AlbumInfo v1, const AlbumInfo v2)
{
    QString albumName1 = v1.albumName;
    QStringList curTextList = DMusic::PinyinSearch::simpleChineseSplit(albumName1);
    albumName1 = curTextList.join("");

    QString albumName2 = v2.albumName;
    curTextList = DMusic::PinyinSearch::simpleChineseSplit(albumName2);
    albumName2 = curTextList.join("");
    return v1.albumName < v2.albumName;
}

AlbumListView::AlbumListView(QString hash, QWidget *parent)
    : DListView(parent)
{
    m_hash = hash;
    setObjectName("AlbumListView");
    albumModel = new AlbumDataModel(0, 1, this);
    albumDelegate = new AlbumDataDelegate;
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

    connect(DataBaseService::getInstance(), &DataBaseService::sigCoverUpdate,
            this, &AlbumListView::slotCoverUpdate);
}

AlbumListView::~AlbumListView()
{
}

void AlbumListView::setAlbumListData(const QList<AlbumInfo> &listinfo)
{
    m_albumInfoList.clear();
    m_albumInfoList = listinfo;
    albumModel->clear();

    for (AlbumInfo albuminfo : m_albumInfoList) {
        QStandardItem *pItem = new QStandardItem;
        //设置icon
        bool iconExists = false;
        for (int i = 0; i < albuminfo.musicinfos.values().size(); i++) {
            MediaMeta metaBind = albuminfo.musicinfos.values().at(i);
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
        albumval.setValue(albuminfo);
        albumModel->setData(idx, albumval, Qt::UserRole);
    }
}

void AlbumListView::resetAlbumListDataByStr(const QString &searchWord)
{
    m_albumInfoList.clear();
    m_albumInfoList = DataBaseService::getInstance()->allAlbumInfos();

    this->albumModel->clear();
    for (AlbumInfo albumInfo : m_albumInfoList) {
        if (!CommonService::getInstance()->containsStr(searchWord, albumInfo.albumName)) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
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
    m_albumInfoList.clear();
    this->albumModel->clear();
    for (AlbumInfo albumInfo : albumInfoList) {
        bool isAlbumContainSong = false;
        for (MediaMeta albumMeta : albumInfo.musicinfos.values()) {
            for (MediaMeta listMeta : mediaMetas) {
                if (albumMeta.hash == listMeta.hash) {
                    isAlbumContainSong = true;
                    m_albumInfoList.append(albumInfo);
                    break;
                }
            }
            if (isAlbumContainSong) {
                break;
            }
        }
        if (!isAlbumContainSong) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
        albumModel->appendRow(pItem);
        auto row = albumModel->rowCount() - 1;
        QModelIndex idx = albumModel->index(row, 0, QModelIndex());
        QVariant albumval;
        albumval.setValue(albumInfo);
        albumModel->setData(idx, albumval, Qt::UserRole);
    }
}

void AlbumListView::resetAlbumListDataBySinger(const QList<SingerInfo> &singerInfos)
{
    QList<AlbumInfo> albumInfoList = DataBaseService::getInstance()->allAlbumInfos();
    m_albumInfoList.clear();

    this->albumModel->clear();
    for (AlbumInfo albumInfo : albumInfoList) {
        bool isAlbumContainSong = false;
        for (SingerInfo singerInfo : singerInfos) {
            if (CommonService::getInstance()->containsStr(singerInfo.singerName, albumInfo.singer)) {
                isAlbumContainSong = true;
                m_albumInfoList.append(albumInfo);
                break;
            }
        }
        if (!isAlbumContainSong) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
        albumModel->appendRow(pItem);
        auto row = albumModel->rowCount() - 1;
        QModelIndex idx = albumModel->index(row, 0, QModelIndex());
        QVariant albumval;
        albumval.setValue(albumInfo);
        albumModel->setData(idx, albumval, Qt::UserRole);
    }
}

QList<AlbumInfo> AlbumListView::getAlbumListData() const
{
    return m_albumInfoList;
}

int AlbumListView::rowCount()
{
    return  albumModel->rowCount();
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

MediaMeta AlbumListView::hoverin() const
{
    return hoverinMeta;
}

void AlbumListView::setThemeType(int type)
{
    musicTheme = type;
    musciListDialog->setThemeType(type);
}

int AlbumListView::getThemeType() const
{
    return musicTheme;
}

void AlbumListView::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
{
//    if (musciListDialog->isVisible())
//        musciListDialog->setPlayPixmap(pixmap, sidebarPixmap);
    playingPix = pixmap;
    sidebarPix = sidebarPixmap;
    albumPix = albumPixmap;
    update();
}

QPixmap AlbumListView::getPlayPixmap() const
{
    return playingPix;
}

QPixmap AlbumListView::getSidebarPixmap() const
{
    return sidebarPix;
}

QPixmap AlbumListView::getAlbumPixmap() const
{
    return albumPix;
}

void AlbumListView::onDoubleClicked(const QModelIndex &index)
{
    AlbumInfo albumTmp = index.data(Qt::UserRole).value<AlbumInfo>();
    musciListDialog->flushDialog(albumTmp.musicinfos, true);
    musciListDialog->exec();
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
    auto index = indexAt(event->pos());
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
    DataBaseService::getInstance()->updatePlaylistSortType(sortType, m_hash);
    if (sortType == DataBaseService::SortByAddTime) {
        qSort(m_albumInfoList.begin(), m_albumInfoList.end(), moreThanTimestamp);
    } else if (sortType == DataBaseService::SortByTitle) {
        qSort(m_albumInfoList.begin(), m_albumInfoList.end(), moreThanTitle);
    }
    albumModel->removeRows(0, albumModel->rowCount());
    for (AlbumInfo meta : m_albumInfoList) {
        QStandardItem *pItem = new QStandardItem();
        albumModel->appendRow(pItem);
        auto row = albumModel->rowCount() - 1;
        QModelIndex idx = albumModel->index(row, 0, QModelIndex());
        QVariant albumval;
        albumval.setValue(meta);
        albumModel->setData(idx, albumval, Qt::UserRole);
    }
    int count = albumModel->rowCount();
    qDebug() << count;
}
