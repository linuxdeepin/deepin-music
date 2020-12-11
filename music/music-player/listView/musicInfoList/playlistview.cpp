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

#include "playlistview.h"

#include <QDebug>
#include <DMenu>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QStyleFactory>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QVariant>
#include <QShortcut>
#include <QMimeData>

#include <DDialog>
#include <DDesktopServices>
#include <DScrollBar>
#include <DGuiApplicationHelper>
#include <DLabel>
#include "util/threadpool.h"
#include "util/pinyinsearch.h"

#include "playitemdelegate.h"
#include "playlistmodel.h"
#include "databaseservice.h"
#include "global.h"
#include "mediameta.h"
#include "player.h"
#include "commonservice.h"
#include "util/global.h"
#include "metadetector.h"
#include "infodialog.h"

#include "ac-desktop-define.h"

DWIDGET_USE_NAMESPACE
bool moreThanTimestamp(MediaMeta v1, MediaMeta v2)
{
    return v1.timestamp < v2.timestamp;
}

bool moreThanTitle(const MediaMeta v1, const MediaMeta v2)
{
    return v1.pinyinTitle < v2.pinyinTitle;
}

bool moreThanSinger(MediaMeta v1, MediaMeta v2)
{
    return v1.pinyinArtist < v2.pinyinArtist;
}

bool moreThanAblum(const MediaMeta v1, const MediaMeta v2)
{
    return v1.pinyinAlbum < v2.pinyinAlbum;
}

PlayListView::PlayListView(QString hash, bool isPlayQueue, QWidget *parent)
    : DListView(parent)
{
    m_IsPlayQueue = isPlayQueue;
    m_currentHash = hash.isEmpty() ? "all" : hash;
    setObjectName("PlayListView");

    m_model = new PlaylistModel(0, 1, this);
    m_model->clear();
    setModel(m_model);

    m_delegate = new PlayItemDelegate;
    setItemDelegate(m_delegate);

    setUniformItemSizes(true);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);

    setMovement(QListView::Free);
    //默认QListView::ListMode
    setViewModeFlag(m_currentHash, QListView::ListMode);

    setResizeMode(QListView::Adjust);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);

    setSelectionMode(QListView::ExtendedSelection);
//setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    slotTheme(DGuiApplicationHelper::instance()->themeType());

    setContextMenuPolicy(Qt::DefaultContextMenu);

    //detail shortcut
    m_pDetailShortcut = new QShortcut(this);
    m_pDetailShortcut->setKey(QKeySequence(QLatin1String("Ctrl+I")));
    connect(m_pDetailShortcut, SIGNAL(activated()), this, SLOT(showDetailInfoDlg()));
    //快捷移出歌单
    m_pRmvSongsShortcut = new QShortcut(this);
    m_pRmvSongsShortcut->setKey(QKeySequence(QLatin1String("Delete")));
    connect(m_pRmvSongsShortcut, SIGNAL(activated()), this, SLOT(slotRmvFromSongList()));
    //快捷显示菜单
//    m_pShowMenuShortcut = new QShortcut(this);
//    m_pShowMenuShortcut->setKey(QKeySequence(QLatin1String("Alt+M")));

    connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotOnDoubleClicked(const QModelIndex &)));

    connect(Player::instance(), SIGNAL(signalUpdatePlayingIcon()),
            this, SLOT(slotUpdatePlayingIcon()), Qt::DirectConnection);
    connect(Player::instance(), &Player::signalPlayQueueMetaRemove,
            this, &PlayListView::slotPlayQueueMetaRemove);

    connect(DataBaseService::getInstance(), &DataBaseService::sigImportFinished,
            this, &PlayListView::slotImportFinished);

    connect(DataBaseService::getInstance(), &DataBaseService::sigCoverUpdate,
            this, &PlayListView::slotCoverUpdate);

    connect(DataBaseService::getInstance(), &DataBaseService::sigRmvSong,
            this, &PlayListView::slotRemoveSingleSong);
}

PlayListView::~PlayListView()
{

}

