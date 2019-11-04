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

#include <DUtil>

#include "../core/playlist.h"
#include "../core/musicsettings.h"
#include "widget/musiclistview.h"
#include "widget/musiclistviewitem.h"
#include "musiclistdatawidget.h"
#include "widget/musicimagebutton.h"
#include <DGuiApplicationHelper>
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

    leftFrame = new DWidget;
    leftFrame->setFixedWidth(220);
    leftFrame->setAutoFillBackground(true);
    auto leftFramePalette = leftFrame->palette();
    leftFramePalette.setColor(DPalette::Background, QColor("#FFFFFF"));
    leftFrame->setPalette(leftFramePalette);

    auto musicLayout = new QVBoxLayout(leftFrame);
    musicLayout->setContentsMargins(10, 5, 10, 5);
    musicLayout->setSpacing(0);

    dataBaseLabel = new DLabel;
    dataBaseLabel->setFixedHeight(40);
    dataBaseLabel->setText(tr("DataBase"));
    dataBaseLabel->setObjectName("MusicListWidgetDataBase");
    dataBaseLabel->setMargin(10);
    auto dataBaseLabelFont = dataBaseLabel->font();
    dataBaseLabelFont.setFamily("SourceHanSansSC-Medium");
    dataBaseLabelFont.setPixelSize(14);
    dataBaseLabel->setFont(dataBaseLabelFont);
    auto dataBaseLabelPalette = dataBaseLabel->palette();
    dataBaseLabelPalette.setColor(DPalette::ButtonText, QColor("#757F92"));
    dataBaseLabel->setPalette(dataBaseLabelPalette);

    customizeLabel = new DLabel;
    customizeLabel->setFixedHeight(40);
    customizeLabel->setText(tr("Play List"));
    customizeLabel->setObjectName("MusicListWidgetCustomizeLabel");
    customizeLabel->setMargin(10);
    customizeLabel->setFont(dataBaseLabelFont);
    customizeLabel->setPalette(dataBaseLabelPalette);

    m_addListBtn = new MusicImageButton(":/mpimage/light/normal/add_normal.svg",
                                        ":/mpimage/light/hover/add_hover.svg",
                                        ":/mpimage/light/press/add_press.svg");
    m_addListBtn->setFixedSize(28, 28);
    m_addListBtn->setFocusPolicy(Qt::NoFocus);

    auto customizeLayout = new QHBoxLayout(this);
    customizeLayout->setContentsMargins(0, 0, 10, 0);
    customizeLayout->addWidget(customizeLabel, 100, Qt::AlignLeft);
    customizeLayout->addStretch();
    customizeLayout->addWidget(m_addListBtn, 0, Qt::AlignRight);

    m_dataBaseListview = new MusicListView;
    m_dataBaseListview->setEditTriggers(QAbstractItemView::EditKeyPressed);
    m_dataBaseListview->setFixedHeight(162);
    m_customizeListview = new MusicListView;
    m_dataListView = new MusicListDataWidget;

    musicLayout->addWidget(dataBaseLabel, 0, Qt::AlignVCenter);
    musicLayout->addWidget(m_dataBaseListview, 0, Qt::AlignTop);
    musicLayout->addLayout(customizeLayout);
    musicLayout->addWidget(m_customizeListview);

    layout->addWidget(leftFrame, 0);
    layout->addWidget(m_dataListView, 100);

    bool themeFlag = false;
    int themeType = MusicSettings::value("base.play.theme").toInt(&themeFlag);
    if (!themeFlag)
        themeType = 1;
    slotTheme(themeType);

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
            m_dataListView->onMusiclistChanged(curPtr);
