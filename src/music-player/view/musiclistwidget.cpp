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

#include "musiclistwidget.h"

#include <QDebug>

#include <QVBoxLayout>
#include <QFocusEvent>

#include <DListWidget>
#include <DLabel>
#include <DPushButton>
#include <DFloatingButton>
#include <DPalette>
#include <DGuiApplicationHelper>
#include <DUtil>

#include "../core/playlist.h"
#include "../core/musicsettings.h"
#include "widget/musiclistview.h"
#include "widget/musiclistviewitem.h"
#include "musiclistdatawidget.h"
#include "widget/musicimagebutton.h"
#include "musiclistscrollarea.h"

MusicListWidget::MusicListWidget(QWidget *parent) : DWidget(parent)
{
    setObjectName("MusicListWidget");

    setAutoFillBackground(true);
    auto palette = this->palette();
    palette.setColor(DPalette::Background, QColor("#F8F8F8"));
    setPalette(palette);

    auto layout = new QHBoxLayout(this);
    setFocusPolicy(Qt::ClickFocus);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    leftFrame = new MusicListScrollArea;
    leftFrame->setFixedWidth(220);
    leftFrame->setAutoFillBackground(true);
    auto leftFramePalette = leftFrame->palette();
    leftFramePalette.setColor(DPalette::Background, QColor("#FFFFFF"));
    leftFrame->setPalette(leftFramePalette);

    m_addListBtn = leftFrame->getAddButton();

    m_dataBaseListview = leftFrame->getDBMusicListView();
    m_customizeListview = leftFrame->getCustomMusicListView();
    m_dataListView = new MusicListDataWidget;

    layout->addWidget(leftFrame, 0);
    layout->addWidget(m_dataListView, 100);

//    bool themeFlag = false;

    int themeType = DGuiApplicationHelper::instance()->themeType();
    slotTheme(themeType);

    connect(this, &MusicListWidget::seaResult,
            m_dataListView,  &MusicListDataWidget::retResult);

    connect(this, &MusicListWidget::seaResult, this, [ = ]() {
        m_customizeListview->clearSelected();
        m_customizeListview->closeAllPersistentEditor();

        m_dataBaseListview->clearSelected();
        m_dataBaseListview->closeAllPersistentEditor();
    });

    connect(this, &MusicListWidget::closeSearch,
            m_dataListView,  &MusicListDataWidget::CloseSearch);

    connect(m_addListBtn, &DPushButton::clicked, this, [ = ](bool /*checked*/) {
        qDebug() << "addPlaylist(true);";
        addFlag = true;
        Q_EMIT this->addPlaylist(true);
    });

    connect(m_dataBaseListview, &MusicListView::pressed,
    this, [ = ](const QModelIndex & index) {
        auto curPtr = m_dataBaseListview->playlistPtr(index);
        if (curPtr != nullptr) {
            m_customizeListview->clearSelected();
            m_customizeListview->closeAllPersistentEditor();
            m_dataListView->selectMusiclistChanged(curPtr);
            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
        m_dataBaseListview->setFocus();
    });
    connect(m_dataBaseListview, &MusicListView::currentChanged,
    this, [ = ](const QModelIndex & current, const QModelIndex & previous) {
        Q_UNUSED(previous)
        auto curPtr = m_dataBaseListview->playlistPtr(current);
        if (curPtr != nullptr) {
            m_customizeListview->clearSelected();
            m_customizeListview->closeAllPersistentEditor();
            m_dataListView->selectMusiclistChanged(curPtr);
            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
        m_dataBaseListview->setFocus();
    });
    connect(m_dataBaseListview, &MusicListView::customResort,
    this, [ = ](const QStringList & uuids) {

        Q_EMIT this->customResort(uuids);

    });
    connect(m_dataBaseListview, &MusicListView::playall,
    this, [ = ](PlaylistPtr playlist) {
        if (playlist->playing() == nullptr) {
            if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID) {
                playlist->playMusicTypeToMeta();
                playlist->play(playlist->first());
            }
            Q_EMIT this->playall(playlist);
        } else {
            Q_EMIT this->resume(playlist, playlist->playing());
        }

    });
    connect(m_dataBaseListview, &MusicListView::pause,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT this->pause(playlist, meta);
    });
    connect(m_dataBaseListview, &MusicListView::importSelectFiles,
    this, [ = ](PlaylistPtr playlist, QStringList urllist) {
        Q_EMIT this->importSelectFiles(playlist, urllist);
    });
    connect(m_dataBaseListview, &MusicListView::addToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        Q_EMIT this->addToPlaylist(playlist, metalist);
    });

    connect(m_customizeListview, &MusicListView::pressed,
    this, [ = ](const QModelIndex & index) {
        auto curPtr = m_customizeListview->playlistPtr(index);
        if (curPtr != nullptr) {
            m_dataBaseListview->clearSelected();
            m_dataBaseListview->closeAllPersistentEditor();
            m_dataListView->selectMusiclistChanged(curPtr);

            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
        m_customizeListview->setFocus();
    });
    connect(m_customizeListview, &MusicListView::currentChanged,
    this, [ = ](const QModelIndex & current, const QModelIndex & previous) {
        Q_UNUSED(previous)
        auto curPtr = m_customizeListview->playlistPtr(current);
        if (curPtr != nullptr) {
            m_dataBaseListview->clearSelected();
            m_dataBaseListview->closeAllPersistentEditor();
            m_dataListView->selectMusiclistChanged(curPtr);

            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
        m_customizeListview->setFocus();
    });
    connect(m_customizeListview, &MusicListView::removeAllList,
    this, [ = ](const MetaPtr meta) {
        auto current = m_dataBaseListview->item(2);
        auto curPtr = m_dataBaseListview->playlistPtr(current);
        if (curPtr != nullptr) {
            if (meta != nullptr) {
                curPtr->play(meta);
                m_dataBaseListview->setCurPlaylist(current);
                m_dataBaseListview->setCurrentItem(current);
            }
            m_customizeListview->clearSelected();
            m_customizeListview->closeAllPersistentEditor();
            m_dataListView->selectMusiclistChanged(curPtr);
            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
    });
    connect(m_customizeListview, &MusicListView::customResort,
    this, [ = ](const QStringList & uuids) {
        Q_EMIT this->customResort(uuids);
    });
    connect(m_customizeListview, &MusicListView::playall,
    this, [ = ](PlaylistPtr playlist) {
        if (playlist->playing() == nullptr) {
            Q_EMIT this->playall(playlist);
        } else {
            Q_EMIT this->resume(playlist, playlist->playing());
        }
    });
    connect(m_customizeListview, &MusicListView::displayNameChanged,
    this, [ = ]() {
        m_dataListView->onMusiclistUpdate();
    });
    connect(m_customizeListview, &MusicListView::pause,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT this->pause(playlist, meta);
    });
    connect(m_customizeListview, &MusicListView::importSelectFiles,
    this, [ = ](PlaylistPtr playlist, QStringList urllist) {
        Q_EMIT this->importSelectFiles(playlist, urllist);
    });
    connect(m_customizeListview, &MusicListView::addToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        Q_EMIT this->addToPlaylist(playlist, metalist);
    });

    //musiclistdatawidget
    connect(m_dataListView, &MusicListDataWidget::playall,
    this, [ = ](PlaylistPtr playlist) {
//        Q_EMIT this->selectPlaylist(playlist);
        Q_EMIT this->playall(playlist);
    });
    connect(m_dataListView, &MusicListDataWidget::playMedia,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT this->playMedia(playlist, meta);
    });
    connect(m_dataListView, &MusicListDataWidget::resume,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT this->resume(playlist, meta);
    });
    connect(m_dataListView, &MusicListDataWidget::pause,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT this->pause(playlist, meta);
    });
    connect(m_dataListView, &MusicListDataWidget::resort,
    this, [ = ](PlaylistPtr playlist, int sortType) {
        Q_EMIT this->resort(playlist, sortType);
    });
    connect(m_dataListView, &MusicListDataWidget::requestCustomContextMenu,
    this, [ = ](const QPoint & pos, char type) {
        Q_EMIT this->requestCustomContextMenu(pos, type);
    });
    connect(m_dataListView, &MusicListDataWidget::addToPlaylist,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        Q_EMIT this->addToPlaylist(playlist, metalist);
    });
    connect(m_dataListView, &MusicListDataWidget::musiclistRemove,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList  & metalist) {
        Q_EMIT this->musiclistRemove(playlist, metalist);
    });
    connect(m_dataListView, &MusicListDataWidget::musiclistDelete,
    this, [ = ](PlaylistPtr playlist, const MetaPtrList  & metalist) {
        Q_EMIT this->musiclistDelete(playlist, metalist);
    });
    connect(m_dataListView, &MusicListDataWidget::showInfoDialog,
    this, [ = ](const MetaPtr meta) {
        Q_EMIT this->showInfoDialog(meta);
    });
    connect(m_dataListView, &MusicListDataWidget::updateMetaCodec,
    this, [ = ](const QString & preTitle, const QString & preArtist, const QString & preAlbum, const MetaPtr  meta) {
        Q_EMIT this->updateMetaCodec(preTitle, preArtist, preAlbum, meta);
    });
    connect(m_dataListView, &MusicListDataWidget::modeChanged,
    this, [ = ](int mode) {
        Q_EMIT this->modeChanged(mode);
    });
    connect(m_dataListView, &MusicListDataWidget::importSelectFiles,
    this, [ = ](PlaylistPtr playlist, QStringList urllist) {
        Q_EMIT this->importSelectFiles(playlist, urllist);
    });
    connect(m_dataListView, &MusicListDataWidget::addMetasFavourite,
    this, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT addMetasFavourite(metalist);
    });
    connect(m_dataListView, &MusicListDataWidget::removeMetasFavourite,
    this, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT removeMetasFavourite(metalist);
    });
}

