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
#include <QPushButton>
#include <QFocusEvent>
#include <QListWidget>
#include <DLabel>
#include <DPushButton>

#include <DUtil>
#include <DThemeManager>

#include "../core/playlist.h"
#include "widget/musiclistview.h"
#include "widget/musiclistitem.h"
#include "musiclistdatawidget.h"

DWIDGET_USE_NAMESPACE

MusicListWidget::MusicListWidget(QWidget *parent) : QFrame(parent)
{
    setObjectName("MusicListWidget");

    auto layout = new QHBoxLayout(this);
    setFocusPolicy(Qt::ClickFocus);
    layout->setContentsMargins(0, 0, 0, 15);
    layout->setSpacing(15);

    auto musicLayout = new QVBoxLayout(this);
    musicLayout->setContentsMargins(0, 0, 0, 15);
    musicLayout->setSpacing(15);

    DLabel *dataBaseLabel = new DLabel;
    dataBaseLabel->setText(tr("DataBase"));
    dataBaseLabel->setObjectName("MusicListWidgetDataBase");
    dataBaseLabel->setMargin(8);

    DLabel *customizeLabel = new DLabel;
    customizeLabel->setText(tr("Play List"));
    customizeLabel->setObjectName("MusicListWidgetCustomizeLabel");
    customizeLabel->setMargin(8);

    DPushButton *addListBtn = new DPushButton;
    addListBtn->setText("+");
    addListBtn->setFixedSize(24, 24);
    addListBtn->setFocusPolicy(Qt::NoFocus);

    auto customizeLayout = new QHBoxLayout(this);
    customizeLayout->addWidget(customizeLabel, 100, Qt::AlignLeft);
    customizeLayout->addStretch();
    customizeLayout->addWidget(addListBtn, 0, Qt::AlignRight);

    m_dataBaseListview = new MusicListView;
    m_dataBaseListview->setFixedHeight(230);
    m_customizeListview = new MusicListView;
    m_dataListView = new MusicListDataWidget;

    musicLayout->addWidget(dataBaseLabel, 0, Qt::AlignVCenter);
    musicLayout->addWidget(m_dataBaseListview, 0, Qt::AlignTop);
    musicLayout->addLayout(customizeLayout);
    musicLayout->addWidget(m_customizeListview, 100, Qt::AlignTop);

    layout->addLayout(musicLayout, 0);
    layout->addWidget(m_dataListView, 100);
    layout->addStretch();

    DThemeManager::instance()->registerWidget(this);

    connect(addListBtn, &DPushButton::clicked, this, [ = ](bool /*checked*/) {
        qDebug() << "addPlaylist(true);";
        Q_EMIT this->addPlaylist(true);
    });

    connect(m_dataBaseListview, &MusicListView::itemClicked,
    this, [ = ](QListWidgetItem * item) {
        this->setEnabled(false);
        auto playlistItem = qobject_cast<MusicListItem *>(m_dataBaseListview->itemWidget(item));
        if (!playlistItem) {
            qCritical() << "playlistItem is empty" << item << playlistItem;
            return;
        }

        m_dataListView->onMusiclistChanged(playlistItem->data());
//        Q_EMIT this->selectPlaylist(playlistItem->data());
        DUtil::TimerSingleShot(500, [this]() {
            this->setEnabled(true);
            Q_EMIT this->hidePlaylist();
        });
    });
    connect(m_dataBaseListview, &MusicListView::currentItemChanged,
    this, [ = ](QListWidgetItem * current, QListWidgetItem * previous) {
        auto itemWidget = qobject_cast<MusicListItem *>(m_dataBaseListview->itemWidget(previous));
        if (itemWidget) {
            itemWidget->setActive(false);
        }
        itemWidget = qobject_cast<MusicListItem *>(m_dataBaseListview->itemWidget(current));
        if (itemWidget) {
            itemWidget->setActive(true);
            m_dataListView->onMusiclistChanged(itemWidget->data());
        }
    });
    connect(m_dataBaseListview, &MusicListView::customResort,
    this, [ = ](const QStringList & uuids) {
        Q_EMIT this->customResort(uuids);
    });

    connect(m_customizeListview, &MusicListView::itemClicked,
    this, [ = ](QListWidgetItem * item) {
        this->setEnabled(false);
        auto playlistItem = qobject_cast<MusicListItem *>(m_customizeListview->itemWidget(item));
        if (!playlistItem) {
            qCritical() << "playlistItem is empty" << item << playlistItem;
            return;
        }

        m_dataListView->onMusiclistChanged(playlistItem->data());
//        Q_EMIT this->selectPlaylist(playlistItem->data());
        DUtil::TimerSingleShot(500, [this]() {
            this->setEnabled(true);
            Q_EMIT this->hidePlaylist();
        });
    });
    connect(m_customizeListview, &MusicListView::currentItemChanged,
    this, [ = ](QListWidgetItem * current, QListWidgetItem * previous) {
        auto itemWidget = qobject_cast<MusicListItem *>(m_customizeListview->itemWidget(previous));
        if (itemWidget) {
            itemWidget->setActive(false);
        }
        itemWidget = qobject_cast<MusicListItem *>(m_customizeListview->itemWidget(current));
        if (itemWidget) {
            itemWidget->setActive(true);
            m_dataListView->onMusiclistChanged(itemWidget->data());
        }
    });
    connect(m_customizeListview, &MusicListView::customResort,
    this, [ = ](const QStringList & uuids) {
        Q_EMIT this->customResort(uuids);
    });

    //musiclistdatawidget
    connect(m_dataListView, &MusicListDataWidget::playall,
    this, [ = ](PlaylistPtr playlist) {
//        Q_EMIT this->selectPlaylist(playlist);
        Q_EMIT this->playall(playlist);
    });
}