//            DUtil::TimerSingleShot(500, [this]() {
//                Q_EMIT this->hidePlaylist();
//            });
            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
    });
    connect(m_dataBaseListview, &MusicListView::currentChanged,
    this, [ = ](const QModelIndex & current, const QModelIndex & previous) {
        auto curPtr = m_dataBaseListview->playlistPtr(current);
        if (curPtr != nullptr) {
            m_customizeListview->clearSelected();
            m_customizeListview->closeAllPersistentEditor();
            m_dataListView->onMusiclistChanged(curPtr);
//            DUtil::TimerSingleShot(500, [this]() {
//                Q_EMIT this->hidePlaylist();
//            });
            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
    });
    connect(m_dataBaseListview, &MusicListView::customResort,
    this, [ = ](const QStringList & uuids) {
        Q_EMIT this->customResort(uuids);
    });
    connect(m_dataBaseListview, &MusicListView::playall,
    this, [ = ](PlaylistPtr playlist) {
        if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID) {
            playlist->playMusicTypeToMeta();
            playlist->play(playlist->first());
        }
        Q_EMIT this->playall(playlist);
    });
    connect(m_dataBaseListview, &MusicListView::pause,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT this->pause(playlist, meta);
    });

    connect(m_customizeListview, &MusicListView::pressed,
    this, [ = ](const QModelIndex & index) {
        auto curPtr = m_customizeListview->playlistPtr(index);
        if (curPtr != nullptr) {
            m_dataBaseListview->clearSelected();
            m_dataBaseListview->closeAllPersistentEditor();
            m_dataListView->onMusiclistChanged(curPtr);
//            DUtil::TimerSingleShot(500, [this]() {
//                Q_EMIT this->hidePlaylist();
//            });
            curPtr->setSearchStr("");
            Q_EMIT selectedPlaylistChange(curPtr);
        }
    });
    connect(m_customizeListview, &MusicListView::currentChanged,
    this, [ = ](const QModelIndex & current, const QModelIndex & previous) {
        auto curPtr = m_customizeListview->playlistPtr(current);
        if (curPtr != nullptr) {
            m_dataBaseListview->clearSelected();
            m_dataBaseListview->closeAllPersistentEditor();
            m_dataListView->onMusiclistChanged(curPtr);
//            DUtil::TimerSingleShot(500, [this]() {
//                Q_EMIT this->hidePlaylist();
//            });
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
        Q_EMIT this->playall(playlist);
    });
    connect(m_customizeListview, &MusicListView::displayNameChanged,
    this, [ = ]() {
        m_dataListView->onMusiclistUpdate();
    });
    connect(m_customizeListview, &MusicListView::pause,
    this, [ = ](PlaylistPtr playlist, const MetaPtr meta) {
        Q_EMIT this->pause(playlist, meta);
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
    this, [ = ](const MetaPtr  meta) {
        Q_EMIT this->updateMetaCodec(meta);
    });
    connect(m_dataListView, &MusicListDataWidget::modeChanged,
    this, [ = ](int mode) {
        Q_EMIT this->modeChanged(mode);
    });
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

void MusicListWidget::onPlaylistAdded(PlaylistPtr playlist)
{
    if (playlist->hide()) {
        return;
    }

    if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID ||
            playlist->id() == AllMusicListID || playlist->id() == FavMusicListID ) {
        m_dataBaseListview->addMusicList(playlist);
    } else {
        m_customizeListview->closeAllPersistentEditor();
        m_customizeListview->addMusicList(playlist, addFlag);
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
            m_dataBaseListview->clearSelection();
            m_customizeListview->clearSelection();
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
    m_dataListView->onMusiclistUpdate();
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

        DPalette leftFramePalette = leftFrame->palette();
        leftFramePalette.setColor(DPalette::Background, QColor("#FFFFFF"));
        leftFrame->setPalette(leftFramePalette);

        DPalette dataBaseLabelPalette = dataBaseLabel->palette();
        dataBaseLabelPalette.setColor(DPalette::ButtonText, QColor("#757F92"));
        dataBaseLabel->setPalette(dataBaseLabelPalette);
        customizeLabel->setPalette(dataBaseLabelPalette);

        DPalette dataBaseListviewPalette = m_dataBaseListview->palette();
        dataBaseListviewPalette.setColor(DPalette::Text, QColor("#414D68"));
        dataBaseListviewPalette.setColor(DPalette::HighlightedText, QColor("#FFFFFF"));
        m_dataBaseListview->setPalette(dataBaseListviewPalette);
        m_customizeListview->setPalette(dataBaseListviewPalette);

        m_addListBtn->setPropertyPic(":/mpimage/light/normal/add_normal.svg",
                                     ":/mpimage/light/hover/add_hover.svg",
                                     ":/mpimage/light/press/add_press.svg");
    } else {
        auto palette = this->palette();
        palette.setColor(DPalette::Background, QColor("#252525"));
        setPalette(palette);

        auto leftFramePalette = leftFrame->palette();
        leftFramePalette.setColor(DPalette::Background, QColor("#232323"));
        leftFrame->setPalette(leftFramePalette);

        auto dataBaseLabelPalette = dataBaseLabel->palette();
        dataBaseLabelPalette.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
        dataBaseLabel->setPalette(dataBaseLabelPalette);
        customizeLabel->setPalette(dataBaseLabelPalette);

        auto dataBaseListviewPalette = m_dataBaseListview->palette();
        dataBaseListviewPalette.setColor(DPalette::Text, QColor("#C0C6D4"));
        dataBaseListviewPalette.setColor(DPalette::HighlightedText, QColor("#FFFFFF"));
        m_dataBaseListview->setPalette(dataBaseListviewPalette);
        m_customizeListview->setPalette(dataBaseListviewPalette);

        m_addListBtn->setPropertyPic(":/mpimage/dark/normal/add_normal.svg",
                                     ":/mpimage/dark/hover/add_hover.svg",
                                     ":/mpimage/dark/press/add_press.svg");
    }

    m_dataListView->slotTheme(type);
    m_dataBaseListview->slotTheme(type);
    m_customizeListview->slotTheme(type);
}

void MusicListWidget::changePicture(QPixmap pixmap)
{
    m_dataBaseListview->changePicture(pixmap);
    m_customizeListview->changePicture(pixmap);
    m_dataListView->changePicture(pixmap);
}

void MusicListWidget::onCustomContextMenuRequest(const QPoint &pos, PlaylistPtr selectedlist, PlaylistPtr favlist, QList<PlaylistPtr> newlists, char type)
{
    m_dataListView->onCustomContextMenuRequest(pos, selectedlist, favlist, newlists, type);
}

