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

#include <DMenu>
#include <DDialog>
#include <DDesktopServices>
#include <DScrollBar>

#include "util/pinyinsearch.h"

#include "singerdelegate.h"
#include "singerdatamodel.h"
#include "databaseservice.h"
#include "commonservice.h"
#include "musiclistdialog.h"
#include "global.h"

#include "ac-desktop-define.h"

DWIDGET_USE_NAMESPACE

bool moreThanTimestamp(SingerInfo v1, SingerInfo v2)
{
    return v1.timestamp < v2.timestamp;
}

bool moreThanSinger(SingerInfo v1, SingerInfo v2)
{
    QString singerName1 = v1.singerName;
    QStringList curTextList = DMusic::PinyinSearch::simpleChineseSplit(singerName1);
    singerName1 = curTextList.join("");

    QString singerName2 = v2.singerName;
    curTextList = DMusic::PinyinSearch::simpleChineseSplit(singerName2);
    singerName2 = curTextList.join("");
    return v1.singerName < v2.singerName;
}

SingerListView::SingerListView(QString hash, QWidget *parent)
    : DListView(parent)
{
    setObjectName("SingerListView");
    m_hash = hash;
    singerModel = new SingerDataModel(0, 1, this);
    signerDelegate = new SingerDataDelegate;
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

    musicListDialog = new MusicListDialog("artist", this);
    AC_SET_OBJECT_NAME(musicListDialog, AC_musicListDialogSinger);
    AC_SET_ACCESSIBLE_NAME(musicListDialog, AC_musicListDialogSinger);

    connect(this, &SingerListView::doubleClicked, this, &SingerListView::onDoubleClicked);

    setSelectionMode(QListView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(DataBaseService::getInstance(), &DataBaseService::sigCoverUpdate,
            this, &SingerListView::slotCoverUpdate);
}

SingerListView::~SingerListView()
{
}

void SingerListView::setSingerListData(const QList<SingerInfo> &listinfo)
{
    m_singerInfoList.clear();
    m_singerInfoList = listinfo;
    setUpdatesEnabled(false);
    singerModel->removeRows(0, singerModel->rowCount());

    for (SingerInfo singerInfo : listinfo) {
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

    setUpdatesEnabled(true);
}

QList<SingerInfo> SingerListView::getSingerListData() const
{
    return m_singerInfoList;
}

void SingerListView::resetSingerListDataByStr(const QString &searchWord)
{
    m_singerInfoList = DataBaseService::getInstance()->allSingerInfos();

    singerModel->clear();
    for (SingerInfo meta : m_singerInfoList) {
        if (!CommonService::getInstance()->containsStr(searchWord, meta.singerName)) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
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

    m_singerInfoList.clear();
    singerModel->clear();
    for (SingerInfo singerInfo : singerInfos) {
        bool isSingerContainSong = false;
        for (MediaMeta meta : mediaMetas) {
            if (CommonService::getInstance()->containsStr(meta.singer, singerInfo.singerName)) {
                isSingerContainSong = true;
                m_singerInfoList.append(singerInfo);
                break;
            }
        }
        if (!isSingerContainSong) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
        singerModel->appendRow(pItem);
        auto row = singerModel->rowCount() - 1;
        QModelIndex idx = singerModel->index(row, 0, QModelIndex());
        QVariant singerval;
        singerval.setValue(singerInfo);
        singerModel->setData(idx, singerval, Qt::UserRole);
    }
}

void SingerListView::resetSingerListDataByAlbum(const QList<AlbumInfo> &albumInfos)
{
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();

    m_singerInfoList.clear();
    singerModel->clear();
    for (SingerInfo singerInfo : m_singerInfoList) {
        bool isSingerContainSong = false;
        for (AlbumInfo albumInfo : albumInfos) {
            if (CommonService::getInstance()->containsStr(albumInfo.singer, singerInfo.singerName)) {
                isSingerContainSong = true;
                m_singerInfoList.append(singerInfo);
                break;
            }
        }
        if (!isSingerContainSong) {
            continue;
        }
        QStandardItem *pItem = new QStandardItem;
        singerModel->appendRow(pItem);
        auto row = singerModel->rowCount() - 1;
        QModelIndex idx = singerModel->index(row, 0, QModelIndex());
        QVariant singerval;
        singerval.setValue(singerInfo);
        singerModel->setData(idx, singerval, Qt::UserRole);
    }
}

int SingerListView::rowCount()
{
    return  singerModel->rowCount();
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
    musicListDialog->setThemeType(type);
}

int SingerListView::getThemeType() const
{
    return musicTheme;
}

void SingerListView::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap, QPixmap albumPixmap)
{
//    if (musciListDialog->isVisible())
//        musciListDialog->setPlayPixmap(pixmap, sidebarPixmap);
    playingPix = pixmap;
    sidebarPix = sidebarPixmap;
    albumPix = albumPixmap;
    update();
}

QPixmap SingerListView::getPlayPixmap() const
{
    return playingPix;
}

QPixmap SingerListView::getSidebarPixmap() const
{
    return sidebarPix;
}

QPixmap SingerListView::getAlbumPixmap() const
{
    return albumPix;
}

DataBaseService::ListSortType SingerListView::getSortType()
{
    DataBaseService::ListSortType type = static_cast<DataBaseService::ListSortType>
                                         (DataBaseService::getInstance()->getPlaylistSortType(m_hash));
    return type;
}

void SingerListView::setSortType(DataBaseService::ListSortType sortType)
{
    DataBaseService::getInstance()->updatePlaylistSortType(sortType, m_hash);
    if (sortType == DataBaseService::SortByAddTime) {
        qSort(m_singerInfoList.begin(), m_singerInfoList.end(), moreThanTimestamp);
    } else if (sortType == DataBaseService::SortBySinger) {
        qSort(m_singerInfoList.begin(), m_singerInfoList.end(), moreThanSinger);
    }
    singerModel->clear();
    for (SingerInfo meta : m_singerInfoList) {
        QStandardItem *pItem = new QStandardItem;
        singerModel->appendRow(pItem);
        auto row = singerModel->rowCount() - 1;
        QModelIndex idx = singerModel->index(row, 0, QModelIndex());
        QVariant singerval;
        singerval.setValue(meta);
        singerModel->setData(idx, singerval, Qt::UserRole);
    }
}

void SingerListView::onDoubleClicked(const QModelIndex &index)
{
    SingerInfo signerTmp = index.data(Qt::UserRole).value<SingerInfo>();
    musicListDialog->flushDialog(signerTmp.musicinfos, false);
    musicListDialog->exec();
}

void SingerListView::slotCoverUpdate(const MediaMeta &meta)
{
    for (int i = 0; i < singerModel->rowCount(); i++) {
        QModelIndex idx = singerModel->index(i, 0, QModelIndex());
        AlbumInfo albumTmp = idx.data(Qt::UserRole).value<AlbumInfo>();

        if (albumTmp.musicinfos.contains(meta.hash)) {
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










