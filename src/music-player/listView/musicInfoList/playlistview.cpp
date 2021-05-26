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
#include "speechCenter.h"
#include "ac-desktop-define.h"
#include "commonservice.h"
#include "songlistview.h"
#include "songlistviewdialog.h"

DWIDGET_USE_NAMESPACE
// 升序
bool moreThanTimestampASC(MediaMeta v1, MediaMeta v2)
{
    return v1.timestamp < v2.timestamp;
}

bool moreThanTitleASC(const MediaMeta v1, const MediaMeta v2)
{
    return v1.pinyinTitle < v2.pinyinTitle;
}

bool moreThanSingerASC(MediaMeta v1, MediaMeta v2)
{
    return v1.pinyinArtist < v2.pinyinArtist;
}

bool moreThanAblumASC(const MediaMeta v1, const MediaMeta v2)
{
    return v1.pinyinAlbum < v2.pinyinAlbum;
}
// 降序
bool moreThanTimestampDES(MediaMeta v1, MediaMeta v2)
{
    return v1.timestamp > v2.timestamp;
}

bool moreThanTitleDES(const MediaMeta v1, const MediaMeta v2)
{
    return v1.pinyinTitle > v2.pinyinTitle;
}

bool moreThanSingerDES(MediaMeta v1, MediaMeta v2)
{
    return v1.pinyinArtist > v2.pinyinArtist;
}

bool moreThanAblumDES(const MediaMeta v1, const MediaMeta v2)
{
    return v1.pinyinAlbum > v2.pinyinAlbum;
}

PlayListView::PlayListView(const QString &hash, bool isPlayQueue, QWidget *parent)
    : DListView(parent)
    , m_currentHash(hash.isEmpty() ? "all" : hash)
    , m_listPageType(NullType)
{
    //m_listPageType = NullType;
    m_IsPlayQueue = isPlayQueue;
    setObjectName("PlayListView");

    m_model = new PlaylistModel(0, 1, this);
    m_model->clear();
    setModel(m_model);

    m_delegate = new PlayItemDelegate(this);
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
    if (CommonService::getInstance()->isTabletEnvironment()) {
        setSelectionMode(QListView::SingleSelection);
    } else {
        setSelectionMode(QListView::ExtendedSelection);
    }
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setContextMenuPolicy(Qt::DefaultContextMenu);

    //detail shortcut
    m_pDetailShortcut = new QShortcut(this);
    m_pDetailShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    m_pDetailShortcut->setKey(QKeySequence(QLatin1String("Ctrl+I")));
    connect(m_pDetailShortcut, &QShortcut::activated, this, &PlayListView::showDetailInfoDlg);
//    //快捷移出歌单
//    m_pRmvSongsShortcut = new QShortcut(this);
//    m_pRmvSongsShortcut->setKey(QKeySequence(QLatin1String("Delete")));
//    connect(m_pRmvSongsShortcut, SIGNAL(activated()), this, SLOT(slotRmvFromSongList()));
//    //快捷显示菜单
//    m_pShowMenuShortcut = new QShortcut(this);
//    m_pShowMenuShortcut->setKey(QKeySequence(QLatin1String("Alt+M")));

    connect(Player::getInstance(), &Player::signalPlaybackStatusChanged,
            this, &PlayListView::slotPlaybackStatusChanged);

    if (CommonService::getInstance()->isTabletEnvironment()) {
        connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotOnClicked(const QModelIndex &)));
    } else {
        connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotOnDoubleClicked(const QModelIndex &)));
    }

    connect(Player::getInstance(), &Player::signalUpdatePlayingIcon,
            this, &PlayListView::slotUpdatePlayingIcon);
    connect(Player::getInstance(), &Player::signalPlayQueueMetaRemove,
            this, &PlayListView::slotPlayQueueMetaRemove);

    connect(DataBaseService::getInstance(), &DataBaseService::signalCoverUpdate,
            this, &PlayListView::slotCoverUpdate);

    connect(DataBaseService::getInstance(), &DataBaseService::signalRmvSong,
            this, &PlayListView::slotRemoveSingleSong);

    connect(DataBaseService::getInstance(), &DataBaseService::signalMusicAddOne,
            this, &PlayListView::slotMusicAddOne);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &PlayListView::setThemeType);

    setThemeType(DGuiApplicationHelper::instance()->themeType());
    // 跳转到播放的位置
    connect(CommonService::getInstance(), &CommonService::sigScrollToCurrentPosition,
            this, &PlayListView::slotScrollToCurrentPosition);
    // 刷新当前页面编码
    connect(CommonService::getInstance(), &CommonService::signalUpdateCodec,
            this, &PlayListView::slotUpdateCodec);
    // 移出cda格式歌曲
    connect(CommonService::getInstance(), &CommonService::signalCdaSongListChanged,
            this, &PlayListView::slotRmvCdaSongs);

    connect(CommonService::getInstance(), &CommonService::loadData,
            this, &PlayListView::slotLoadData, Qt::QueuedConnection);

    // 平板模式槽函数，不需要环境判断，信号只在平板时发送
    // 设置选择模式
    connect(CommonService::getInstance(), &CommonService::signalSelectMode,
            this, &PlayListView::slotSetSelectModel);

    connect(CommonService::getInstance(), &CommonService::signalSelectAll,
            this, &PlayListView::slotSelectAll);

    // 横竖屏切换
    connect(CommonService::getInstance(), &CommonService::signalHScreen,
            this, &PlayListView::slotHScreen);
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

void PlayListView::reloadAllSonglist()
{
    m_currentHash = "all";
    m_model->clear();
    QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();

    DataBaseService::ListSortType sortType = getSortType();
    this->setDataBySortType(mediaMetas, sortType);
}

//void PlayListView::setCurrentItem(QStandardItem *item)
//{
//    setCurrentIndex(m_model->indexFromItem(item));
//}