PlaylistPtr MusicListWidget::curPlaylist()
{
    PlaylistPtr curPlaylist = nullptr;
    if (m_dataListView != nullptr)
        curPlaylist = m_dataListView->curPlaylist();
    return curPlaylist;
}

void MusicListWidget::onSearchText(QString str)
{
    m_dataListView->onSearchText(str);
}


void MusicListWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    m_dataListView->onMusicPlayed(playlist, meta);
    onCurrentChanged(playlist);
}

void MusicListWidget::focusOutEvent(QFocusEvent *event)
{
    // TODO: monitor mouse position
    QPoint mousePos = mapToParent(mapFromGlobal(QCursor::pos()));
    if (!this->geometry().contains(mousePos)) {
        if (event && event->reason() == Qt::MouseFocusReason) {
            DUtil::TimerSingleShot(50, [this]() {
                qDebug() << "self lost focus hide";
                Q_EMIT this->hidePlaylist();
            });
        }
    }
    DWidget::focusOutEvent(event);
}

void MusicListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        m_dataBaseListview->closeAllPersistentEditor();
        m_dataBaseListview->clearSelection();
        m_dataBaseListview->setCurrentItem(nullptr);
        m_customizeListview->closeAllPersistentEditor();
        m_customizeListview->clearSelection();
        m_customizeListview->setCurrentItem(nullptr);
    }
    DWidget::keyPressEvent(event);
}

