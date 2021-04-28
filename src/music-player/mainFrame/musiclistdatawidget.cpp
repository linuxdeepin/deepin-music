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

#include "musiclistdatawidget.h"

#include <QDebug>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QStackedWidget>
#include <QShortcut>

#include <DLabel>
#include <DPushButton>
#include <DToolButton>
#include <DComboBox>
#include <DLabel>
#include <DFrame>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DApplicationHelper>

#include "util/pinyinsearch.h"

#include "widget/ddropdown.h"
#include "listView/musicInfoList/playlistview.h"
#include "widget/searchresulttabwidget.h"

#include "util/pinyinsearch.h"
#include "commonservice.h"
#include "albumlistview.h"
#include "singerlistview.h"
#include "databaseservice.h"
#include <malloc.h>
#include "ac-desktop-define.h"
#include "databaseservice.h"
#include "infodialog.h"
#include "player.h"
#include "subsonglistwidget.h"

DWIDGET_USE_NAMESPACE

MusicListDataWidget::MusicListDataWidget(QWidget *parent) :
    DWidget(parent)
{
    this->initUI();
    connect(CommonService::getInstance(), &CommonService::signalSwitchToView, this, &MusicListDataWidget::slotViewChanged);
    connect(CommonService::getInstance(), &CommonService::signalPlayAllMusic, this, &MusicListDataWidget::slotPlayAllClicked);
    connect(DataBaseService::getInstance(), &DataBaseService::signalImportFinished,
            this, &MusicListDataWidget::slotImportFinished);
    connect(DataBaseService::getInstance(), &DataBaseService::signalRmvSong,
            this, &MusicListDataWidget::slotRemoveSingleSong);
    connect(DataBaseService::getInstance(), &DataBaseService::signalPlaylistNameUpdate,
            this, &MusicListDataWidget::slotPlaylistNameUpdate);
    // 音乐列表数据变化
    connect(m_musicListView, &PlayListView::rowCountChanged, this, &MusicListDataWidget::slotMusicRowCountChanged);
}

MusicListDataWidget::~MusicListDataWidget()
{
}

void MusicListDataWidget::showEmptyHits(int count)
{
    if (count > 0) {
        m_contentWidget->show();
        m_emptyHits->hide();
    } else {
        m_contentWidget->hide();
        m_emptyHits->show();
    }
}

void MusicListDataWidget::initInfoLabel(QString hash)
{
    m_currentHash = hash;
    QString countStr;
    int songCount = 0;
    if (hash == "all" || hash == "musicResult") {
        if (hash == "musicResult") {
            songCount = m_searchResultTabWidget->getMusicCountByMusic();
            showEmptyHits(songCount);
            refreshSortAction("musicResult");
        } else {
            songCount = DataBaseService::getInstance()->allMusicInfosCount();
        }
        if (0 == songCount) {
            countStr = QString("   ") + MusicListDataWidget::tr("No songs");
        } else if (1 == songCount) {
            countStr = QString("   ") + MusicListDataWidget::tr("1 song");
        } else {
            countStr = QString("   ") + MusicListDataWidget::tr("%1 songs").arg(songCount);
        }
    }
    m_infoLabel->setText(countStr);
}