void PlayListView::setThemeType(int type)
{
    m_themeType = type;
}

int PlayListView::getThemeType() const
{
    return m_themeType;
}

QStandardItem *PlayListView::item(int row, int column) const
{
    return  m_model->item(row, column);
}

void PlayListView::setCurrentItem(QStandardItem *item)
{
    setCurrentIndex(m_model->indexFromItem(item));
}

void PlayListView::initAllSonglist(QString hash)
{
    m_currentHash = hash;
    m_model->clear();
    QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();

    DataBaseService::ListSortType sortType = getSortType();
    if (sortType == DataBaseService::SortByAddTime) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanTimestamp);
    } else if (sortType == DataBaseService::SortByTitle) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanTitle);
    } else if (sortType == DataBaseService::SortBySinger) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanSinger);
    } else if (sortType == DataBaseService::SortByAblum) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanAblum);
    }

    m_model->clear();
    for (int i = 0; i < mediaMetas.size(); i++) {
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + mediaMetas.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon(":/common/image/cover_max.svg");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;
        MediaMeta meta = mediaMetas.at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

void PlayListView::initCostomSonglist(const QString &hash)
{
    m_currentHash = hash;
    m_model->clear();
    QList<MediaMeta> list = DataBaseService::getInstance()->customizeMusicInfos(hash);

    for (int i = 0; i < list.size(); i++) {
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + list.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon(":/common/image/cover_max.svg");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;
        MediaMeta meta = list.at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

void PlayListView::resetSonglistByStr(const QString &searchWord)
{
    m_model->clear();
    QList<MediaMeta> list = DataBaseService::getInstance()->allMusicInfos();

    for (int i = 0; i < list.size(); i++) {
        if (!CommonService::getInstance()->containsStr(searchWord, list.at(i).title)) {
            continue;
        }
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + list.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon(":/common/image/cover_max.svg");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;//
        MediaMeta meta = list.at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

void PlayListView::resetSonglistByAlbum(const QList<AlbumInfo> &albuminfos)
{
    QList<MediaMeta> list;
    for (AlbumInfo albuminfo : albuminfos) {
        for (MediaMeta meta : albuminfo.musicinfos.values()) {
            list.append(meta);
        }
    }

    m_model->clear();
    for (int i = 0; i < list.size(); i++) {
        QStandardItem *newItem = new QStandardItem;
        QString imagesDirPath = Global::cacheDir() + "/images/" + list.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon(":/common/image/cover_max.svg");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;//
        MediaMeta meta = list.at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

void PlayListView::resetSonglistBySinger(const QList<SingerInfo> &singerInfos)
{
    QList<MediaMeta> list;
    for (SingerInfo singerInfo : singerInfos) {
        for (MediaMeta meta : singerInfo.musicinfos.values()) {
            list.append(meta);
        }
    }

    m_model->clear();
    for (int i = 0; i < list.size(); i++) {
        QStandardItem *newItem = new QStandardItem;
        QString imagesDirPath = Global::cacheDir() + "/images/" + list.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon(":/common/image/cover_max.svg");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;//
        MediaMeta meta = list.at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

QList<MediaMeta> PlayListView::getMusicListData()
{
    QList<MediaMeta> list;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0, QModelIndex());
        MediaMeta meta = idx.data(Qt::UserRole).value<MediaMeta>();
        list.append(meta);
    }
    return list;
}

QPixmap PlayListView::getSidebarPixmap()
{
    if (m_sidebarPixmap.isNull()) {
        m_sidebarPixmap = QPixmap(":/mpimage/light/music_withe_sidebar/music1.svg");
    }
    return m_sidebarPixmap;
}

QPixmap PlayListView::getPlayPixmap(bool isSelect)
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
    Player::instance()->playingIcon().paint(&painter, QRect(0, 0, 20, 20), Qt::AlignCenter, QIcon::Active, QIcon::On);
    update();
    return playingPixmap;
}

void PlayListView::playListChange()
{
    //todo..
    setUpdatesEnabled(false);
    setAutoScroll(false);

    m_model->clear();
    for (auto meta : *Player::instance()->getPlayList()) {
        QStandardItem *newItem = new QStandardItem;
        m_model->appendRow(newItem);
        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        QVariant mediaMeta;
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
    setAutoScroll(true);
    setUpdatesEnabled(true);
}

void PlayListView::setCurrentHash(QString hash)
{
    m_currentHash = hash;
}

QString PlayListView::getCurrentHash()
{
    return m_currentHash;
}

QListView::ViewMode PlayListView::getViewMode()
{
    if (!m_viewModeMap.contains(m_currentHash)) {
        m_viewModeMap[m_currentHash] = QListView::ViewMode::ListMode;
    }
    return m_viewModeMap[m_currentHash];
}

DataBaseService::ListSortType PlayListView::getSortType()
{
    DataBaseService::ListSortType type = static_cast<DataBaseService::ListSortType>
                                         (DataBaseService::getInstance()->getPlaylistSortType(m_currentHash));
    return type;
}

void PlayListView::setSortType(DataBaseService::ListSortType sortType)
{
    DataBaseService::getInstance()->updatePlaylistSortType(sortType, m_currentHash);
    QList<MediaMeta> mediaMetas = getMusicListData();
    if (sortType == DataBaseService::SortByAddTime) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanTimestamp);
    } else if (sortType == DataBaseService::SortByTitle) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanTitle);
    } else if (sortType == DataBaseService::SortBySinger) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanSinger);
    } else if (sortType == DataBaseService::SortByAblum) {
        qSort(mediaMetas.begin(), mediaMetas.end(), moreThanAblum);
    }

    m_model->clear();
    for (int i = 0; i < mediaMetas.size(); i++) {
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + mediaMetas.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon(":/common/image/cover_max.svg");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;
        MediaMeta meta = mediaMetas.at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

void PlayListView::setViewModeFlag(QString hash, QListView::ViewMode mode)
{
    m_viewModeMap[hash] = mode;
    if (mode == QListView::IconMode) {
        setIconSize(QSize(150, 150));
        setGridSize(QSize(-1, -1));
        setSpacing(20);
        setViewportMargins(-10, -10, -35, 10);
    } else {
        setIconSize(QSize(36, 36));
        setGridSize(QSize(-1, -1));
        setSpacing(0);
        setViewportMargins(0, 0, 8, 0);
    }
    setViewMode(mode);
}

void PlayListView::slotOnDoubleClicked(const QModelIndex &index)
{
    //todo检查文件是否存在
    MediaMeta itemMeta = index.data(Qt::UserRole).value<MediaMeta>();
    qDebug() << "------" << itemMeta.hash;

    if (!m_IsPlayQueue) {
        // 设置新的播放列表
        Player::instance()->clearPlayList();
        Player::instance()->setPlayList(getMusicListData());
        // 通知播放队列列表改变
        emit Player::instance()->signalPlayListChanged();
        // 设置当前播放playlist的hash
        Player::instance()->setCurrentPlayListHash(m_currentHash, false);
    }
    Player::instance()->playMeta(itemMeta);
}

void PlayListView::slotUpdatePlayingIcon()
{
    this->update();
}

void PlayListView::slotImportFinished(QString hash)
{
    emit CommonService::getInstance()->showPopupMessage(
        DataBaseService::getInstance()->getPlaylistNameByUUID("all"), 1, 1);

    //只刷新全部歌曲列表
    if (m_currentHash != hash) {
        return;
    }
    QList<MediaMeta> list = DataBaseService::getInstance()->getNewLoadMusicInfos();
    qDebug() << "------PlayListView::slotImportFinished count = " << list.size();

    DataBaseService::ListSortType sortType = getSortType();
    for (int i = 0; i < list.size(); i++) {
        MediaMeta addMeta = list.at(i);
        if (m_model->rowCount() == 0) {
            insertRow(0, addMeta);
        } else {
            bool isInserted = false;
            for (int rowIndex = 0; rowIndex < m_model->rowCount(); rowIndex++) {
                QModelIndex index = m_model->index(rowIndex, 0, QModelIndex());
                MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();
                if (sortType == DataBaseService::ListSortType::SortByAddTime) {
                    if (addMeta.timestamp <= meta.timestamp) {
                        insertRow(rowIndex, addMeta);
                        isInserted = true;
                        break;
                    }
                } else if (sortType == DataBaseService::ListSortType::SortByTitle) {
                    if (addMeta.pinyinTitle <= meta.pinyinTitle) {
                        insertRow(rowIndex, addMeta);
                        isInserted = true;
                        break;
                    }
                } else if (sortType == DataBaseService::ListSortType::SortBySinger) {
                    if (addMeta.pinyinArtist <= meta.pinyinArtist) {
                        insertRow(rowIndex, addMeta);
                        isInserted = true;
                        break;
                    }
                } else if (sortType == DataBaseService::ListSortType::SortByAblum) {
                    if (addMeta.pinyinAlbum <= meta.pinyinAlbum) {
                        insertRow(rowIndex, addMeta);
                        isInserted = true;
                        break;
                    }
                }
            }
            if (!isInserted) {
                insertRow(m_model->rowCount() - 1, addMeta);
            }
        }
    }
}

void PlayListView::slotCoverUpdate(const MediaMeta &meta)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0, QModelIndex());
        MediaMeta metaBind = idx.data(Qt::UserRole).value<MediaMeta>();

        if (metaBind.hash == meta.hash) {
            QStandardItem *item = dynamic_cast<QStandardItem *>(m_model->item(i, 0));
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

void PlayListView::slotTheme(int type)
{
    m_themeType = type;
}

void PlayListView::slotRemoveSingleSong(const QString &listHash, const QString &musicHash)
{
    if (!m_IsPlayQueue &&  listHash != m_currentHash) {
        return;
    }
    int row =  m_model->rowCount();
    for (int i = 0; i < row; i++) {
        QModelIndex mindex = m_model->index(i, 0, QModelIndex());
        MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
        if (meta.hash == musicHash) {
            this->removeItem(i);
            m_model->removeRow(row);
            update();
            break;
        }
    }
}

void PlayListView::slotAddToPlayQueue()
{
    QItemSelectionModel *selection = selectionModel();
    for (int i = 0; i < selection->selectedRows().size(); i++) {
        QModelIndex curIndex = selection->selectedRows().at(i);
        Player::instance()->playListAppendMeta(curIndex.data(Qt::UserRole).value<MediaMeta>());
    }

    Player::instance()->signalPlayListChanged();
}

void PlayListView::slotPlayMusic()
{
    slotOnDoubleClicked(this->currentIndex());
}

void PlayListView::showDetailInfoDlg()
{
    if (!m_pInfoDlg) {
        m_pInfoDlg = new InfoDialog(this);
        AC_SET_OBJECT_NAME(m_pInfoDlg, AC_infoDialog);
        AC_SET_ACCESSIBLE_NAME(m_pInfoDlg, AC_infoDialog);
    }

    QModelIndex mindex =  this->currentIndex();
    MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
    m_pInfoDlg->updateInfo(meta);
    m_pInfoDlg->show();
}

void PlayListView::slotPlayQueueMetaRemove(QString metaHash)
{
    if (!m_IsPlayQueue) {
        return;
    }
    int row =  m_model->rowCount();
    for (int i = 0; i < row; i++) {
        QModelIndex mindex = m_model->index(i, 0, QModelIndex());
        MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
        if (meta.hash == metaHash) {
            this->removeItem(i);
            m_model->removeRow(row);
            update();
            break;
        }
    }
}

void PlayListView::slotAddToFavSongList(const QString songName)
{
    QList<MediaMeta> listMeta;
    QItemSelectionModel *selection = selectionModel();
    for (int i = 0; i < selection->selectedRows().size(); i++) {
        QModelIndex curIndex = selection->selectedRows().at(i);
        listMeta << curIndex.data(Qt::UserRole).value<MediaMeta>();
    }

    int insertCount = DataBaseService::getInstance()->addMetaToPlaylist("fav", listMeta);
    CommonService::getInstance()->showPopupMessage(songName, selection->selectedRows().size(), insertCount);
    emit CommonService::getInstance()->fluashFavoriteBtnIco();
}

void PlayListView::slotAddToNewSongList(const QString songName)
{
    QItemSelectionModel *selection = selectionModel();
    QList<MediaMeta> metaList;
    for (int i = 0; i < selection->selectedRows().size(); i++) {
        QModelIndex curIndex = selection->selectedRows().at(i);
        MediaMeta meta = curIndex.data(Qt::UserRole).value<MediaMeta>();
        metaList.append(meta);
    }

    emit CommonService::getInstance()->addNewSongList();

    if (metaList.size() > 0) {
        QList<DataBaseService::PlaylistData> customSongList = DataBaseService::getInstance()->getCustomSongList();
        QString songlistUuid = customSongList.last().uuid;
        int insertCount = DataBaseService::getInstance()->addMetaToPlaylist(songlistUuid, metaList);
        CommonService::getInstance()->showPopupMessage(songName, metaList.size(), insertCount);
        //刷新
        emit CommonService::getInstance()->switchToView(CustomType, songlistUuid);
    }
}

void PlayListView::slotAddToCustomSongList()
{
    QAction *obj =   dynamic_cast<QAction *>(sender());
    QString songlistHash = obj->data().value<QString>();
    QList<MediaMeta> metas;
    QModelIndexList mindexlist =  this->selectedIndexes();
    for (QModelIndex mindex : mindexlist) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        metas.append(imt);
    }

    int insertCount = DataBaseService::getInstance()->addMetaToPlaylist(songlistHash, metas);
    CommonService::getInstance()->showPopupMessage(obj->text(), metas.size(), insertCount);
}

void PlayListView::slotOpenInFileManager()
{
    QModelIndexList modellist = selectionModel()->selectedRows();
    if (modellist.size() == 0)
        return;
    MediaMeta imt = modellist.at(0).data(Qt::UserRole).value<MediaMeta>();
    auto dirUrl = QUrl::fromLocalFile(imt.localPath);
    Dtk::Widget::DDesktopServices::showFileItem(dirUrl);
}

void PlayListView::slotRmvFromSongList()
{
    QModelIndexList modellist = selectionModel()->selectedRows();
    if (modellist.size() == 0)
        return;

    QStringList metaList;
    for (QModelIndex mindex : modellist) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        metaList << imt.hash;
    }

    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setObjectName("MessageBox");
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonNormal);
    int deleteFlag = warnDlg.addButton(tr("Remove"), false, Dtk::Widget::DDialog::ButtonWarning);

    MediaMeta meta = modellist.first().data(Qt::UserRole).value<MediaMeta>();
    if (1 == metaList.length()) {
        warnDlg.setMessage(QString(tr("Are you sure you want to remove %1?")).arg(meta.title));
    } else {
        warnDlg.setMessage(QString(tr("Are you sure you want to remove the selected %1 songs?").arg(metaList.size())));
    }

    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    if (deleteFlag == warnDlg.exec()) {
        //数据库中删除时有信号通知刷新界面
        if (!m_IsPlayQueue) {
            DataBaseService::getInstance()->removeSelectedSongs(m_currentHash, metaList, false);
            // 更新player中缓存的歌曲信息，如果存在正在播放的歌曲，停止播放
            if (m_currentHash == "all") {
                Player::instance()->playRmvMeta(metaList);
            }
        } else {
            Player::instance()->playRmvMeta(metaList);
        }
    }
}

