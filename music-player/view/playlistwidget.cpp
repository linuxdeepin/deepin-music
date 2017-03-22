/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlistwidget.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFocusEvent>

#include <DUtil>
#include <thememanager.h>

#include "../core/playlist.h"
#include "widget/playlistview.h"
#include "widget/playlistitem.h"

DWIDGET_USE_NAMESPACE

PlaylistWidget::PlaylistWidget(QWidget *parent) : QFrame(parent)
{
    setObjectName("PlaylistWidget");

    auto layout = new QVBoxLayout(this);
    setFocusPolicy(Qt::ClickFocus);
    layout->setContentsMargins(0, 0, 0, 15);
    layout->setSpacing(15);

    m_listview = new PlayListView;

    auto btAddFrame = new QFrame;
    btAddFrame->setObjectName("PlaylistWidgetAddFrame");
    btAddFrame->setFocusPolicy(Qt::NoFocus);
    auto btAddFameLayout = new QVBoxLayout(btAddFrame);
    btAddFameLayout->setMargin(0);

    auto btAdd = new QPushButton();
    btAdd->setFixedSize(190, 36);
    btAdd->setObjectName("PlaylistWidgetAdd");
    btAdd->setText("+ " + tr("New playlist"));

    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setVerticalStretch(100);
    m_listview->setSizePolicy(sp);

    layout->addWidget(m_listview, 0, Qt::AlignHCenter);
    btAddFameLayout->addWidget(btAdd, 0, Qt::AlignBottom | Qt::AlignHCenter);
    layout->addWidget(btAddFrame, 0, Qt::AlignHCenter | Qt::AlignBottom);
    layout->addStretch();

    ThemeManager::instance()->regisetrWidget(this);

    connect(btAdd, &QPushButton::clicked, this, [ = ](bool /*checked*/) {
        qDebug() << "addPlaylist(true);";
        emit this->addPlaylist(true);
    });

    connect(m_listview, &PlayListView::itemClicked,
    this, [ = ](QListWidgetItem * item) {
        this->setEnabled(false);
        auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
        if (!playlistItem) {
            qCritical() << "playlistItem is empty" << item << playlistItem;
            return;
        }
        emit this->selectPlaylist(playlistItem->data());
        DUtil::TimerSingleShot(500, [this]() {
            this->setEnabled(true);
            emit this->hidePlaylist();
        });
    });
    connect(m_listview, &PlayListView::currentItemChanged,
    this, [ = ](QListWidgetItem * current, QListWidgetItem * previous) {
        auto itemWidget = qobject_cast<PlayListItem *>(m_listview->itemWidget(previous));
        if (itemWidget) {
            itemWidget->setActive(false);
        }
        itemWidget = qobject_cast<PlayListItem *>(m_listview->itemWidget(current));
        if (itemWidget) {
            itemWidget->setActive(true);
        }
    });
    connect(m_listview, &PlayListView::customResort,
    this, [ = ](const QStringList & uuids) {
        emit this->customResort(uuids);
    });
}

void PlaylistWidget::initData(QList<PlaylistPtr > playlists, PlaylistPtr last)
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
        m_listview->addItem(item);
        m_listview->setItemWidget(item, new PlayListItem(playlist));

        auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
        connect(playlistItem, &PlayListItem::remove, this, [ = ]() {
            m_listview->removeItemWidget(item);
            delete m_listview->takeItem(m_listview->row(item));
            // remote to firest
            Q_ASSERT(m_listview->count() > 0);
            m_listview->updateScrollbar();
            m_listview->setCurrentItem(m_listview->item(0));
        });

        connect(playlistItem, &PlayListItem::playall,
                this, &PlaylistWidget::playall);

        if (last->id() == playlist->id()) {
            current = item;
        }

    }

    if (current) {
        m_listview->setCurrentItem(current);
    } else {
        m_listview->setCurrentItem(m_listview->item(0));
    }
    m_listview->updateScrollbar();
}

void PlaylistWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr)
{
    for (int i = 0; i < m_listview->count(); ++i) {
        QListWidgetItem *item = m_listview->item(i);
        auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
        if (playlistItem->data()->id() == playlist->id()) {
            playlistItem->setPlay(true);
        } else {
            playlistItem->setPlay(false);
        }
    }
}

void PlaylistWidget::focusOutEvent(QFocusEvent *event)
{
    // TODO: monitor mouse position
    QPoint mousePos = mapToParent(mapFromGlobal(QCursor::pos()));
//    qDebug() << mapFromGlobal(QCursor::pos()) << mousePos;
//    qDebug() << event->reason();
    if (!this->geometry().contains(mousePos)) {
        if (event && event->reason() == Qt::MouseFocusReason) {
            DUtil::TimerSingleShot(50, [this]() {
                qDebug() << "self lost focus hide";
                emit this->hidePlaylist();
            });
        }
    }
    QFrame::focusOutEvent(event);
}

void PlaylistWidget::onPlaylistAdded(PlaylistPtr playlist)
{
    if (playlist->hide()) {
        return;
    }

    auto item = new QListWidgetItem;
    m_listview->addItem(item);
    m_listview->setItemWidget(item, new PlayListItem(playlist));

//   disable drag
//    if (playlist->id() == AllMusicListID || playlist->id() == FavMusicListID) {
//        item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
//    }
//    qDebug() << "pppppppp" << item->flags();

    auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
    connect(playlistItem, &PlayListItem::remove, this, [ = ]() {
        m_listview->removeItemWidget(item);
        delete m_listview->takeItem(m_listview->row(item));
    });

    connect(playlistItem, &PlayListItem::playall,
            this, &PlaylistWidget::playall);

    m_listview->setCurrentItem(item);
    m_listview->updateScrollbar();
}

void PlaylistWidget::onCurrentChanged(PlaylistPtr playlist)
{
    if (playlist) {
        m_listview->setCurrentItem(nullptr);
    }
    for (int i = 0; i < m_listview->count(); ++i) {
        QListWidgetItem *item = m_listview->item(i);
        auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
        if (playlistItem->data()->id() == playlist->id()) {
            m_listview->setCurrentItem(item);
            playlistItem->setActive(true);
        } else {
            playlistItem->setActive(false);
        }
    }
}