void PlayListView::initAllSonglist(const QString &hash)
{
    m_currentHash = hash;
    m_model->clear();

    if (DataBaseService::getInstance()->allMusicInfosCount() > FirstLoadCount) {
        QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->getMusicInfosBySortAndCount(FirstLoadCount);
        qDebug() << __FUNCTION__ << "mediaMetas.size = " << mediaMetas.size();
        m_model->clear();
        for (int i = 0; i < mediaMetas.size(); i++) {
            QStandardItem *newItem = new QStandardItem;

            QString imagesDirPath = Global::cacheDir() + "/images/" + mediaMetas.at(i).hash + ".jpg";
            QFileInfo file(imagesDirPath);
            QIcon icon;
            if (file.exists()) {
                icon = QIcon(imagesDirPath);
            } else {
                icon = QIcon::fromTheme("cover_max");
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
        emit CommonService::getInstance()->loadData();
    } else {
        QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();
        DataBaseService::ListSortType sortType = getSortType();
        this->setDataBySortType(mediaMetas, sortType);
    }
}

void PlayListView::initCostomSonglist(const QString &hash)
{
    if (DataBaseService::getInstance()->getDelStatus()) {
        DataBaseService::getInstance()->setDelNeedSleep();
    }

    m_currentHash = hash;

    QList<MediaMeta> mediaMetas;
    DataBaseService::ListSortType sortType = DataBaseService::SortByNull;
    if (hash == "CdaRole") { //从player获取cda缓存数据
        mediaMetas = Player::getInstance()->getCdaPlayList();
    } else {
        mediaMetas = DataBaseService::getInstance()->customizeMusicInfos(hash);
        sortType = getSortType();
    }

    mediaMetas = this->setDataBySortType(mediaMetas, sortType);
    // 返回给语音中心排好序的列表
    if (SpeechCenter::getInstance()->getNeedRefresh()) {
        SpeechCenter::getInstance()->setMediaMetas(mediaMetas);
    }
}

void PlayListView::resetSonglistByStr(const QString &searchWord)
{
    m_model->clear();
    QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();

    DataBaseService::ListSortType sortType = getSortType();

    // 排序
    sortList(mediaMetas, sortType);

    for (int i = 0; i < mediaMetas.size(); i++) {
        if (!CommonService::getInstance()->containsStr(searchWord, mediaMetas.at(i).title)) {
            continue;
        }
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + mediaMetas.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon::fromTheme("cover_max");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;//
        MediaMeta meta = mediaMetas.at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

void PlayListView::resetSonglistByAlbum(const QList<AlbumInfo> &albuminfos)
{
    QList<MediaMeta> mediaMetas;
    for (AlbumInfo albuminfo : albuminfos) {
        for (MediaMeta meta : albuminfo.musicinfos.values()) {
            mediaMetas.append(meta);
        }
    }
    DataBaseService::ListSortType sortType = getSortType();
    this->setDataBySortType(mediaMetas, sortType);
}

void PlayListView::resetSonglistBySinger(const QList<SingerInfo> &singerInfos)
{
    QList<MediaMeta> mediaMetas;
    for (SingerInfo singerInfo : singerInfos) {
        for (MediaMeta meta : singerInfo.musicinfos.values()) {
            mediaMetas.append(meta);
        }
    }
    DataBaseService::ListSortType sortType = getSortType();
    this->setDataBySortType(mediaMetas, sortType);
}

QList<MediaMeta> PlayListView::getMusicListData()
{
    QList<MediaMeta> list;
    if (DataBaseService::getInstance()->getDelStatus()) {
        // 批量删除后的音乐文件
        QStringList metaList = DataBaseService::getInstance()->getDelMetaHashs();
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex idx = m_model->index(i, 0, QModelIndex());
            MediaMeta meta = idx.data(Qt::UserRole).value<MediaMeta>();
            if (!metaList.contains(meta.hash)) {
                list.append(meta);
            }
        }
    } else {
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex idx = m_model->index(i, 0, QModelIndex());
            MediaMeta meta = idx.data(Qt::UserRole).value<MediaMeta>();
            list.append(meta);
        }
    }
    return list;
}

QList<MediaMeta> PlayListView::setDataBySortType(QList<MediaMeta> &mediaMetas, DataBaseService::ListSortType sortType)
{
    // 排序
    sortList(mediaMetas, sortType);
    m_model->clear();
    for (int i = 0; i < mediaMetas.size(); i++) {
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + mediaMetas.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon::fromTheme("cover_max");
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
    return mediaMetas;
}

QPixmap PlayListView::getPlayPixmap(bool isSelect)
{
    // 修改icon绘制方法为修改像素点颜色
    QColor color;
    if (isSelect) {
        color = QColor(Qt::white);
    } else {
        color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
    }

    QImage playingImage = Player::getInstance()->playingIcon().pixmap(QSize(20, 20), QIcon::Active, QIcon::On).toImage();
    for (int i = 0; i < playingImage.width(); i++) {
        for (int j = 0; j < playingImage.height(); j++) {
            if (playingImage.pixelColor(i, j) != QColor(0, 0, 0, 0)) {
                playingImage.setPixelColor(i, j, color);
            }
        }
    }
    QPixmap playingPixmap = QPixmap::fromImage(playingImage);
//    update();
    return playingPixmap;
}

void PlayListView::playListChange()
{
    //todo..
    setUpdatesEnabled(false);
    setAutoScroll(false);

    m_model->clear();
    for (auto meta : *Player::getInstance()->getPlayList()) {
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
    update();
}

//void PlayListView::setCurrentHash(QString hash)
//{
//    m_currentHash = hash;
//}

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
    case DataBaseService::SortBySinger: {
        if (getSortType() == DataBaseService::SortBySingerASC) {
            sortType = DataBaseService::SortBySingerDES;
        } else {
            sortType = DataBaseService::SortBySingerASC;
        }
        break;
    }
    case DataBaseService::SortByAblum: {
        if (getSortType() == DataBaseService::SortByAblumASC) {
            sortType = DataBaseService::SortByAblumDES;
        } else {
            sortType = DataBaseService::SortByAblumASC;
        }
        break;
    }
    default:
        sortType = DataBaseService::SortByAddTimeASC;
        break;
    }

    DataBaseService::getInstance()->updatePlaylistSortType(sortType, m_currentHash);
    QList<MediaMeta> mediaMetas = getMusicListData();
    this->setDataBySortType(mediaMetas, sortType);
}

void PlayListView::showErrorDlg()
{
    QList<DDialog *> ql = this->findChildren<DDialog *>("uniqueinvaliddailog");
    if (ql.size() > 0) {
        if (!ql.first()->isHidden())
            return ;
    }

    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setObjectName("uniqueinvaliddailog");
    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.setTitle(tr("File is invalid or does not exist, load failed"));
    warnDlg.addButtons(QStringList() << tr("OK"));
    warnDlg.setDefaultButton(0);
    if (0 == warnDlg.exec()) {
        //播放下一首
        Player::getInstance()->playNextMeta(true);
    }
}

int PlayListView::getMusicCount()
{
    return m_model->rowCount();
}

void PlayListView::setMusicListView(QMap<QString, MediaMeta> musicinfos, const QString &hash)
{
    m_currentHash = hash;
    m_model->clear();
    for (int i = 0; i < musicinfos.values().size(); i++) {
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + musicinfos.values().at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon::fromTheme("cover_max");
        }
        newItem->setIcon(icon);
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant mediaMeta;
        MediaMeta meta = musicinfos.values().at(i);
        mediaMeta.setValue(meta);
        m_model->setData(index, mediaMeta, Qt::UserRole);
    }
}

void PlayListView::setViewModeFlag(QString hash, QListView::ViewMode mode)
{
    m_viewModeMap[hash] = mode;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        if (mode == QListView::IconMode) {
            setGridSize(QSize(-1, -1));
            setSpacing(20);
            setIconSize(QSize(200, 243));
            // 修改底部间距
            setViewportMargins(30, -15, -35, 0);
            if (CommonService::getInstance()->isHScreen()) {
                setSpacing(20);
                setViewportMargins(30, -15, -35, 0);
            } else {
                setSpacing(33);
                setViewportMargins(23, -15, -35, 0);
            }
        } else {
            setIconSize(QSize(36, 36));
            setGridSize(QSize(-1, -1));
            setSpacing(0);
            // 修改顶部间距
            setViewportMargins(10, 0, 10, 0);
        }
    } else {
        if (mode == QListView::IconMode) {
            setGridSize(QSize(-1, -1));
            setSpacing(0);
            setIconSize(QSize(170, 210));
            // 修改底部间距
            setViewportMargins(0, 0, -35, 0);
        } else {
            setIconSize(QSize(36, 36));
            setGridSize(QSize(-1, -1));
            setSpacing(0);
            // 修改顶部间距
            setViewportMargins(0, 0, 8, 0);
        }
    }
    setViewMode(mode);
}