void PlayListView::slotDelFromLocal()
{
    QList<MediaMeta> metas;
    QModelIndexList mindexlist =  this->selectedIndexes();
    QStringList strlist;
    for (QModelIndex mindex : mindexlist) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        metas.append(imt);
        strlist << imt.hash;
    }

    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setObjectName("MessageBox");
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonNormal);
    int deleteFlag = warnDlg.addButton(tr("Delete"), false, Dtk::Widget::DDialog::ButtonWarning);

    auto cover = QImage(QString(":/common/image/del_notify.svg"));
    if (1 == metas.length()) {
        auto meta = metas.first();
        warnDlg.setMessage(QString(tr("Are you sure you want to delete %1?")).arg(meta.title));
    } else {
        //                warnDlg.setTitle(QString(tr("Are you sure you want to delete the selected %1 songs?")).arg(metalist.length()));
        DLabel *t_titleLabel = new DLabel(this);
        t_titleLabel->setForegroundRole(DPalette::TextTitle);
        DLabel *t_infoLabel = new DLabel(this);
        t_infoLabel->setForegroundRole(DPalette::TextTips);
        t_titleLabel->setText(tr("Are you sure you want to delete the selected %1 songs?").arg(metas.length()));
        t_infoLabel->setText(tr("The song files contained will also be deleted"));
        warnDlg.addContent(t_titleLabel, Qt::AlignHCenter);
        warnDlg.addContent(t_infoLabel, Qt::AlignHCenter);
        warnDlg.addSpacing(20);
    }

    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    if (deleteFlag == warnDlg.exec()) {
        DataBaseService::getInstance()->removeSelectedSongs("all", strlist, true);
        // 更新player中缓存的歌曲信息，如果存在正在播放的歌曲，停止播放
        Player::instance()->playRmvMeta(strlist);
    }
}