// 左侧菜单切换ListView
void MusicListDataWidget::slotViewChanged(ListPageSwitchType switchtype, const QString &hashOrSearchword, QMap<QString, MediaMeta> musicinfos)
{
    // 任意非0数，隐藏无搜索结果界面
    showEmptyHits(1);
    if (switchtype != AlbumSubSongListType
            && switchtype != SingerSubSongListType
            && switchtype != SearchAlbumSubSongListType
            && switchtype != SearchSingerSubSongListType) {
        m_actionBar->setVisible(true);
    }
    //设置dropdown使能
    if (m_musicDropdown) {
        m_musicDropdown->setEnabled(switchtype != CdaType ? true : false);
    }

    CommonService::getInstance()->setListPageSwitchType(switchtype);
    qDebug() << "------MusicListDataWidget::viewChanged switchtype = " << switchtype;
    switch (switchtype) {
    case AlbumType: {
        if (!m_albumListView) { // alloc
            m_albumListView = new AlbumListView("album", this);
            m_albumListView->setThemeType(m_musicListView->getThemeType());
            m_pStackedWidget->addWidget(m_albumListView);
            AC_SET_OBJECT_NAME(m_albumListView, AC_albumListView);
            AC_SET_ACCESSIBLE_NAME(m_albumListView, AC_albumListView);
        }
        m_albumListView->setAlbumListData(DataBaseService::getInstance()->allAlbumInfos()); //set album data
        m_albumListView->setViewModeFlag(m_albumListView->getViewMode());
        m_pStackedWidget->setCurrentWidget(m_albumListView);
        m_preHash = "album";
        m_preSwitchtype = AlbumType;
        m_titleLabel->setText(DataBaseService::getInstance()->getPlaylistNameByUUID("album"));
        refreshModeBtn(m_albumListView->viewMode());
        refreshInfoLabel("album");
        refreshSortAction("album");
        refreshPlayAllBtn(m_albumListView->getAlbumCount());
        break;
    }
    case SingerType: {
        if (!m_singerListView) {
            m_singerListView = new SingerListView("artist", this);
            m_singerListView->setThemeType(m_musicListView->getThemeType());
            m_pStackedWidget->addWidget(m_singerListView);
            AC_SET_OBJECT_NAME(m_singerListView, AC_singerListView);
            AC_SET_ACCESSIBLE_NAME(m_singerListView, AC_singerListView);
        }
        m_singerListView->setSingerListData(DataBaseService::getInstance()->allSingerInfos()); //set singer data
        m_singerListView->setViewModeFlag(m_singerListView->getViewMode());
        m_pStackedWidget->setCurrentWidget(m_singerListView);
        m_preHash = "artist";
        m_preSwitchtype = SingerType;
        m_titleLabel->setText(DataBaseService::getInstance()->getPlaylistNameByUUID("artist"));
        refreshModeBtn(m_singerListView->viewMode());
        refreshInfoLabel("artist");
        refreshSortAction("artist");
        refreshPlayAllBtn(m_singerListView->getSingerCount());
        break;
    }
    case AllSongListType: {
        m_musicListView->reloadAllSonglist();
        m_titleLabel->setText(DataBaseService::getInstance()->getPlaylistNameByUUID("all"));
        m_musicListView->setViewModeFlag("all", m_musicListView->getViewMode());
        refreshModeBtn(m_musicListView->getViewMode());
        refreshInfoLabel("all");
        m_pStackedWidget->setCurrentWidget(m_musicListView);
        m_preHash = "all";
        m_preSwitchtype = AllSongListType;
        refreshSortAction("all");
        refreshPlayAllBtn(m_musicListView->getMusicCount());
        break;
    }
    case FavType: {
        m_musicListView->initCostomSonglist("fav");
        m_titleLabel->setText(DataBaseService::getInstance()->getPlaylistNameByUUID("fav"));
        m_musicListView->setViewModeFlag("fav", m_musicListView->getViewMode());
        m_pStackedWidget->setCurrentWidget(m_musicListView);
        m_preHash = "fav";
        m_preSwitchtype = FavType;
        refreshModeBtn(m_musicListView->getViewMode());
        refreshInfoLabel("fav");
        refreshPlayAllBtn(m_musicListView->getMusicCount());
        refreshSortAction("fav");
        break;
    }
    case CdaType: {
        m_musicListView->initCostomSonglist(hashOrSearchword);
        m_titleLabel->setText(tr("CD playlist"));
        m_musicListView->setViewModeFlag(hashOrSearchword, m_musicListView->getViewMode());
        m_pStackedWidget->setCurrentWidget(m_musicListView);
        m_preHash = hashOrSearchword;
        m_preSwitchtype = CdaType;
        refreshModeBtn(m_musicListView->getViewMode());
        refreshInfoLabel(hashOrSearchword);
        refreshPlayAllBtn(m_musicListView->getMusicCount());
        refreshSortAction();
        break;
    }
    case CustomType: {
        m_musicListView->initCostomSonglist(hashOrSearchword);
        QFontMetrics titleFm(m_titleLabel->font());
        QString text = titleFm.elidedText(DataBaseService::getInstance()->getPlaylistNameByUUID(hashOrSearchword), Qt::ElideRight, 300);
        m_titleLabel->setText(text);
        m_musicListView->setViewModeFlag(hashOrSearchword, m_musicListView->getViewMode());
        m_pStackedWidget->setCurrentWidget(m_musicListView);
        m_preHash = hashOrSearchword;
        m_preSwitchtype = CustomType;
        refreshModeBtn(m_musicListView->getViewMode());
        refreshInfoLabel(hashOrSearchword);
        refreshPlayAllBtn(m_musicListView->getMusicCount());
        refreshSortAction(hashOrSearchword);
        break;
    }
    case SearchMusicResultType:
    case SearchSingerResultType:
    case SearchAlbumResultType: {
        // 搜索歌曲结果
        if (!m_searchResultTabWidget) {
            m_searchResultTabWidget = new SearchResultTabWidget(this);
            AC_SET_OBJECT_NAME(m_searchResultTabWidget, AC_searchResultTabWidget);
            AC_SET_ACCESSIBLE_NAME(m_searchResultTabWidget, AC_searchResultTabWidget);

            m_pStackedWidget->addWidget(m_searchResultTabWidget);
            connect(m_searchResultTabWidget, &SearchResultTabWidget::sigSearchTypeChanged,
                    this, &MusicListDataWidget::refreshInfoLabel);
            connect(m_searchResultTabWidget, &SearchResultTabWidget::sigSearchTypeChanged,
                    this, &MusicListDataWidget::refreshModeBtnByHash);
        }
        m_pStackedWidget->setCurrentWidget(m_searchResultTabWidget);
        m_titleLabel->setText(tr("Search Results"));
        m_searchResultTabWidget->refreshListview(switchtype, hashOrSearchword);
        m_searchResultTabWidget->setCurrentPage(switchtype);
        refreshPlayAllBtn(1);//搜索界面，重置播放所有为可点击状态
        if (switchtype == SearchMusicResultType) {
            refreshInfoLabel("musicResult");
            refreshSortAction("musicResult");
        } else if (switchtype == SearchSingerResultType) {
            refreshInfoLabel("artistResult");
            refreshSortAction("artistResult");
        } else if (switchtype == SearchAlbumResultType) {
            refreshInfoLabel("albumResult");
            refreshSortAction("albumResult");
        }
        refreshModeBtn(m_searchResultTabWidget->getViewMode());
        break;
    }
    case PreType: {
        // 返回进入二级页面前的页面
        if (m_preSwitchtype == AlbumSubSongListType) {
            slotViewChanged(AlbumType, "album", QMap<QString, MediaMeta>());
        } else if (m_preSwitchtype == SingerSubSongListType) {
            slotViewChanged(SingerType, "artist", QMap<QString, MediaMeta>());
        } else if (m_preSwitchtype == SearchAlbumSubSongListType) {
            m_pStackedWidget->setCurrentWidget(m_searchResultTabWidget);
        } else if (m_preSwitchtype == SearchSingerSubSongListType) {
            m_pStackedWidget->setCurrentWidget(m_searchResultTabWidget);
        } else {
            slotViewChanged(m_preSwitchtype, m_preHash, QMap<QString, MediaMeta>());
        }
        break;
    }
    case AlbumSubSongListType:
    case SingerSubSongListType:
    case SearchAlbumSubSongListType:
    case SearchSingerSubSongListType: {
        m_preSwitchtype = switchtype;
        m_actionBar->setVisible(false);
        if (m_subSonglistWidget == nullptr) {
            // 添加二级页面
            m_subSonglistWidget = new SubSonglistWidget("", this);
            AC_SET_OBJECT_NAME(m_subSonglistWidget, AC_subSonglistWidget);
            AC_SET_ACCESSIBLE_NAME(m_subSonglistWidget, AC_subSonglistWidget);
            m_pStackedWidget->addWidget(m_subSonglistWidget);
        }
        if (musicinfos.size() > 0) {
            m_subSonglistWidget->flushDialog(musicinfos, switchtype);
        }
        m_pStackedWidget->setCurrentWidget(m_subSonglistWidget);
        break;
    }
    default:
        refreshSortAction();
        break;
    }
    if (CommonService::getInstance()->isTabletEnvironment()) {
        emit CommonService::getInstance()->setSelectModel(CommonService::SingleSelect);
    }
}