void MusicListWidget::initData(QList<PlaylistPtr > playlists, PlaylistPtr last)
{
    if (playlists.length() <= 0) {
        qCritical() << "playlist is empty";
        return;
    }

    QListWidgetItem *current = nullptr;
    for (auto &playlist : playlists) {
        if (playlist->hide()) {
            continue;
        }

        qDebug() << "init with" << playlist->id() << playlist->displayName();

        auto item = new QListWidgetItem;
        m_dataBaseListview->addItem(item);
        m_dataBaseListview->setItemWidget(item, new MusicListItem(playlist));

        auto playlistItem = qobject_cast<MusicListItem *>(m_dataBaseListview->itemWidget(item));
        connect(playlistItem, &MusicListItem::remove, this, [ = ]() {
            m_dataBaseListview->removeItemWidget(item);
            delete m_dataBaseListview->takeItem(m_dataBaseListview->row(item));
            // remote to firest
            Q_ASSERT(m_dataBaseListview->count() > 0);
            m_dataBaseListview->updateScrollbar();
            m_dataBaseListview->setCurrentItem(m_dataBaseListview->item(0));
        });

        connect(playlistItem, &MusicListItem::playall,
                this, &MusicListWidget::playall);

        if (last->id() == playlist->id()) {
            current = item;
        }

    }

    if (current) {
        m_dataBaseListview->setCurrentItem(current);
    } else {
        m_dataBaseListview->setCurrentItem(m_dataBaseListview->item(0));
    }
    m_dataBaseListview->updateScrollbar();
}

void MusicListWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr)
{
    for (int i = 0; i < m_dataBaseListview->count(); ++i) {
        QListWidgetItem *item = m_dataBaseListview->item(i);
        auto playlistItem = qobject_cast<MusicListItem *>(m_dataBaseListview->itemWidget(item));
        if (playlistItem->data()->id() == playlist->id()) {
            playlistItem->setPlay(true);
        } else {
            playlistItem->setPlay(false);
        }
    }
}

void MusicListWidget::focusOutEvent(QFocusEvent *event)
{
    // TODO: monitor mouse position
    QPoint mousePos = mapToParent(mapFromGlobal(QCursor::pos()));
//    qDebug() << mapFromGlobal(QCursor::pos()) << mousePos;
//    qDebug() << event->reason();
    if (!this->geometry().contains(mousePos)) {
        if (event && event->reason() == Qt::MouseFocusReason) {
            DUtil::TimerSingleShot(50, [this]() {
                qDebug() << "self lost focus hide";
                Q_EMIT this->hidePlaylist();
            });
        }
    }
    QFrame::focusOutEvent(event);
}

void MusicListWidget::onPlaylistAdded(PlaylistPtr playlist)
{
    if (playlist->hide()) {
        return;
    }

    auto item = new QListWidgetItem;

    if (playlist->id() == AlbumMusicListID || playlist->id() == ArtistMusicListID ||
            playlist->id() == AllMusicListID || playlist->id() == FavMusicListID) {
        m_dataBaseListview->addItem(item);
        m_dataBaseListview->setItemWidget(item, new MusicListItem(playlist));
        if (playlist->playing())
            m_dataBaseListview->setCurrentItem(item);
        m_dataBaseListview->updateScrollbar();

        auto playlistItem = qobject_cast<MusicListItem *>(m_dataBaseListview->itemWidget(item));
        connect(playlistItem, &MusicListItem::playall,
                this, &MusicListWidget::playall);
    } else {
        m_customizeListview->addItem(item);
        m_customizeListview->setItemWidget(item, new MusicListItem(playlist));

        auto playlistItem = qobject_cast<MusicListItem *>(m_customizeListview->itemWidget(item));
        connect(playlistItem, &MusicListItem::remove, this, [ = ]() {
            m_customizeListview->removeItemWidget(item);
            delete m_customizeListview->takeItem(m_customizeListview->row(item));
        });

        connect(playlistItem, &MusicListItem::playall,
                this, &MusicListWidget::playall);

        if (playlist->playing())
            m_customizeListview->setCurrentItem(item);
        m_customizeListview->updateScrollbar();
    }
    if (playlist->playing())
        m_dataListView->onMusiclistChanged(playlist);
}

void MusicListWidget::onCurrentChanged(PlaylistPtr playlist)
{
    if (playlist) {
        m_dataBaseListview->setCurrentItem(nullptr);
        m_customizeListview->setCurrentItem(nullptr);
    }
    for (int i = 0; i < m_dataBaseListview->count(); ++i) {
        QListWidgetItem *item = m_dataBaseListview->item(i);
        auto playlistItem = qobject_cast<MusicListItem *>(m_dataBaseListview->itemWidget(item));
        if (playlistItem->data()->id() == playlist->id()) {
            m_dataBaseListview->setCurrentItem(item);
            playlistItem->setActive(true);
        } else {
            playlistItem->setActive(false);
        }
    }
    for (int i = 0; i < m_customizeListview->count(); ++i) {
        QListWidgetItem *item = m_customizeListview->item(i);
        auto playlistItem = qobject_cast<MusicListItem *>(m_customizeListview->itemWidget(item));
        if (playlistItem->data()->id() == playlist->id()) {
            m_customizeListview->setCurrentItem(item);
            playlistItem->setActive(true);
        } else {
            playlistItem->setActive(false);
        }
    }
}