void PlayListView::insertRow(int row, MediaMeta meta)
{
    QStandardItem *newItem = new QStandardItem;

    QString imagesDirPath = Global::cacheDir() + "/images/" + meta.hash + ".jpg";
    QFileInfo file(imagesDirPath);
    QIcon icon;
    if (file.exists()) {
        icon = QIcon(imagesDirPath);
    } else {
        icon = QIcon(":/common/image/cover_max.svg");
    }
    newItem->setIcon(icon);
    m_model->insertRow(row, newItem);

    QModelIndex index = m_model->index(row, 0, QModelIndex());
    QVariant mediaMeta;
    mediaMeta.setValue(meta);
    m_model->setData(index, mediaMeta, Qt::UserRole);
}

void PlayListView::keyPressEvent(QKeyEvent *event)
{
//    switch (event->modifiers()) {
//    case Qt::NoModifier:
//        switch (event->key()) {
//        case Qt::Key_Delete: {
//            QItemSelectionModel *selection = this->selectionModel();
//            d->removeSelection(selection);
//        }
//        break;
//        case Qt::Key_Return: {
//            QItemSelectionModel *selection = this->selectionModel();
//            if (!selection->selectedRows().isEmpty()) {
//                auto index = selection->selectedRows().first();
//                if (d->model->meta(index) == playlist()->playing()) {
//                    Q_EMIT resume(d->model->meta(index));
//                } else {
//                    Q_EMIT playMedia(d->model->meta(index));
//                }
//            }
//        }
//        break;
//        }
//        break;
//    case Qt::ShiftModifier:
//        switch (event->key()) {
//        case Qt::Key_Delete:
//            break;
//        }
//        break;
//    case Qt::ControlModifier:
//        switch (event->key()) {
//        case Qt::Key_I:
//            QItemSelectionModel *selection = this->selectionModel();
//            if (selection->selectedRows().length() <= 0) {
//                return;
//            }
//            auto index = selection->selectedRows().first();
//            auto meta = d->model->meta(index);
//            Q_EMIT showInfoDialog(meta);
//            break;
//        }
//        break;
//    default:
//        break;
//    }

    QAbstractItemView::keyPressEvent(event);
}