void MusicListDataWidget::switchViewModel()
{
    DToolButton *ptb = static_cast<DToolButton *>(sender());
    if (!ptb)
        return;

    if (m_pStackedWidget->currentWidget() == m_albumListView) {
        m_albumListView->setViewModeFlag(ptb == m_btIconMode ?
                                         DListView::IconMode : DListView::ListMode);
    } else if (m_pStackedWidget->currentWidget() == m_singerListView) {
        m_singerListView->setViewModeFlag(ptb == m_btIconMode ?
                                          DListView::IconMode : DListView::ListMode);
    } else if (m_pStackedWidget->currentWidget() == m_musicListView) {
        m_musicListView->setViewModeFlag(m_musicListView->getCurrentHash(), ptb == m_btIconMode ?
                                         DListView::IconMode : DListView::ListMode);
    } else if (m_pStackedWidget->currentWidget() == m_searchResultTabWidget) {
        m_searchResultTabWidget->setViewMode(ptb == m_btIconMode ?
                                             DListView::IconMode : DListView::ListMode);
    }

    m_btIconMode->setChecked(ptb == m_btIconMode);
    m_btlistMode->setChecked(ptb != m_btIconMode);
}

void MusicListDataWidget::slotSortChange(QAction *action)
{
    DDropdown *ptb = static_cast<DDropdown *>(sender());
    if (!ptb)
        return;
    ptb->setCurrentAction(action);
    DataBaseService::ListSortType sortType = action->data().value<DataBaseService::ListSortType>();
    if (m_pStackedWidget->currentWidget() == m_musicListView) {
        m_musicListView->setSortType(sortType);
    } else if (m_pStackedWidget->currentWidget() == m_albumListView) {
        m_albumListView->setSortType(sortType);
        m_albumListView->update();
    } else if (m_pStackedWidget->currentWidget() == m_singerListView) {
        m_singerListView->setSortType(sortType);
    } else if (m_pStackedWidget->currentWidget() == m_searchResultTabWidget) {
        m_searchResultTabWidget->setSortType(sortType);
    }
}

void MusicListDataWidget::slotImportFinished(QString hash, int successCount)
{
    Q_UNUSED(hash)

    if (successCount <= 0) {
        return;
    }
    qDebug() << "---MusicListDataWidget::slotImportFinished m_currentHash = " << m_currentHash;
    refreshInfoLabel(m_currentHash);
    if (m_currentHash == "all" || m_currentHash == "album" || m_currentHash == "artist") {
        if (m_albumListView && m_pStackedWidget->currentWidget() == m_albumListView) {
            m_albumListView->setAlbumListData(DataBaseService::getInstance()->allAlbumInfos()); //set album data
        } else if (m_singerListView && m_pStackedWidget->currentWidget() == m_singerListView) {
            m_singerListView->setSingerListData(DataBaseService::getInstance()->allSingerInfos()); //set singer data
        }
    }
}

bool MusicListDataWidget::eventFilter(QObject *o, QEvent *e)
{
//    if (o == d->btPlayAll) {
//        if (e->type() == QEvent::FocusIn) {

//            Q_EMIT changeFocus("AllMusicListID");
//        }
//    } else if (o == d->btIconMode) {
//        if (e->type() == QEvent::KeyPress) {
//            QKeyEvent *event = static_cast<QKeyEvent *>(e);
//            if (event->key() == Qt::Key_Return) {

//                Q_EMIT d->btIconMode->click();
//            }
//        }
//    }  else  if (o == d->btlistMode) {
//        if (e->type() == QEvent::KeyPress) {
//            QKeyEvent *event = static_cast<QKeyEvent *>(e);
//            if (event->key() == Qt::Key_Return) {

//                Q_EMIT d->btlistMode->click();
//            }
//        } else if (e->type() == QEvent::FocusOut) {

//            Q_EMIT changeFocus("btlistModeFocusOut");
//        }
//    } else if (o == d->musicListView) {

//        if (e->type() == QEvent::KeyPress) {
//            QKeyEvent *event = static_cast<QKeyEvent *>(e);
//            if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_M)) {

//                int rowIndex = d->musicListView->currentIndex().row();
//                int row = 40 * rowIndex;
//                QPoint pos;

//                if (row > 440) {
//                    QPoint posm(300, 220);
//                    pos = posm;
//                } else {
//                    QPoint posm(300, row);
//                    pos = posm;
//                }

//                Q_EMIT requestCustomContextMenu(pos, 1);
//            }
//        } else if (e->type() == QEvent::FocusIn) {

//            int rowIndex = d->musicListView->currentIndex().row();

//            if (rowIndex == -1) {
//                auto index = d->musicListView->item(0, 0);
//                d->musicListView->setCurrentItem(index);
//            }

//        } else if (e->type() == QEvent::FocusOut) {

//        }
//    }

    return QWidget::eventFilter(o, e);
}

void MusicListDataWidget::dragEnterEvent(QDragEnterEvent *event)
{
    DWidget::dragEnterEvent(event);
    if (event->mimeData()->hasFormat("text/uri-list")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }
}

void MusicListDataWidget::dropEvent(QDropEvent *event)
{
    // 不需要向下传递事件
//    DWidget::dropEvent(event);

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << url.toLocalFile();
    }

    if (!localpaths.isEmpty()) {
        DataBaseService::getInstance()->importMedias(m_currentHash, localpaths);
    }
}
// 大标题跟随resize变化
void MusicListDataWidget::resizeEvent(QResizeEvent *event)
{
    m_lableWidget->setGeometry(m_actionBar->geometry());
    return QWidget::resizeEvent(event);
}

