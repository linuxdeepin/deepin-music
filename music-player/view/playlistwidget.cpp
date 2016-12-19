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

#include <dthememanager.h>

#include "../core/playlist.h"
#include "viewpresenter.h"
#include "widget/playlistview.h"
#include "widget/playlistitem.h"

DWIDGET_USE_NAMESPACE

PlaylistWidget::PlaylistWidget(QWidget *parent) : QFrame(parent)
{
    setObjectName("PlaylistWidget");

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 15);
    layout->setSpacing(15);

    m_listview = new PlayListView;

    auto btAdd = new QPushButton();
    btAdd->setFixedSize(190, 36);
    btAdd->setObjectName("PlaylistWidgetAdd");
    btAdd->setText(tr("+ Add Playlist"));

    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setVerticalStretch(100);
    m_listview->setSizePolicy(sp);

    layout->addWidget(m_listview, 0, Qt::AlignHCenter);
    layout->addWidget(btAdd, 0, Qt::AlignBottom | Qt::AlignHCenter);

    D_THEME_INIT_WIDGET(PlaylistWidget);

    connect(btAdd, &QPushButton::clicked, this, [ = ](bool /*checked*/) {
        emit ViewPresenter::instance()->addPlaylist(true);
    });

    connect(m_listview, &PlayListView::itemClicked,
    this, [ = ](QListWidgetItem * item) {
        auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
        if (!playlistItem) {
            qCritical() << "playlistItem is empty" << item << playlistItem;
            return;
        }
        emit ViewPresenter::instance()->selectPlaylist(playlistItem->data());
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
        auto item = new QListWidgetItem;
        m_listview->addItem(item);
        m_listview->setItemWidget(item, new PlayListItem(playlist));

        auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
        connect(playlistItem, &PlayListItem::remove, this, [ = ]() {
            m_listview->removeItemWidget(item);
            delete m_listview->takeItem(m_listview->row(item));
            // remote to firest
            Q_ASSERT(m_listview->count() > 0);
            m_listview->setCurrentItem(m_listview->item(0));
        });

        connect(playlistItem, &PlayListItem::playall,
                ViewPresenter::instance(), &ViewPresenter::playall);

        if (last->id() == playlist->id()) {
            current = item;
        }

        if (playlist->hide()) {
            m_listview->setItemHidden(item, true);
        }
    }

    if (current) {
        m_listview->setCurrentItem(current);
    } else {
        m_listview->setCurrentItem(m_listview->item(0));
    }
}

void PlaylistWidget::focusOutEvent(QFocusEvent *event)
{
    // TODO: hide
    qDebug() << event;
}

void PlaylistWidget::onPlaylistAdded(PlaylistPtr playlist)
{
    auto item = new QListWidgetItem;
    m_listview->addItem(item);
    m_listview->setItemWidget(item, new PlayListItem(playlist));
    auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
    connect(playlistItem, &PlayListItem::remove, this, [ = ]() {
        m_listview->removeItemWidget(item);
        delete m_listview->takeItem(m_listview->row(item));
    });

    connect(playlistItem, &PlayListItem::playall,
            ViewPresenter::instance(), &ViewPresenter::playall);
    m_listview->scrollToBottom();
    m_listview->setCurrentItem(item);
    if (playlist->hide()) {
        m_listview->setItemHidden(item, true);
    }
}

void PlaylistWidget::onCurrentChanged(PlaylistPtr playlist)
{
    for (int i = 0; i < m_listview->count(); ++i) {
        QListWidgetItem *item = m_listview->item(i);
        auto playlistItem = qobject_cast<PlayListItem *>(m_listview->itemWidget(item));
        if (playlistItem->data()->id() == playlist->id()) {
            m_listview->setCurrentItem(item);
        }
    }
}