void PlayListView::keyboardSearch(const QString &search)
{
    Q_UNUSED(search);
}

void PlayListView::contextMenuEvent(QContextMenuEvent *event)
{
    QItemSelectionModel *selection = selectionModel();

    if (selection->selectedRows().size() <= 0) {
        return;
    }

    QPoint globalPos = mapToGlobal(event->pos());

    DMenu allMusicMenu;//first level menu
    DMenu playlistMenu;//second level menu
    DMenu textCodecMenu; //coding of song information
    QAction *actRmv = nullptr; //remove action
    QAction *actDel = nullptr;

    if (!m_IsPlayQueue) {
        playlistMenu.addAction(tr("Play queue"))->setData("play queue");
        playlistMenu.addSeparator();
    }
    QAction *actFav = playlistMenu.addAction(tr("My favorites"));
    actFav->setData("fav");

    playlistMenu.addSeparator();
    playlistMenu.addAction(tr("Add to new playlist"))->setData("song list");
    playlistMenu.addSeparator();

    //add custom playlist to second menu
    QList<DataBaseService::PlaylistData> strplaylist = DataBaseService::getInstance()->getCustomSongList();
    for (DataBaseService::PlaylistData pd : strplaylist) {
        if (m_currentHash != pd.uuid) { //filter itself
            QAction *pact = playlistMenu.addAction(pd.displayName);
            pact->setData(QVariant(pd.uuid)); //to know which custom view to reach
            connect(pact, SIGNAL(triggered()), this, SLOT(slotAddToCustomSongList()));
        }
    }


    if (selection->selectedRows().size() == 1) {// 选中一首歌
        QModelIndex curIndex = selection->selectedRows().at(0);
        MediaMeta currMeta = curIndex.data(Qt::UserRole).value<MediaMeta>();

        // 收藏按钮是否可以点击
        if (DataBaseService::getInstance()->favoriteExist(currMeta)) {
            actFav->setEnabled(false);
        }

        // 播放或则暂停按钮
        QAction *actplay = nullptr;
        if (currMeta.hash != Player::instance()->activeMeta().hash ||
                Player::instance()->status() != Player::Playing) {
            actplay = allMusicMenu.addAction(tr("Play"));
            connect(actplay, &QAction::triggered, this, &PlayListView::slotPlayMusic);
        } else {
            actplay = allMusicMenu.addAction(tr("Pause"));
            connect(actplay, &QAction::triggered, Player::instance(), &Player::pause);
        }

        if (currMeta.invalid)
            actplay->setEnabled(false);

        allMusicMenu.addAction(tr("Add to playlist"))->setMenu(&playlistMenu);
        allMusicMenu.addSeparator();
        QAction *actdisplay = allMusicMenu.addAction(tr("Display in file manager"));
        if (m_IsPlayQueue) {
            actRmv = allMusicMenu.addAction(tr("Remove from play queue"));
        } else {
            actRmv = allMusicMenu.addAction(tr("Remove from playlist"));
        }
        actDel = allMusicMenu.addAction(tr("Delete from local disk"));

        allMusicMenu.addSeparator();

        QList<QByteArray> codecList = MetaDetector::detectEncodings(currMeta);

        if (!codecList.contains("UTF-8")) {
            codecList.push_front("UTF-8");
        }
        if (QLocale::system().name() == "zh_CN") {
            if (codecList.contains("GB18030")) {
                codecList.removeAll("GB18030");
            }

            if (!codecList.isEmpty()) {
                codecList.push_front("GB18030");
            }
        }

        for (auto codec : codecList) {
            auto act = textCodecMenu.addAction(codec);
            act->setParent(&textCodecMenu);
            act->setCheckable(true);

            if (codec == currMeta.codec) {
                act->setChecked(true);
            }

            act->setData(QVariant::fromValue(codec));
        }

        if (codecList.length() >= 1) {
            allMusicMenu.addSeparator();
            allMusicMenu.addAction(tr("Encoding"))->setMenu(&textCodecMenu);
        }

        allMusicMenu.addSeparator();
        auto actsonginfo = allMusicMenu.addAction(tr("Song info"));

        //connnect
        connect(actdisplay, SIGNAL(triggered()), this, SLOT(slotOpenInFileManager()));
        connect(actsonginfo, SIGNAL(triggered()), this, SLOT(showDetailInfoDlg()));
        connect(&textCodecMenu, &QMenu::triggered, this, &PlayListView::slotTextCodecMenuClicked);
    } else {// 选中多首歌

        // 收藏按钮是否可以点击
        actFav->setEnabled(false);
        for (int i = 0; i < selection->selectedRows().size(); i++) {
            QModelIndex curIndex = selection->selectedRows().at(i);
            MediaMeta currMeta = curIndex.data(Qt::UserRole).value<MediaMeta>();

            if (!DataBaseService::getInstance()->favoriteExist(currMeta)) {
                actFav->setEnabled(true);
                break;
            }
        }

        allMusicMenu.addAction(tr("Add to playlist"))->setMenu(&playlistMenu);
        if (m_IsPlayQueue) {
            actRmv = allMusicMenu.addAction(tr("Remove from play queue"));
        } else {
            actRmv = allMusicMenu.addAction(tr("Remove from playlist"));
        }
        actDel = allMusicMenu.addAction(tr("Delete from local disk"));
    }

    connect(actRmv, SIGNAL(triggered()), this, SLOT(slotRmvFromSongList()));
    connect(actDel, SIGNAL(triggered()), this, SLOT(slotDelFromLocal()));
    connect(&playlistMenu, &QMenu::triggered, this, &PlayListView::slotPlaylistMenuClicked);

    allMusicMenu.exec(globalPos);
}