void MusicListDataWidget::slotPlayAllClicked()
{
    switch (CommonService::getInstance()->getListPageSwitchType()) {
    case AlbumType: {
        // 清空播放队列
        Player::getInstance()->clearPlayList();
        // 添加到播放列表
        QList<MediaMeta> allMediaMetas = DataBaseService::getInstance()->allMusicInfos();
        QList<MediaMeta> playMediaMetas;
        QStringList metaList = DataBaseService::getInstance()->getDelMetaHashs();
        if (DataBaseService::getInstance()->getDelStatus()) {
            for (MediaMeta meta : allMediaMetas) {
                if (!metaList.contains(meta.hash)) {
                    playMediaMetas.append(meta);
                }
            }
            Player::getInstance()->setPlayList(playMediaMetas);
        } else {
            Player::getInstance()->setPlayList(allMediaMetas);
        }
        // 查找第一首歌
        MediaMeta playMeta;
        QList<AlbumInfo> albumInfos =  m_albumListView->getAlbumListData();
        if (DataBaseService::getInstance()->getDelStatus()) {
            for (int i = 0; i < albumInfos.size(); i++) {
                AlbumInfo albumTmp = albumInfos.at(i);
                QMap<QString, MediaMeta> albumTmpMap = albumTmp.musicinfos;
                for (QMap<QString, MediaMeta>::Iterator iterator = albumTmpMap.begin(); iterator != albumTmpMap.end(); iterator++) {
                    if (!metaList.contains((*iterator).hash)) {
                        playMeta = (*iterator);
                        break;
                    }
                }
//                for (MediaMeta meta : albumTmp.musicinfos.values()) {
//                    if (!metaList.contains(meta.hash)) {
//                        playMeta = meta;
//                        break;
//                    }
//                }
                if (!playMeta.hash.isEmpty()) {
                    break;
                }
            }
        } else {
            if (albumInfos.size() > 0) {
                playMeta = albumInfos.at(0).musicinfos.values().at(0);
            }
        }

        // 通知播放队列改变
        Player::getInstance()->setCurrentPlayListHash(m_currentHash, false);
        emit Player::getInstance()->signalPlayListChanged();

        // 设置第一首播放音乐
        if (Player::getInstance()->getPlayList()->size() > 0) {
            Player::getInstance()->playMeta(playMeta);
        }
        break;
    }
    case SingerType: {
        // 清空播放队列
        Player::getInstance()->clearPlayList();
        // 添加到播放列表
        QList<MediaMeta> allMediaMetas = DataBaseService::getInstance()->allMusicInfos();
        QList<MediaMeta> playMediaMetas;
        QStringList metaList = DataBaseService::getInstance()->getDelMetaHashs();
        if (DataBaseService::getInstance()->getDelStatus()) {
            for (MediaMeta meta : allMediaMetas) {
                if (!metaList.contains(meta.hash)) {
                    playMediaMetas.append(meta);
                }
            }
            Player::getInstance()->setPlayList(playMediaMetas);
        } else {
            Player::getInstance()->setPlayList(allMediaMetas);
        }
        // 查找第一首歌
        MediaMeta playMeta;
        QList<SingerInfo> singerInfos =  m_singerListView->getSingerListData();
        if (DataBaseService::getInstance()->getDelStatus()) {
            for (int i = 0; i < singerInfos.size(); i++) {
                SingerInfo singerTmp = singerInfos.at(i);
                QMap<QString, MediaMeta> singerTmpMap = singerTmp.musicinfos;
                for (QMap<QString, MediaMeta>::Iterator iterator = singerTmpMap.begin(); iterator != singerTmpMap.end(); iterator++) {
                    if (!metaList.contains((*iterator).hash)) {
                        playMeta = (*iterator);
                        break;
                    }
                }
//                for (MediaMeta meta : singerTmp.musicinfos.values()) {
//                    if (!metaList.contains(meta.hash)) {
//                        playMeta = meta;
//                        break;
//                    }
//                }
                if (!playMeta.hash.isEmpty()) {
                    break;
                }
            }
        } else {
            if (singerInfos.size() > 0) {
                playMeta = singerInfos.at(0).musicinfos.values().at(0);
            }
        }

        // 通知播放队列改变
        Player::getInstance()->setCurrentPlayListHash(m_currentHash, false);
        emit Player::getInstance()->signalPlayListChanged();

        // 设置第一首播放音乐
        if (Player::getInstance()->getPlayList()->size() > 0) {
            Player::getInstance()->playMeta(playMeta);
        }
        break;
    }
    // 同下共用
    case AllSongListType:
    // 同下共用
    case FavType:
    case CdaType:
    case CustomType:
        // 清空播放队列
        Player::getInstance()->clearPlayList();
        // 添加到播放列表
        for (auto meta : m_musicListView->getMusicListData()) {
            Player::getInstance()->playListAppendMeta(meta);
        }

        // 通知播放队列改变
        Player::getInstance()->setCurrentPlayListHash(m_currentHash, false);
        emit Player::getInstance()->signalPlayListChanged();

        // 设置第一首播放音乐
        if (Player::getInstance()->getPlayList()->size() > 0) {
            Player::getInstance()->playMeta(Player::getInstance()->getPlayList()->first());
        }
        break;
    case SearchMusicResultType:
    case SearchSingerResultType:
    case SearchAlbumResultType:
        // 清空播放队列
        Player::getInstance()->clearPlayList();
        // 添加到播放列表
        for (auto meta : m_searchResultTabWidget->getMusicListData()) {
            Player::getInstance()->playListAppendMeta(meta);
        }

        // 通知播放队列改变
        Player::getInstance()->setCurrentPlayListHash(m_currentHash, false);
        emit Player::getInstance()->signalPlayListChanged();

        // 设置第一首播放音乐
        if (Player::getInstance()->getPlayList()->size() > 0) {
            Player::getInstance()->playMeta(Player::getInstance()->getPlayList()->first());
        }
        break;
    default:
        break;
    }
}

void MusicListDataWidget::slotMusicRowCountChanged()
{
    // 当前页面为歌曲列表时刷新按钮使能状态
    if (m_musicListView && m_pStackedWidget->currentWidget() == m_musicListView) {
        refreshPlayAllBtn(m_musicListView->getMusicCount());
    }
}

