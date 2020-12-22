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

    connect(DataBaseService::getInstance(), &DataBaseService::signalCoverUpdate,
            this, &SingerListView::slotCoverUpdate);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &SingerListView::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());

    // 歌曲删除
    connect(DataBaseService::getInstance(), &DataBaseService::signalRmvSong,
            this, &SingerListView::slotRemoveSingleSong);
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
    for (int i = 0; i < singerModel->rowCount(); i++) {
        QModelIndex idx = singerModel->index(i, 0, QModelIndex());
        SingerInfo singerTmp = idx.data(Qt::UserRole).value<SingerInfo>();
        list.append(singerTmp);
    }
    return list;
}

void SingerListView::resetSingerListDataByStr(const QString &searchWord)
{
    QList<SingerInfo> singerInfoList = DataBaseService::getInstance()->allSingerInfos();
    DataBaseService::ListSortType sortType = getSortType();
    switch (sortType) {
    case DataBaseService::SortByAddTimeASC: {
        qSort(singerInfoList.begin(), singerInfoList.end(), moreThanTimestampASC);
        break;
    }
    case DataBaseService::SortBySingerASC: {
        qSort(singerInfoList.begin(), singerInfoList.end(), moreThanSingerASC);
        break;
    }
    case DataBaseService::SortByAddTimeDES: {
        qSort(singerInfoList.begin(), singerInfoList.end(), moreThanTimestampDES);
        break;
    }
    case DataBaseService::SortBySingerDES: {
        qSort(singerInfoList.begin(), singerInfoList.end(), moreThanSingerDES);
        break;
    }
    default:
        break;
    }

    singerModel->clear();
    for (SingerInfo meta : singerInfoList) {
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

    singerModel->clear();
    for (SingerInfo singerInfo : singerInfos) {
        bool isSingerContainSong = false;
        for (MediaMeta meta : mediaMetas) {
            if (CommonService::getInstance()->containsStr(meta.singer, singerInfo.singerName)) {
                isSingerContainSong = true;
                break;
            }
        }
        if (!isSingerContainSong) {
            continue;
        }
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

void SingerListView::resetSingerListDataByAlbum(const QList<AlbumInfo> &albumInfos)
{
    QList<SingerInfo> singerInfos = DataBaseService::getInstance()->allSingerInfos();
    DataBaseService::ListSortType sortType = getSortType();
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

    singerModel->clear();
    for (SingerInfo singerInfo : singerInfos) {
        bool isSingerContainSong = false;
        for (AlbumInfo albumInfo : albumInfos) {
            if (CommonService::getInstance()->containsStr(albumInfo.singer, singerInfo.singerName)) {
                isSingerContainSong = true;
                break;
            }
        }
        if (!isSingerContainSong) {
            continue;
        }
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

int SingerListView::getMusicCount()
{
    int count = 0;
    for (SingerInfo info : getSingerListData()) {
        count += info.musicinfos.size();
    }
    return count;
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

QPixmap SingerListView::getPlayPixmap(bool isSelect)
{
    QPixmap playingPixmap = QPixmap(QSize(20, 20));
    playingPixmap.fill(Qt::transparent);
    QPainter painter(&playingPixmap);
    DTK_NAMESPACE::Gui::DPalette pa;// = this->palette();
    if (isSelect) {
        painter.setPen(QColor(Qt::white));
    } else {
        painter.setPen(pa.color(QPalette::Active, DTK_NAMESPACE::Gui::DPalette::Highlight));
    }
    Player::getInstance()->playingIcon().paint(&painter, QRect(0, 0, 20, 20), Qt::AlignCenter, QIcon::Active, QIcon::On);
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
            break;
        }
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