void MusicListWidget::onPlaylistAdded(PlaylistPtr playlist, bool newflag)
{
    if (playlist == nullptr) {
        return;
    }
    if (playlist->hide()) {
        return;
    }

    if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID ||
            playlist->id() == AllMusicListID || playlist->id() == FavMusicListID) {
        m_dataBaseListview->addMusicList(playlist);
    } else {
        m_customizeListview->closeAllPersistentEditor();
        m_customizeListview->addMusicList(playlist, newflag);
        addFlag = false;
    }
//    auto item = new MusicListViewItem(playlist);
//    if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID ||
//            playlist->id() == AllMusicListID || playlist->id() == FavMusicListID ) {
//        m_dataBaseListview->addItem(item);
//    } else {
//        m_customizeListview->addItem(item);
//        if (addFlag) {
//            m_customizeListview->clearSelection();
//            m_customizeListview->openPersistentEditor(item);
//        }
//        addFlag = false;
//    }
    if (playlist->playing())
        m_dataListView->onMusiclistChanged(playlist);
}

void MusicListWidget::onCurrentChanged(PlaylistPtr playlist)
{
    if (playlist->id() != SearchMusicListID) {
        if (playlist) {
//            m_dataBaseListview->clearSelection();
//            m_customizeListview->clearSelection();
        }
        for (int i = 0; i < m_dataBaseListview->count(); ++i) {
            auto *item = m_dataBaseListview->item(i);
            auto curPlaylist = m_dataBaseListview->playlistPtr(item);
            if (playlist == curPlaylist) {
                Q_EMIT selectedPlaylistChange(curPlaylist);
                m_dataBaseListview->setCurPlaylist(item);
                m_customizeListview->setCurPlaylist(nullptr);
            }
        }
        for (int i = 0; i < m_customizeListview->count(); ++i) {
            auto *item = m_customizeListview->item(i);
            auto curPlaylist = m_customizeListview->playlistPtr(item);
            if (playlist == curPlaylist) {
                Q_EMIT selectedPlaylistChange(curPlaylist);
                m_customizeListview->setCurPlaylist(item);
                m_dataBaseListview->setCurPlaylist(nullptr);
            }
        }
    } else {
        onMusiclistUpdate();
    }
}

void MusicListWidget::onMusiclistChanged(PlaylistPtr playlist)
{
    m_dataListView->onMusiclistChanged(playlist);
}

void MusicListWidget::onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_UNUSED(playlist)
    Q_UNUSED(metalist)
    m_dataListView->onMusicListAdded(playlist, metalist);
}

void MusicListWidget::onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_UNUSED(playlist)
    Q_UNUSED(metalist)

    m_dataListView->onMusicListRemoved(playlist, metalist);
}

void MusicListWidget::onMusiclistUpdate()
{
    m_dataListView->onMusiclistUpdate();
}

void MusicListWidget::slotTheme(int type)
{
    if (type == 0)
        type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#F8F8F8"));
        setPalette(palette);
    } else {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#252525"));
        setPalette(palette);
    }

    leftFrame->slotTheme(type);
    m_dataListView->slotTheme(type);
    m_dataBaseListview->slotTheme(type);
    m_customizeListview->slotTheme(type);
}

void MusicListWidget::changePicture(QPixmap pixmap, QPixmap albumPixmap, QPixmap sidebarPixmap)
{
    leftFrame->changePicture(pixmap, albumPixmap, sidebarPixmap);
    m_dataListView->changePicture(pixmap, sidebarPixmap, albumPixmap);
}

void MusicListWidget::onCustomContextMenuRequest(const QPoint &pos, PlaylistPtr selectedlist, PlaylistPtr favlist, QList<PlaylistPtr> newlists, char type)
{
    m_dataListView->onCustomContextMenuRequest(pos, selectedlist, favlist, newlists, type);
}