void MusicListDataWidget::initUI()
{
    setObjectName("MusicListDataWidget");
    setAcceptDrops(true);
    m_currentHash = "all";

    setAutoFillBackground(true);
    auto palette = this->palette();
    QColor background("#FFFFFF");
    background.setAlphaF(0.1);
    palette.setColor(DPalette::Background, background);
    setPalette(palette);

    // 用来和搜索时无结果的时候做切换，老代码里所有控件写visible太冗长
    QVBoxLayout *layoutContent = new QVBoxLayout(this);
    layoutContent->setContentsMargins(0, 0, 0, 0);
    layoutContent->setSpacing(0);
    m_contentWidget = new DWidget(this);
    m_contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutContent->addWidget(m_contentWidget, 1);

    QVBoxLayout *layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    // action widget
    m_actionBar = new ActionBar;
    // 按设计修改
    m_actionBar->setFixedHeight(60);
    m_actionBar->setObjectName("MusicListDataActionBar");
    layout->addWidget(m_actionBar, 0);
    m_pStackedWidget = new QStackedWidget(this);
    layout->addWidget(m_pStackedWidget, 1);

    // action layout
    QHBoxLayout *actionInfoBarLayout = new QHBoxLayout(m_actionBar);
    if (CommonService::getInstance()->isTabletEnvironment()) {
        actionInfoBarLayout->setContentsMargins(50, 0, 50, 0);
    } else {
        actionInfoBarLayout->setContentsMargins(10, 0, 8, 0);
    }
    actionInfoBarLayout->setSpacing(0);
    // 初始化全部播放按钮
    initBtPlayAll(actionInfoBarLayout);
    // 初始化数量标签
    initCountLabel(actionInfoBarLayout);
    // 两边控件撑满
    actionInfoBarLayout->addStretch(100);
    // 初始化大标题
    initTitle(actionInfoBarLayout);
    // 初始化列表模式
    initListIconMode(actionInfoBarLayout);
    // 初始化专辑排序action
    initAlbumAction(actionInfoBarLayout);
    // 初始化演唱者排序action
    initArtistAction(actionInfoBarLayout);
    // 初始化歌曲排序action
    initMusicAction(actionInfoBarLayout);

    // 初始化搜索结果为空时的标签
    initemptyHits(layoutContent);

    // 启动首页
    m_musicListView = new PlayListView("all", false);

    AC_SET_OBJECT_NAME(m_musicListView, AC_PlayListView);
    AC_SET_ACCESSIBLE_NAME(m_musicListView, AC_PlayListView);
    m_musicListView->initAllSonglist("all");
    m_musicListView->setFocusPolicy(Qt::StrongFocus);
    m_pStackedWidget->addWidget(m_musicListView);
//    m_pCenterWidget->setMouseTracking(true);
    m_pStackedWidget->setCurrentWidget(m_musicListView);
    initInfoLabel("all");
    refreshSortAction();
    slotTheme(DGuiApplicationHelper::instance()->themeType());
}
// 初始化播放所有按钮
void MusicListDataWidget::initBtPlayAll(QHBoxLayout *layout)
{
    m_btPlayAll = new DPushButton;
    auto playAllPalette = m_btPlayAll->palette();
    playAllPalette.setColor(DPalette::ButtonText, Qt::white);
    playAllPalette.setColor(DPalette::Dark, QColor("#FD5E5E"));
    playAllPalette.setColor(DPalette::Light, QColor("#ED5656"));
    m_btPlayAll->setPalette(playAllPalette);
    m_btPlayAll->setIcon(QIcon::fromTheme("play_all"));
    m_btPlayAll->setObjectName(AC_musicListDataPlayAll);
    m_btPlayAll->setText(tr("Play All"));
    if (CommonService::getInstance()->isTabletEnvironment()) {
        m_btPlayAll->setFixedSize(QSize(100, 40));
    } else {
        m_btPlayAll->setFixedSize(QSize(93, 30));
        // 字体宽度加图标大于93,重新设置按钮宽度
        QFontMetrics font(m_btPlayAll->font());
        m_btPlayAll->setFixedWidth((font.width(m_btPlayAll->text()) + 18) >= 93 ? (font.width(m_btPlayAll->text()) + 38) : 93);
    }
    m_btPlayAll->setFocusPolicy(Qt::NoFocus);
    m_btPlayAll->setIconSize(QSize(18, 18));

    m_btPlayAll->setFocusPolicy(Qt::TabFocus);
    m_btPlayAll->setDefault(true);
    m_btPlayAll->installEventFilter(this);

    DFontSizeManager::instance()->bind(m_btPlayAll, DFontSizeManager::T6, QFont::Medium);
    layout->addWidget(m_btPlayAll, 0, Qt::AlignLeft | Qt::AlignVCenter);

    connect(m_btPlayAll, &DPushButton::clicked, this, &MusicListDataWidget::slotPlayAllClicked);
}
// 初始化数量标签
void MusicListDataWidget::initCountLabel(QHBoxLayout *layout)
{
    m_infoLabel = new DLabel;
    m_infoLabel->setObjectName("MusicListDataTitle");
    m_infoLabel->setText(tr("All Music"));
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setForegroundRole(DPalette::ButtonText);
    DFontSizeManager::instance()->bind(m_infoLabel, DFontSizeManager::T6, QFont::Medium);

    layout->addWidget(m_infoLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
}
// 初始化大标题
void MusicListDataWidget::initTitle(QHBoxLayout *layout)
{
    Q_UNUSED(layout)
    m_lableWidget = new QWidget(m_actionBar);
    m_lableWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QHBoxLayout *lableLayout = new QHBoxLayout(m_lableWidget);
    lableLayout->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new DLabel;
    QFont titleFont = m_titleLabel->font();
    titleFont.setFamily("SourceHanSansSC");
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setFixedHeight(36);
    m_titleLabel->setFixedWidth(300);
    m_titleLabel->setObjectName("MusicListDataTitle");
    m_titleLabel->setText(tr("All Music"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setForegroundRole(DPalette::BrightText);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T3, QFont::Medium);

    m_lableWidget->setGeometry(m_actionBar->geometry());
    lableLayout->addWidget(m_titleLabel, 100, Qt::AlignCenter);
}
// 初始化列表模式
void MusicListDataWidget::initListIconMode(QHBoxLayout *layout)
{
    // icon模式
    m_btIconMode = new DToolButton;
    m_btIconMode->setIcon(QIcon::fromTheme("picture_list_texts"));
    m_btIconMode->setIconSize(QSize(36, 36));
    m_btIconMode->setFixedSize(36, 36);
    AC_SET_OBJECT_NAME(m_btIconMode, AC_btIconMode);
    AC_SET_ACCESSIBLE_NAME(m_btIconMode, AC_btIconMode);
    m_btIconMode->setCheckable(true);
    m_btIconMode->setChecked(false);
    m_btIconMode->setFocusPolicy(Qt::TabFocus);
    m_btIconMode->installEventFilter(this);
    // 列表模式
    m_btlistMode = new DToolButton;
    m_btlistMode->setIcon(QIcon::fromTheme("text_list_texts"));
    m_btlistMode->setIconSize(QSize(36, 36));
    m_btlistMode->setFixedSize(36, 36);
    m_btlistMode->setCheckable(true);
    m_btlistMode->setChecked(true);
    m_btlistMode->setFocusPolicy(Qt::TabFocus);
    m_btlistMode->installEventFilter(this);
    AC_SET_OBJECT_NAME(m_btlistMode, AC_btlistMode);
    AC_SET_ACCESSIBLE_NAME(m_btlistMode, AC_btlistMode);

    layout->addWidget(m_btIconMode, 0, Qt::AlignRight);
    layout->addWidget(m_btlistMode, 0, Qt::AlignRight);

    connect(m_btIconMode, &DToolButton::clicked, this, &MusicListDataWidget::switchViewModel);
    connect(m_btlistMode, &DToolButton::clicked, this, &MusicListDataWidget::switchViewModel);
}

void MusicListDataWidget::initAlbumAction(QHBoxLayout *layout)
{
    m_albumDropdown = new DDropdown;
    m_albumDropdown->setObjectName("MusicListAlbumDataSort");
    m_albumDropdown->addAction(tr("Time added"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortByAddTime));
    m_albumDropdown->addAction(tr("Album"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortByTitle));
    m_albumDropdown->setCurrentAction();
    m_albumDropdown->hide();
    layout->addWidget(m_albumDropdown, 0, Qt::AlignRight | Qt::AlignVCenter);
    connect(m_albumDropdown, &DDropdown::triggered, this, &MusicListDataWidget::slotSortChange);

    AC_SET_OBJECT_NAME(m_albumDropdown, AC_albumDropdown);
    AC_SET_ACCESSIBLE_NAME(m_albumDropdown, AC_albumDropdown);
}

void MusicListDataWidget::initArtistAction(QHBoxLayout *layout)
{
    m_artistDropdown = new DDropdown;
    m_artistDropdown->setObjectName("MusicListArtistDataSort");
    m_artistDropdown->addAction(tr("Time added"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortByAddTime));
    m_artistDropdown->addAction(tr("Artist"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortBySinger));
    m_artistDropdown->setCurrentAction();
    m_artistDropdown->hide();
    layout->addWidget(m_artistDropdown, 0, Qt::AlignRight | Qt::AlignVCenter);
    connect(m_artistDropdown, &DDropdown::triggered, this, &MusicListDataWidget::slotSortChange);

    AC_SET_OBJECT_NAME(m_artistDropdown, AC_artistDropdown);
    AC_SET_ACCESSIBLE_NAME(m_artistDropdown, AC_artistDropdown);
}

void MusicListDataWidget::initMusicAction(QHBoxLayout *layout)
{
    m_musicDropdown = new DDropdown;
    m_musicDropdown->setObjectName("MusicListMusicDataSort");
    m_musicDropdown->addAction(tr("Time added"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortByAddTime));
    m_musicDropdown->addAction(tr("Title"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortByTitle));
    m_musicDropdown->addAction(tr("Artist"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortBySinger));
    m_musicDropdown->addAction(tr("Album"), QVariant::fromValue<DataBaseService::ListSortType>(DataBaseService::SortByAblum));
    m_musicDropdown->setCurrentAction();
    layout->addWidget(m_musicDropdown, 0, Qt::AlignRight | Qt::AlignVCenter);
    connect(m_musicDropdown, &DDropdown::triggered, this, &MusicListDataWidget::slotSortChange);

    AC_SET_OBJECT_NAME(m_musicDropdown, AC_musicDropdown);
    AC_SET_ACCESSIBLE_NAME(m_musicDropdown, AC_musicDropdown);
}

void MusicListDataWidget::initemptyHits(QVBoxLayout *layout)
{
    m_emptyHits = new DLabel();
    m_emptyHits->setObjectName("MusicListDataEmptyHits");
    m_emptyHits->hide();
    auto emptyHitsFont = m_emptyHits->font();
    emptyHitsFont.setFamily("SourceHanSansSC");
// 解决字体不会根据系统字体大小改变问题
//    emptyHitsFont.setPixelSize(20);
    m_emptyHits->setFont(emptyHitsFont);
    m_emptyHits->setText(MusicListDataWidget::tr("No search results"));

    auto emptyLayout = new QVBoxLayout();
    emptyLayout->setContentsMargins(0, 0, 0, 0);
    emptyLayout->setSpacing(10);

    emptyLayout->addStretch(100);
    emptyLayout->addWidget(m_emptyHits, 0, Qt::AlignCenter);
    emptyLayout->addStretch(100);

    layout->addLayout(emptyLayout, 0);

// 这段代码导致了ListView底部空白问题
//    layout->addSpacing(8);
}

void MusicListDataWidget::refreshModeBtn(DListView::ViewMode mode)
{
    m_btIconMode->setChecked(mode == DListView::IconMode);
    m_btlistMode->setChecked(mode == DListView::ListMode);
}

void MusicListDataWidget::refreshInfoLabel(QString hash)
{
    m_currentHash = hash;
    QString countStr;
    int singerCount = 0;
    int songCount = 0;
    if (hash == "album" || hash == "albumResult") {
        int albumCount = 0;
        if (hash == "albumResult") {
            songCount = m_searchResultTabWidget->getMusicCountByAlbum();
            albumCount = m_searchResultTabWidget->getAlbumCount();
            showEmptyHits(songCount);
            refreshSortAction("albumResult");
        } else {
            albumCount = DataBaseService::getInstance()->allAlbumInfos().size();
            songCount = DataBaseService::getInstance()->allMusicInfos(false).size();
        }

        if (songCount == 0) {
            countStr = QString("   ") + MusicListDataWidget::tr("No songs");
        } else if (songCount == 1) {
            countStr = QString("   ") + MusicListDataWidget::tr("1 album - 1 song");
        } else {
            if (albumCount == 1) {
                countStr = QString("   ") + MusicListDataWidget::tr("%1 album - %2 songs").arg(albumCount).arg(songCount);
            } else {
                countStr = QString("   ") + MusicListDataWidget::tr("%1 albums - %2 songs").arg(albumCount).arg(songCount);
            }
        }
    } else if (hash == "artist" || hash == "artistResult") {
        if (hash == "artistResult") {
            songCount = m_searchResultTabWidget->getMusicCountBySinger();
            singerCount = m_searchResultTabWidget->getSingerCount();
            showEmptyHits(songCount);
            refreshSortAction("artistResult");
        } else {
            singerCount = DataBaseService::getInstance()->allSingerInfos().size();
            songCount = DataBaseService::getInstance()->allMusicInfos().size();
        }
        if (songCount == 0) {
            countStr = QString("   ") + MusicListDataWidget::tr("No songs");
        } else if (songCount == 1) {
            countStr = QString("   ") + MusicListDataWidget::tr("1 artist - 1 song");
        } else {
            if (singerCount == 1) {
                countStr = QString("   ") + MusicListDataWidget::tr("%1 artist - %2 songs").arg(singerCount).arg(songCount);
            } else {
                countStr = QString("   ") + MusicListDataWidget::tr("%1 artists - %2 songs").arg(singerCount).arg(songCount);
            }
        }
    } else if (hash == "all" || hash == "musicResult") {
        if (hash == "musicResult") {
            songCount = m_searchResultTabWidget->getMusicCountByMusic();
            showEmptyHits(songCount);
            refreshSortAction("musicResult");
        } else {
            songCount = m_musicListView->getMusicCount();
        }
        if (0 == songCount) {
            countStr = QString("   ") + MusicListDataWidget::tr("No songs");
        } else if (1 == songCount) {
            countStr = QString("   ") + MusicListDataWidget::tr("1 song");
        } else {
            countStr = QString("   ") + MusicListDataWidget::tr("%1 songs").arg(songCount);
        }
    } else  {
        //自定义歌单一定有hash
        if (hash.isEmpty()) {
            return;
        }
        songCount = m_musicListView->getMusicCount();
        if (0 == songCount) {
            countStr = QString("   ") + MusicListDataWidget::tr("No songs");
        } else if (1 == songCount) {
            countStr = QString("   ") + MusicListDataWidget::tr("1 song");
        } else {
            countStr = QString("   ") + MusicListDataWidget::tr("%1 songs").arg(songCount);
        }
    }
    m_infoLabel->setText(countStr);
}

void MusicListDataWidget::refreshModeBtnByHash(const QString &hash)
{
    if (hash == "albumResult" || hash == "artistResult" || hash == "musicResult") {
        refreshModeBtn(m_searchResultTabWidget->getViewMode());
    }
}
// 根据当前页面中内容数量刷新播放所有按钮状态
void MusicListDataWidget::refreshPlayAllBtn(int count)
{
    auto playAllPalette = m_btPlayAll->palette();
    QColor color(Qt::white);
    if (count > 0) {
        m_btPlayAll->setEnabled(true);
    } else {
        m_btPlayAll->setEnabled(false);
        color.setAlphaF(0.4);
    }
    playAllPalette.setColor(DPalette::ButtonText, color);
    m_btPlayAll->setPalette(playAllPalette);
}

void MusicListDataWidget::slotRemoveSingleSong(const QString &listHash, const QString &musicHash)
{
    Q_UNUSED(listHash)
    Q_UNUSED(musicHash)
    refreshInfoLabel(m_currentHash);
}

void MusicListDataWidget::slotPlaylistNameUpdate(const QString &listHash)
{
    if (m_currentHash != listHash) {
        return;
    }
    QFontMetrics titleFm(m_titleLabel->font());
    QString text = titleFm.elidedText(DataBaseService::getInstance()->getPlaylistNameByUUID(m_currentHash), Qt::ElideRight, 300);
    m_titleLabel->setText(text);
}

void MusicListDataWidget::refreshSortAction(const QString &hash)
{
    if (m_pStackedWidget->currentWidget() == m_musicListView ||
            (hash == "musicResult")) {
        m_musicDropdown->setVisible(true);
        m_albumDropdown->setVisible(false);
        m_artistDropdown->setVisible(false);
        for (int i = 0; i < m_musicDropdown->actions().size(); i++) {
            QAction *action = m_musicDropdown->actions().at(i);
            DataBaseService::ListSortType sortType = action->data().value<DataBaseService::ListSortType>();
            if (CommonService::getInstance()->getListPageSwitchType() == CdaType) {
                if (sortType == DataBaseService::SortByTitleASC
                        || sortType == DataBaseService::SortByTitleDES
                        || sortType == DataBaseService::SortByTitle) {
                    m_musicDropdown->setCurrentAction(action);
                    break;
                }
            } else {
                DataBaseService::ListSortType searchSortType = (hash == "musicResult" ? m_searchResultTabWidget->getSortType() : m_musicListView->getSortType());
                switch (searchSortType) {
                case DataBaseService::SortByAddTimeASC:
                case DataBaseService::SortByAddTimeDES: {
                    searchSortType = DataBaseService::SortByAddTime;
                    break;
                }
                case DataBaseService::SortByTitleASC:
                case DataBaseService::SortByTitleDES: {
                    searchSortType = DataBaseService::SortByTitle;
                    break;
                }
                case DataBaseService::SortByAblumASC:
                case DataBaseService::SortByAblumDES: {
                    searchSortType = DataBaseService::SortByAblum;
                    break;
                }
                case DataBaseService::SortBySingerASC:
                case DataBaseService::SortBySingerDES: {
                    searchSortType = DataBaseService::SortBySinger;
                    break;
                }
                default:
                    break;
                }
                if (sortType == searchSortType) {
                    m_musicDropdown->setCurrentAction(action);
                    break;
                }
            }
        }
    } else if (m_pStackedWidget->currentWidget() == m_albumListView ||
               (hash == "albumResult")) {
        m_musicDropdown->setVisible(false);
        m_albumDropdown->setVisible(true);
        m_artistDropdown->setVisible(false);
        for (int i = 0; i < m_albumDropdown->actions().size(); i++) {
            QAction *action = m_albumDropdown->actions().at(i);
            DataBaseService::ListSortType sortType = action->data().value<DataBaseService::ListSortType>();
            if (sortType == (hash == "albumResult" ? m_searchResultTabWidget->getSortType() : m_albumListView->getSortType())) {
                m_albumDropdown->setCurrentAction(action);
            }
        }
    } else if (m_pStackedWidget->currentWidget() == m_singerListView ||
               (hash == "artistResult")) {
        m_musicDropdown->setVisible(false);
        m_albumDropdown->setVisible(false);
        m_artistDropdown->setVisible(true);
        for (int i = 0; i < m_artistDropdown->actions().size(); i++) {
            QAction *action = m_artistDropdown->actions().at(i);
            DataBaseService::ListSortType sortType = action->data().value<DataBaseService::ListSortType>();
            if (sortType == (hash == "artistResult" ? m_searchResultTabWidget->getSortType() : m_singerListView->getSortType())) {
                m_artistDropdown->setCurrentAction(action);
            }
        }
    }
}

void MusicListDataWidget::slotTheme(int type)
{
    if (type == 1) {
        auto palette = this->palette();
        QColor background("#FFFFFF");
        background.setAlphaF(0.1);
        palette.setColor(DPalette::Background, background);
        setPalette(palette);

        auto emptyHitsPalette = m_emptyHits->palette();
        QColor titleLabelPaletteColor("#000000");
        titleLabelPaletteColor.setAlphaF(0.3);
        emptyHitsPalette.setColor(QPalette::WindowText, titleLabelPaletteColor);
        m_emptyHits->setPalette(emptyHitsPalette);
        m_emptyHits->setForegroundRole(QPalette::WindowText);

        auto playAllPalette = m_btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, Qt::white);
        playAllPalette.setColor(DPalette::Light, QColor("#FD5E5E"));
        playAllPalette.setColor(DPalette::Dark, QColor("#ED5656"));
        m_btPlayAll->setPalette(playAllPalette);

        auto infoLabelPalette = m_infoLabel->palette();
        // 根据设计重新设置颜色
        QColor infoLabelColor(QColor("#777777"));
//        infoLabelColor.setAlphaF(0.7);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        DApplicationHelper::instance()->setPalette(m_infoLabel, infoLabelPalette);
    } else {
        auto palette = this->palette();
        QColor background("#252525");
        palette.setColor(DPalette::Background, background);
        setPalette(palette);

        auto emptyHitsPalette = m_emptyHits->palette();
        QColor titleLabelPaletteColor("#C0C6D4");
        titleLabelPaletteColor.setAlphaF(0.4);
        emptyHitsPalette.setColor(QPalette::WindowText, titleLabelPaletteColor);
        m_emptyHits->setPalette(emptyHitsPalette);
        m_emptyHits->setForegroundRole(QPalette::WindowText);


        auto playAllPalette = m_btPlayAll->palette();
        playAllPalette.setColor(DPalette::ButtonText, "#FFFFFF");
        playAllPalette.setColor(DPalette::Light, QColor("#DA2D2D"));
        playAllPalette.setColor(DPalette::Dark, QColor("#A51B1B"));
        m_btPlayAll->setPalette(playAllPalette);

        auto infoLabelPalette = m_infoLabel->palette();
//        QColor infoLabelColor = infoLabelPalette.color(DPalette::BrightText);
//        infoLabelColor.setAlphaF(0.7);
        // 根据设计重新设置颜色
        QColor infoLabelColor(QColor("#FFFFFF"));
        infoLabelColor.setAlphaF(0.7);
        infoLabelPalette.setColor(DPalette::ButtonText, infoLabelColor);
        m_infoLabel->setForegroundRole(DPalette::ButtonText);
        DApplicationHelper::instance()->setPalette(m_infoLabel, infoLabelPalette);
    }
    if (m_albumListView != nullptr && m_pStackedWidget->currentWidget() == m_albumListView) {
        refreshPlayAllBtn(m_albumListView->getAlbumCount());
    } else if (m_singerListView != nullptr && m_pStackedWidget->currentWidget() == m_singerListView) {
        refreshPlayAllBtn(m_singerListView->getSingerCount());
    } else if (m_musicListView != nullptr && m_pStackedWidget->currentWidget() == m_musicListView) {
        refreshPlayAllBtn(m_musicListView->getMusicCount());
    }
}

ActionBar::ActionBar(QWidget *parent)
{
    Q_UNUSED(parent)
    MoveFlag = false;
}

void ActionBar::mouseReleaseEvent(QMouseEvent *event)
{
    MoveFlag = true;
    DWidget::mouseReleaseEvent(event);
}

void ActionBar::mousePressEvent(QMouseEvent *event)
{
    MoveFlag = false;
    DWidget::mousePressEvent(event);
}
void ActionBar::mouseMoveEvent(QMouseEvent *event)
{
    if (MoveFlag)
        DWidget::mousePressEvent(event);
}