void PlayListView::slotOnClicked(const QModelIndex &index)
{
    //todo检查文件是否存在
    if (CommonService::getInstance()->getSelectModel() != CommonService::MultSelect || m_IsPlayQueue) {
        MediaMeta itemMeta = index.data(Qt::UserRole).value<MediaMeta>();
        qDebug() << "------" << itemMeta.hash;
        if (!QFileInfo(itemMeta.localPath).exists() && itemMeta.mmType != MIMETYPE_CDA) {
            //停止当前的歌曲
            Player::getInstance()->stop();
            //弹出提示框
            showErrorDlg();
        } else {
            playMusic(itemMeta);
        }
    }
}

void PlayListView::slotOnDoubleClicked(const QModelIndex &index)
{
    //todo检查文件是否存在
    MediaMeta itemMeta = index.data(Qt::UserRole).value<MediaMeta>();
    qDebug() << "------" << itemMeta.hash;
    if (!QFileInfo(itemMeta.localPath).exists() && itemMeta.mmType != MIMETYPE_CDA) {
        //停止当前的歌曲
        Player::getInstance()->stop();
        //弹出提示框
        showErrorDlg();
    } else {
        playMusic(itemMeta);
    }
}

void PlayListView::slotLoadData()
{
    QList<MediaMeta> mediaMetas = DataBaseService::getInstance()->allMusicInfos();
    DataBaseService::ListSortType sortType = getSortType();
    // 排序
    sortList(mediaMetas, sortType);

    for (int i = FirstLoadCount; i < mediaMetas.size(); i++) {
        QStandardItem *newItem = new QStandardItem;

        QString imagesDirPath = Global::cacheDir() + "/images/" + mediaMetas.at(i).hash + ".jpg";
        QFileInfo file(imagesDirPath);
        QIcon icon;
        if (file.exists()) {
            icon = QIcon(imagesDirPath);
        } else {
            icon = QIcon::fromTheme("cover_max");
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

void PlayListView::slotUpdatePlayingIcon()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0, QModelIndex());
        MediaMeta metaBind = idx.data(Qt::UserRole).value<MediaMeta>();
        if (metaBind.hash == Player::getInstance()->getActiveMeta().hash) {
            this->update(idx);
            break;
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
    if (listHash == "all" || listHash == m_currentHash) {
        int row =  m_model->rowCount();
        for (int i = 0; i < row; i++) {
            QModelIndex mindex = m_model->index(i, 0, QModelIndex());
            MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
            if (meta.hash == musicHash) {
                m_model->removeRow(mindex.row());
                //搜索结果中删除，刷新数量显示
                if (m_currentHash == "musicResult") {
                    emit musicResultListCountChanged("musicResult");
                }
                break;
            }
        }
    }
    // 由于该类复用，所以需要通过是否可见判断信号是否发送
    if (this->isVisible() && m_model->rowCount() == 0 && (m_listPageType == AlbumSubSongListType || m_listPageType == SingerSubSongListType
                                                          || m_listPageType == SearchAlbumSubSongListType || m_listPageType == SearchSingerSubSongListType)) {
        emit CommonService::getInstance()->signalSwitchToView(PreType, "", QMap<QString, MediaMeta>());
    }
}

void PlayListView::slotMusicAddOne(const QString &listHash, MediaMeta addMeta)
{
    if (m_currentHash == "album" || m_currentHash == "albumResult"
            || m_currentHash == "artist" || m_currentHash == "artistResult") {
        // 二级页面
        if (m_model->rowCount() < 0) {
            return;
        }
        if (m_currentHash == "album" || m_currentHash == "albumResult") {
            // 专辑二级页面根据专辑名称匹配
            QModelIndex index = m_model->index(0, 0, QModelIndex());
            MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();
            if (addMeta.album == meta.album) {
                insertRow(m_model->rowCount(), addMeta);
            }
        } else if (m_currentHash == "artist" || m_currentHash == "artistResult") {
            // 歌手二级页面根据歌手名称匹配
            QModelIndex index = m_model->index(0, 0, QModelIndex());
            MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();
            if (addMeta.singer == meta.singer) {
                insertRow(m_model->rowCount(), addMeta);
            }
        }
    } else {
        // 普通歌单页面
        if (!this->isVisible() || listHash != m_currentHash) {
            return;
        }
        DataBaseService::ListSortType sortType = DataBaseService::SortByAddTimeASC;//getSortType();
        if (m_model->rowCount() == 0) {
            insertRow(0, addMeta);
        } else {
            //如果已经存在，则不加入
            if (!isContain(addMeta.hash)) {
                bool isInserted = false;
                for (int rowIndex = 0; rowIndex < m_model->rowCount(); rowIndex++) {
                    isInserted = false;
                    QModelIndex index = m_model->index(rowIndex, 0, QModelIndex());
                    MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();
                    switch (sortType) {
                    case DataBaseService::SortByAddTimeASC: {
                        if (addMeta.timestamp <= meta.timestamp) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    case DataBaseService::SortByTitleASC: {
                        if (addMeta.pinyinTitle <= meta.pinyinTitle) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    case DataBaseService::SortBySingerASC: {
                        if (addMeta.pinyinArtist <= meta.pinyinArtist) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    case DataBaseService::SortByAblumASC: {
                        if (addMeta.pinyinAlbum <= meta.pinyinAlbum) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    case DataBaseService::SortByAddTimeDES: {
                        if (addMeta.timestamp >= meta.timestamp) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    case DataBaseService::SortByTitleDES: {
                        if (addMeta.pinyinTitle >= meta.pinyinTitle) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    case DataBaseService::SortBySingerDES: {
                        if (addMeta.pinyinArtist >= meta.pinyinArtist) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    case DataBaseService::SortByAblumDES: {
                        if (addMeta.pinyinAlbum <= meta.pinyinAlbum) {
                            insertRow(rowIndex, addMeta);
                            isInserted = true;
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                if (!isInserted) {
                    insertRow(m_model->rowCount(), addMeta);
                }
            }
        }
    }
}

void PlayListView::slotScrollToCurrentPosition(const QString &songlistHash)
{
    qDebug() << __FUNCTION__ << songlistHash;
    // listmode情况下跳转到播放位置
    if (songlistHash == "all" && this->viewMode() == QListView::ListMode) {
        int height = 0;
        QString currentMetaHash = Player::getInstance()->getActiveMeta().hash;
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex idx = m_model->index(i, 0, QModelIndex());
            QSize size = m_delegate->sizeHint(QStyleOptionViewItem(), idx);
            MediaMeta meta = idx.data(Qt::UserRole).value<MediaMeta>();
            if (meta.hash == currentMetaHash) {
                this->verticalScrollBar()->setValue(height);
                break;
            }
            height += size.height();
        }
    }
}

void PlayListView::slotAddToPlayQueue()
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    for (int i = 0; i < modelIndexList.size(); i++) {
        QModelIndex curIndex = modelIndexList.at(i);
        Player::getInstance()->playListAppendMeta(curIndex.data(Qt::UserRole).value<MediaMeta>());
    }

    Player::getInstance()->signalPlayListChanged();
}

void PlayListView::slotPlayMusic()
{
    if (CommonService::getInstance()->isTabletEnvironment()) {
        slotOnClicked(this->currentIndex());
    } else {
        slotOnDoubleClicked(this->currentIndex());
    }
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

void PlayListView::slotPlayQueueMetaRemove(const QString &metaHash)
{
    if (!m_IsPlayQueue) {
        return;
    }
    int row =  m_model->rowCount();
    for (int i = 0; i < row; i++) {
        QModelIndex mindex = m_model->index(i, 0, QModelIndex());
        MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
        if (meta.hash == metaHash) {
            m_model->removeRow(i);
            update();
            break;
        }
    }
}

void PlayListView::slotPlaybackStatusChanged(Player::PlaybackStatus statue)
{
    Q_UNUSED(statue)
    if (this->isVisible()) {
        this->update();
    }
}

void PlayListView::slotAddToFavSongList(const QString &songName)
{
    QList<MediaMeta> listMeta;
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    for (int i = 0; i < modelIndexList.size(); i++) {
        QModelIndex curIndex = modelIndexList.at(i);
        MediaMeta meta = curIndex.data(Qt::UserRole).value<MediaMeta>();
        if (meta.mmType != MIMETYPE_CDA)
            listMeta << curIndex.data(Qt::UserRole).value<MediaMeta>();
    }

    if (listMeta.size() == 0)
        return;
    int insertCount = DataBaseService::getInstance()->addMetaToPlaylist("fav", listMeta);
    emit CommonService::getInstance()->signalShowPopupMessage(songName, modelIndexList.size(), insertCount);
    emit CommonService::getInstance()->signalFluashFavoriteBtnIcon();
}

void PlayListView::slotAddToNewSongList(const QString &songName)
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    QList<MediaMeta> metaList;
    for (int i = 0; i < modelIndexList.size(); i++) {
        QModelIndex curIndex = modelIndexList.at(i);
        MediaMeta meta = curIndex.data(Qt::UserRole).value<MediaMeta>();
        if (meta.mmType != MIMETYPE_CDA)
            metaList.append(meta);
    }

    emit CommonService::getInstance()->signalAddNewSongList();

    if (metaList.size() > 0) {
        QList<DataBaseService::PlaylistData> customSongList = DataBaseService::getInstance()->getCustomSongList();
        QString songlistUuid = customSongList.last().uuid;
        int insertCount = DataBaseService::getInstance()->addMetaToPlaylist(songlistUuid, metaList);
        // 消息通知
        CommonService::getInstance()->signalShowPopupMessage(songName, metaList.size(), insertCount);
        // 刷新自定义歌单页面
        emit CommonService::getInstance()->signalSwitchToView(CustomType, songlistUuid);
    }
}

void PlayListView::slotAddToCustomSongList()
{
    QAction *obj =   dynamic_cast<QAction *>(sender());
    QString songlistHash = obj->data().value<QString>();
    QList<MediaMeta> metas;
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    for (QModelIndex mindex : modelIndexList) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        if (imt.mmType != MIMETYPE_CDA)
            metas.append(imt);
    }
    if (metas.size() == 0)
        return;
    int insertCount = DataBaseService::getInstance()->addMetaToPlaylist(songlistHash, metas);
    emit CommonService::getInstance()->signalShowPopupMessage(obj->property("displayName").toString(), metas.size(), insertCount);
}

void PlayListView::slotOpenInFileManager()
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    if (modelIndexList.size() == 0)
        return;
    MediaMeta imt = modelIndexList.at(0).data(Qt::UserRole).value<MediaMeta>();
    auto dirUrl = QUrl::fromLocalFile(imt.localPath);
    Dtk::Widget::DDesktopServices::showFileItem(dirUrl);
}

void PlayListView::slotRmvFromSongList()
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    if (modelIndexList.size() == 0)
        return;

    QStringList metaList;
    for (QModelIndex mindex : modelIndexList) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        //过滤cda格式歌曲
        if (imt.mmType != MIMETYPE_CDA) {
            metaList << imt.hash;
        }
    }

    //过滤cda格式以后再判断size
    if (metaList.size() == 0)
        return;
    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setObjectName("MessageBox");
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.addButton(tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
    warnDlg.addButton(tr("Remove"), true, Dtk::Widget::DDialog::ButtonWarning); //index 1
    MediaMeta meta = modelIndexList.first().data(Qt::UserRole).value<MediaMeta>();
    if (1 == metaList.length()) {
        warnDlg.setMessage(QString(tr("Are you sure you want to remove %1?")).arg(meta.title));
    } else {
        warnDlg.setMessage(QString(tr("Are you sure you want to remove the selected %1 songs?").arg(metaList.size())));
    }

    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    if (warnDlg.exec() > QDialog::Rejected) {
        //数据库中删除时有信号通知刷新界面
        if (!m_IsPlayQueue) {
            DataBaseService::getInstance()->removeSelectedSongs(m_currentHash, metaList, false);
            if (CommonService::getInstance()->isTabletEnvironment()) {
                tabletClearSelection();
            } else {
                clearSelection();
            }
            QString playListHash = Player::getInstance()->getCurrentPlayListHash();
            // 如果是专辑或者歌手,playRmvMeta的逻辑放在专辑与歌手中处理,二级页面删除后继续播放逻辑
            if (playListHash != "album" && playListHash != "artist" && playListHash != "albumResult" && playListHash != "artistResult") {
                Player::getInstance()->playRmvMeta(metaList);
            }
        } else {
            Player::getInstance()->playRmvMeta(metaList);
        }
    }
    if (CommonService::getInstance()->isTabletEnvironment() && !m_IsPlayQueue) {
        emit CommonService::getInstance()->setSelectModel(CommonService::SingleSelect);
    }
}

void PlayListView::slotDelFromLocal()
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    QStringList strlist;
    for (QModelIndex mindex : modelIndexList) {
        MediaMeta imt = mindex.data(Qt::UserRole).value<MediaMeta>();
        //过滤cda格式歌曲
        if (imt.mmType != MIMETYPE_CDA) {
            strlist << imt.hash;
        }
    }

    if (strlist.size() == 0)
        return;
    Dtk::Widget::DDialog warnDlg(this);
    warnDlg.setObjectName("MessageBox");
    warnDlg.setTextFormat(Qt::RichText);
    warnDlg.addButton(tr("Cancel"), true, Dtk::Widget::DDialog::ButtonNormal);
    int deleteFlag = warnDlg.addButton(tr("Delete"), false, Dtk::Widget::DDialog::ButtonWarning);

    if (1 == strlist.length()) {
        QModelIndex idx = modelIndexList.at(0);
        MediaMeta imt = idx.data(Qt::UserRole).value<MediaMeta>();
        warnDlg.setMessage(QString(tr("Are you sure you want to delete %1?")).arg(imt.title));
    } else {
        //                warnDlg.setTitle(QString(tr("Are you sure you want to delete the selected %1 songs?")).arg(metalist.length()));
        DLabel *t_titleLabel = new DLabel(this);
        t_titleLabel->setForegroundRole(DPalette::TextTitle);
        DLabel *t_infoLabel = new DLabel(this);
        t_infoLabel->setForegroundRole(DPalette::TextTips);
        t_titleLabel->setText(tr("Are you sure you want to delete the selected %1 songs?").arg(strlist.length()));
        t_infoLabel->setText(tr("The song files contained will also be deleted"));
        warnDlg.addContent(t_titleLabel, Qt::AlignHCenter);
        warnDlg.addContent(t_infoLabel, Qt::AlignHCenter);
        warnDlg.addSpacing(20);
    }

    warnDlg.setIcon(QIcon::fromTheme("deepin-music"));
    if (deleteFlag == warnDlg.exec()) {
        DataBaseService::getInstance()->removeSelectedSongs(m_currentHash, strlist, true);
        // 如果是专辑或者歌手,playRmvMeta的逻辑放在专辑与歌手中处理,二级页面删除后继续播放逻辑
        QString playListHash = Player::getInstance()->getCurrentPlayListHash();
        if (playListHash != "album" && playListHash != "artist" && playListHash != "albumResult" && playListHash != "artistResult") {
            Player::getInstance()->playRmvMeta(strlist);
        }
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
        icon = QIcon::fromTheme("cover_max");
    }
    newItem->setIcon(icon);
    m_model->insertRow(row, newItem);

    QModelIndex index = m_model->index(row, 0, QModelIndex());
    QVariant mediaMeta;
    mediaMeta.setValue(meta);
    m_model->setData(index, mediaMeta, Qt::UserRole);
}

bool PlayListView::isContain(const QString &hash)
{
    bool bIsContain = false;
    int row =  m_model->rowCount();
    for (int i = 0; i < row; i++) {
        QModelIndex mindex = m_model->index(i, 0, QModelIndex());
        MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
        if (meta.hash == hash) {
            bIsContain = true;
            break;
        }
    }
    return bIsContain;
}

void PlayListView::keyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_Return: {
            QModelIndexList mindexlist;
            if (CommonService::getInstance()->isTabletEnvironment()) {
                mindexlist =  this->tabletSelectedIndexes();
            } else {
                mindexlist =  this->selectedIndexes();
            }
            if (!mindexlist.isEmpty()) {
                QModelIndex index = mindexlist.first();
                MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();

                playMusic(meta);
            }
            break;
        }
        case Qt::Key_Delete: {
            slotRmvFromSongList();
        }
        }
        break;
    default:
        break;
    }

    QAbstractItemView::keyPressEvent(event);
}

void PlayListView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    if (modelIndexList.size() <= 0) {
        m_menuIsShow = false;
        return;
    }

    m_menuIsShow = true;
    DMenu allMusicMenu;//first level menu
    DMenu playlistMenu;//second level menu
    DMenu textCodecMenu; //coding of song information
    QAction *actRmv = nullptr; //remove action
    QPoint globalPos = mapToGlobal(event->pos());
    if (CommonService::getInstance()->isTabletEnvironment()) {
        actRmv = allMusicMenu.addAction(tr("Delete"));
        allMusicMenu.addSeparator();
        QAction *actplaylist =  allMusicMenu.addAction(tr("Add to playlist"));
        connect(actRmv, SIGNAL(triggered()), this, SLOT(slotRmvFromSongList()));
        connect(actplaylist, &QAction::triggered, this, &PlayListView::slotShowSongList);
    } else {
        //查找是否有cda格式歌曲
        QStringList metalist;
        foreach (QModelIndex mindex, modelIndexList) {
            MediaMeta meta = mindex.data(Qt::UserRole).value<MediaMeta>();
            if (meta.mmType != MIMETYPE_CDA)
                metalist << meta.hash;
        }

        //全是cda歌曲不处理右键菜单
        if (metalist.size() == 0) {
            m_menuIsShow = false;
            return;
        }

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
                QFontMetrics titleFm(actFav->font());
                QString displayName = pd.displayName;
                QString text = titleFm.elidedText(QString(pd.displayName.replace("&", "&&")), Qt::ElideMiddle, 170);

                QAction *pact = playlistMenu.addAction(text);

                pact->setProperty("displayName", displayName);
                pact->setData(QVariant(pd.uuid)); //to know which custom view to reach

                connect(pact, &QAction::triggered, this, &PlayListView::slotAddToCustomSongList);
            }
        }


        if (modelIndexList.size() == 1) {// 选中一首歌
            QModelIndex curIndex = modelIndexList.at(0);
            MediaMeta currMeta = curIndex.data(Qt::UserRole).value<MediaMeta>();

            // 收藏按钮是否可以点击
            if (DataBaseService::getInstance()->favoriteExist(currMeta)) {
                actFav->setEnabled(false);
            }

            // 播放或则暂停按钮
            QAction *actplay = nullptr;
            if (currMeta.hash != Player::getInstance()->getActiveMeta().hash ||
                    Player::getInstance()->status() != Player::Playing) {
                actplay = allMusicMenu.addAction(tr("Play"));
                connect(actplay, &QAction::triggered, this, &PlayListView::slotPlayMusic);
            } else {
                actplay = allMusicMenu.addAction(tr("Pause"));
                connect(actplay, &QAction::triggered, Player::getInstance(), &Player::pause);
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
            connect(actdisplay, &QAction::triggered, this, &PlayListView::slotOpenInFileManager);
            connect(actsonginfo, &QAction::triggered, this, &PlayListView::showDetailInfoDlg);
            connect(&textCodecMenu, &QMenu::triggered, this, &PlayListView::slotTextCodecMenuClicked);
        } else {// 选中多首歌

            // 收藏按钮是否可以点击
            actFav->setEnabled(false);
            for (int i = 0; i < modelIndexList.size(); i++) {
                QModelIndex curIndex = modelIndexList.at(i);
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
        connect(actDel, &QAction::triggered, this, &PlayListView::slotDelFromLocal);
        connect(&playlistMenu, &QMenu::triggered, this, &PlayListView::slotPlaylistMenuClicked);
    }
    allMusicMenu.exec(globalPos);
    m_menuIsShow = false;
}

void PlayListView::dragMoveEvent(QDragMoveEvent *event)
{
    if ((event->mimeData()->hasFormat("text/uri-list"))) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        DListView::dragMoveEvent(event);
    }
}

void PlayListView::dropEvent(QDropEvent *event)
{
    if ((!event->mimeData()->hasFormat("text/uri-list"))) {
        return;
    }

    if (event->mimeData()->hasFormat("text/uri-list")) {
        auto urls = event->mimeData()->urls();
        QStringList localpaths;
        for (auto &url : urls) {
            localpaths << url.toLocalFile();
        }

        if (!localpaths.isEmpty() && m_currentHash != "CdaRole") { //cda歌单不需要添加歌曲
            DataBaseService::getInstance()->importMedias(m_currentHash, localpaths);
        }
    }

    DListView::dropEvent(event);
}

bool PlayListView::getIsPlayQueue() const
{
    return m_IsPlayQueue;
}

bool PlayListView::getMenuIsShow()
{
    return m_menuIsShow;
}

void PlayListView::mousePressEvent(QMouseEvent *event)
{
    // 点击空白处时取消所有已选择项
    QModelIndex index = this->indexAt(event->pos());
    if (index.row() == -1) {
        if (CommonService::getInstance()->isTabletEnvironment()) {
            tabletClearSelection();
        } else {
            clearSelection();
        }
    } else {
        m_pressIndex = index;
        CommonService::TabletSelectMode model = CommonService::getInstance()->getSelectModel();
        if (model == CommonService::SingleSelect || m_IsPlayQueue) {
            for (int i = 0; i <  m_model->rowCount(); i++) {
                QModelIndex curIndex = m_model->index(i, 0);
                MediaMeta metaTemp = curIndex.data(Qt::UserRole).value<MediaMeta>();
                QVariant mediaMeta;
                if (curIndex.row() == index.row()) {
                    metaTemp.beSelect = true;
                } else {
                    metaTemp.beSelect = false;
                }
                mediaMeta.setValue(metaTemp);
                m_model->setData(curIndex, mediaMeta, Qt::UserRole);
            }
        }
    }
    QListView::mousePressEvent(event);
}

void PlayListView::mouseReleaseEvent(QMouseEvent *event)
{
    QModelIndex index = this->indexAt(event->pos());
    if (index.row() != -1 && m_pressIndex == index) {
        CommonService::TabletSelectMode model = CommonService::getInstance()->getSelectModel();

        if (!m_IsPlayQueue && model == CommonService::MultSelect) {
            MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();
            meta.beSelect = !meta.beSelect;
            QVariant varmeta;
            varmeta.setValue(meta);
            m_model->setData(m_model->index(index.row(), 0), varmeta, Qt::UserRole);
        }
    }
    QListView::mouseReleaseEvent(event);
}

//void PlayListView::reflushItemMediaMeta(const MediaMeta &meta)
//{
//    for (int i = 0; i <  m_model->rowCount(); i++) {
//        QModelIndex curIndex = m_model->index(i, 0);
//        MediaMeta metaTemp = curIndex.data(Qt::UserRole).value<MediaMeta>();

//        if (meta.hash == metaTemp.hash) {
//            QVariant mediaMeta;
//            mediaMeta.setValue(meta);
//            m_model->setData(curIndex, mediaMeta, Qt::UserRole);
//            break;
//        }
//    }
//}

void PlayListView::slotTextCodecMenuClicked(QAction *action)
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    if (modelIndexList.size() > 0) {
        QModelIndex curIndex = modelIndexList.at(0);
        MediaMeta meta = curIndex.data(Qt::UserRole).value<MediaMeta>();

        qDebug() << action->data().toByteArray();
        meta.codec = action->data().toByteArray();
        meta.updateCodec(action->data().toByteArray());
        QVariant varmeta;
        varmeta.setValue(meta);
        m_model->setData(curIndex, varmeta, Qt::UserRole);

        //restore to db
        DataBaseService::getInstance()->updateMetaCodec(meta);

        //同步编码到其他页面
        CommonService::getInstance()->signalUpdateCodec(meta);
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

void PlayListView::slotUpdateCodec(const MediaMeta &meta)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        MediaMeta tmpmeta = m_model->index(i, 0).data(Qt::UserRole).value<MediaMeta>();
        if (meta.hash == tmpmeta.hash) {
            QVariant varmeta;
            varmeta.setValue(meta);
            m_model->setData(m_model->index(i, 0), varmeta, Qt::UserRole);
            return;
        }
    }
}

void PlayListView::slotRmvCdaSongs()
{
    //remove cda songs
    if (m_IsPlayQueue) {
        for (int i = 0; i < m_model->rowCount();) {
            MediaMeta tmpmeta = m_model->index(i, 0).data(Qt::UserRole).value<MediaMeta>();
            if (tmpmeta.mmType == MIMETYPE_CDA) {
                m_model->removeRow(i);
            } else {
                i++;
            }
        }
    }
}

void PlayListView::slotSetSelectModel(CommonService::TabletSelectMode model)
{
    this->update();
    // 切换模式，清空选项
    tabletClearSelection();
}

void PlayListView::slotSelectAll()
{
    if (CommonService::getInstance()->isTabletEnvironment()) {
        for (int i = 0; i <  m_model->rowCount(); i++) {
            QModelIndex curIndex = m_model->index(i, 0);
            MediaMeta metaTemp = curIndex.data(Qt::UserRole).value<MediaMeta>();
            QVariant mediaMeta;
            metaTemp.beSelect = true;
            mediaMeta.setValue(metaTemp);
            m_model->setData(curIndex, mediaMeta, Qt::UserRole);
        }
    } else {
        this->selectAll();
    }
}

void PlayListView::slotShowSongList()
{
    SongListViewDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setWindowTitle(tr("Add to playlist"));
    dialog.m_songListView->addSongListItem("fav", tr("My favorites"), QIcon::fromTheme("music_mycollection"));

    if (!m_IsPlayQueue) {
        dialog.m_songListView->addSongListItem("play", tr("Play queue"), QIcon::fromTheme("music_famousballad"));
    }

    QList<DataBaseService::PlaylistData> strplaylist = DataBaseService::getInstance()->getCustomSongList();
    for (DataBaseService::PlaylistData pd : strplaylist) {
        if (m_currentHash != pd.uuid) { //filter itself
            dialog.m_songListView->addSongListItem(pd.uuid, pd.displayName, QIcon::fromTheme("music_famousballad"));
        }
    }

    connect(dialog.m_songListView, &SongListView::signalItemTriggered, this, &PlayListView::slotAddToSongList);
    dialog.exec();
}

void PlayListView::slotAddToSongList(const QString &hash, const QString &name)
{
    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }
    if (modelIndexList.size() == 0)
        return;
    QList<MediaMeta> metas;
    for (QModelIndex mindex : modelIndexList) {
        metas << (mindex.data(Qt::UserRole).value<MediaMeta>());
    }
    int insertCount = DataBaseService::getInstance()->addMetaToPlaylist(hash, metas);
    emit CommonService::getInstance()->signalShowPopupMessage(name, metas.size(), insertCount);
}

void PlayListView::slotHScreen(bool isHScreen)
{
    if (this->viewMode() == QListView::IconMode) {
        if (isHScreen) {
            setSpacing(20);
            setViewportMargins(30, -15, -35, 0);
        } else {
            setSpacing(33);
            setViewportMargins(23, -15, -35, 0);
        }
    }
}

void PlayListView::playMusic(const MediaMeta &meta)
{
    if (Player::getInstance()->getActiveMeta().hash == meta.hash) {
        if (Player::getInstance()->status() == Player::Paused) {
            Player::getInstance()->resume();
        } else if (Player::getInstance()->status() == Player::Stopped) {
            Player::getInstance()->playMeta(meta);
            // 通知播放队列列表改变
            emit Player::getInstance()->signalPlayListChanged();
            // 设置当前播放playlist的hash
            if (m_listPageType == SearchAlbumSubSongListType
                    || m_listPageType == SearchSingerSubSongListType) {
                // 搜索结果中播放hash为all
                Player::getInstance()->setCurrentPlayListHash("all", false);
            } else {
                Player::getInstance()->setCurrentPlayListHash(m_currentHash, false);
            }
        }
    } else {
        if (!m_IsPlayQueue) {
            // 设置新的播放列表
            Player::getInstance()->clearPlayList();
            Player::getInstance()->setPlayList(getMusicListData());
            // 通知播放队列列表改变
            emit Player::getInstance()->signalPlayListChanged();
            // 设置当前播放playlist的hash
            Player::getInstance()->setCurrentPlayListHash(m_currentHash, false);
        }
        Player::getInstance()->playMeta(meta);
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

    QModelIndexList modelIndexList;
    if (CommonService::getInstance()->isTabletEnvironment()) {
        modelIndexList =  this->tabletSelectedIndexes();
    } else {
        modelIndexList =  this->selectedIndexes();
    }

    for (QModelIndex index : modelIndexList) {
        selection.append(QItemSelectionRange(index));
    }

    if (!modelIndexList.isEmpty())
        scrollTo(modelIndexList.first());

    setAutoScroll(false);
    DListView::startDrag(supportedActions);
    setAutoScroll(true);

    if (!selection.isEmpty()) {
        selectionModel()->select(selection, QItemSelectionModel::Select);
    }
}

QModelIndexList PlayListView::tabletSelectedIndexes()
{
    QModelIndexList list;
    for (int i = 0; i <  m_model->rowCount(); i++) {
        QModelIndex curIndex = m_model->index(i, 0);
        MediaMeta metaTemp = curIndex.data(Qt::UserRole).value<MediaMeta>();
        if (metaTemp.beSelect) {
            list.append(curIndex);
        }
    }
    return list;
}

void PlayListView::tabletClearSelection()
{
    clearSelection();
    for (int i = 0; i <  m_model->rowCount(); i++) {
        QModelIndex curIndex = m_model->index(i, 0);
        MediaMeta metaTemp = curIndex.data(Qt::UserRole).value<MediaMeta>();
        QVariant mediaMeta;
        metaTemp.beSelect = false;
        mediaMeta.setValue(metaTemp);
        m_model->setData(curIndex, mediaMeta, Qt::UserRole);
    }
}

// 排序
void PlayListView::sortList(QList<MediaMeta> &musicInfos, const DataBaseService::ListSortType &sortType)
{
    switch (sortType) {
    case DataBaseService::SortByAddTimeASC: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanTimestampASC);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanTimestampASC);
        break;
    }
    case DataBaseService::SortByTitleASC: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanTitleASC);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanTitleASC);
        break;
    }
    case DataBaseService::SortBySingerASC: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanSingerASC);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanSingerASC);
        break;
    }
    case DataBaseService::SortByAblumASC: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanAblumASC);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanAblumASC);
        break;
    }
    case DataBaseService::SortByAddTimeDES: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanTimestampDES);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanTimestampDES);
        break;
    }
    case DataBaseService::SortByTitleDES: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanTitleDES);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanTitleDES);
        break;
    }
    case DataBaseService::SortBySingerDES: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanSingerDES);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanSingerDES);
        break;
    }
    case DataBaseService::SortByAblumDES: {
        std::sort(musicInfos.begin(), musicInfos.end(), moreThanAblumDES);
        //qSort(musicInfos.begin(), musicInfos.end(), moreThanAblumDES);
        break;
    }
    default:
        break;
    }
}

void PlayListView::setListPageSwitchType(ListPageSwitchType type)
{
    m_listPageType = type;
}

bool PlayListView::getIsPlayQueue()
{
    return m_IsPlayQueue;
}