void PlayListView::dragMoveEvent(QDragMoveEvent *event)
{
    auto index = indexAt(event->pos());
    if (/*index.isValid() && */(event->mimeData()->hasFormat("text/uri-list")  || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        DListView::dragMoveEvent(event);
    }
}

void PlayListView::dropEvent(QDropEvent *event)
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
            DataBaseService::getInstance()->importMedias(m_currentHash, localpaths);
        }
    }

    DListView::dropEvent(event);
}

bool PlayListView::getIsPlayQueue() const
{
    return m_IsPlayQueue;
}

void PlayListView::reflushItemMediaMeta(const MediaMeta &meta)
{
    for (int i = 0; i <  m_model->rowCount(); i++) {
        QModelIndex curIndex = m_model->index(i, 0);
        MediaMeta metaTemp = curIndex.data(Qt::UserRole).value<MediaMeta>();

        if (meta.hash == metaTemp.hash) {
            QVariant mediaMeta;
            mediaMeta.setValue(meta);
            m_model->setData(curIndex, mediaMeta, Qt::UserRole);
            break;
        }
    }
}

void PlayListView::slotTextCodecMenuClicked(QAction *action)
{
    QItemSelectionModel *selection = selectionModel();
    if (selection->selectedRows().size() > 0) {
        QModelIndex curIndex = selection->selectedRows().at(0);
        MediaMeta meta = curIndex.data(Qt::UserRole).value<MediaMeta>();

        qDebug() << action->data().toByteArray();
        meta.codec = action->data().toByteArray();
        meta.updateCodec(action->data().toByteArray());
        //reflushItemMediaMeta(meta);
        QVariant varmeta;
        varmeta.setValue(meta);
        m_model->setData(curIndex, varmeta, Qt::UserRole);

        //restore to db
        DataBaseService::getInstance()->updateMetaCodec(meta);
    }
}

void PlayListView::slotPlaylistMenuClicked(QAction *action)
{
    QString actionText = action->data().toString();
    if (actionText == "fav") {
        slotAddToFavSongList(action->text());
    } else if (actionText == "song list") {
        slotAddToNewSongList(action->text());
    } else if (actionText == "play queue") {
        slotAddToPlayQueue();
    }
}

void PlayListView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

void PlayListView::dragEnterEvent(QDragEnterEvent *event)
{
    auto t_formats = event->mimeData()->formats();
    if (event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    }
}

void PlayListView::startDrag(Qt::DropActions supportedActions)
{
    QItemSelection selection;

    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        selection.append(QItemSelectionRange(index));
    }

    if (!selectionModel()->selectedIndexes().isEmpty())
        scrollTo(selectionModel()->selectedIndexes().first());
    setAutoScroll(false);
    DListView::startDrag(supportedActions);
    setAutoScroll(true);

    if (!selection.isEmpty()) {
        selectionModel()->select(selection, QItemSelectionModel::Select);
    }
}

