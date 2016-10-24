/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musiclistwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QStringListModel>

#include <QMediaPlayer>

#include <dthememanager.h>
#include <dcombobox.h>
DWIDGET_USE_NAMESPACE

#include "musiclistview.h"
#include "musicitem.h"
#include "../model/musiclistmodel.h"
#include "../core/playlist.h"

MusicListWidget::MusicListWidget(QWidget *parent) : QFrame(parent)
{
    setObjectName("MusicListWidget");

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *actionBar = new QFrame;
    actionBar->setFixedHeight(40);
    actionBar->setObjectName("MusicListActionBar");

    auto actionBarLayout = new QHBoxLayout(actionBar);
    actionBarLayout->setContentsMargins(20, 0, 20, 0);
    actionBarLayout->setSpacing(0);

    auto btPlayAll = new QPushButton;
    btPlayAll->setObjectName("MusicListPlayAll");
    btPlayAll->setText(tr("Play All"));
    btPlayAll->setFixedHeight(28);

    auto cbSort = new DComboBox;
    cbSort->setObjectName("MusicListSort");
    cbSort->addItem(tr("By Title"));
    cbSort->addItem(tr("By Add Time"));

    actionBarLayout->addWidget(btPlayAll, 0, Qt::AlignCenter);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(cbSort, 0, Qt::AlignCenter);
    m_musiclist = new MusicListView;

    layout->addWidget(actionBar, 0, Qt::AlignTop);
    layout->addWidget(m_musiclist, 0, Qt::AlignTop);

    D_THEME_INIT_WIDGET(MusicListWidget);

    connect(m_musiclist, &MusicListView::doubleClicked,
    this, [ = ](const QModelIndex & index) {
        auto musicItem = qobject_cast<MusicItem *>(m_musiclist->indexWidget(index));
        if (musicItem) {
            emit musicItem->play();
        }
    });
}

void MusicListWidget::resizeEvent(QResizeEvent *event)
{
    // TODO: remove resize
    QWidget::resizeEvent(event);
    m_musiclist->setFixedHeight(event->size().height() - 40);
}

void MusicListWidget::onMusicPlayed(QSharedPointer<Playlist> palylist, const MusicInfo &info)
{
    if (palylist != m_palylist) {
        qWarning() << "check playlist failed!"
                   << "m_palylist:" << m_palylist
                   << "playlist:" << m_palylist;
        return;
    }
    // TODO: spead up find item
    MusicItem *musicItem = nullptr;
    QListWidgetItem *item = nullptr;
    for (int i = 0; i < m_musiclist->count(); ++i) {
        item = m_musiclist->item(i);
        musicItem = qobject_cast<MusicItem *>(m_musiclist->itemWidget(item));
        if (musicItem && musicItem->info().id == info.id) {
            qDebug() << "find"<< i << item << m_musiclist->count();
            break;
        }
    }
    if (m_last) {
        m_last->stop();
    }
    m_last = musicItem;

    if (!musicItem) {
        return;
    }
    musicItem->onMusicPlay();
    m_musiclist->setCurrentItem(item);
}

void MusicListWidget::onMusicRemoved(QSharedPointer<Playlist> palylist, const MusicInfo &info)
{
    if (palylist != m_palylist) {
        qWarning() << "check playlist failed!"
                   << "m_palylist:" << m_palylist
                   << "playlist:" << m_palylist;
        return;
    }
    // TODO: spead up find item
    MusicItem *musicItem = nullptr;
    QListWidgetItem *item = nullptr;
    for (int i = 0; i < m_musiclist->count(); ++i) {
        item = m_musiclist->item(i);
        musicItem = qobject_cast<MusicItem *>(m_musiclist->itemWidget(item));
        if (musicItem && musicItem->info().id == info.id) {
            break;
        }
    }

    if (m_last == musicItem) {
        m_last = nullptr;
    }

    m_musiclist->removeItemWidget(item);
    delete m_musiclist->takeItem(m_musiclist->row(item));

    // TODO: how to scroll
//        m_musiclist->scrollToItem(m_musiclist->item(row));
}

void MusicListWidget::addMusicInfo(MusicListView *m_musiclist, const MusicInfo &info)
{
//    qDebug() << "add" << m_musiclist->count()<<info.title;
    auto item = new QListWidgetItem;
    auto musicItem = new MusicItem(m_musiclist->model()->rowCount() + 1, info);

    m_musiclist->addItem(item);
    m_musiclist->setItemWidget(item, musicItem);

    connect(musicItem, &MusicItem::remove, this, [ = ]() {
        emit this->musicRemove(m_palylist, musicItem->info());
    });

    connect(musicItem, &MusicItem::play, this, [ = ]() {
        if (m_last) {
            m_last->stop();
        }
        m_last = musicItem;
        emit musicClicked(m_palylist, musicItem->info());
    });

    connect(musicItem, &MusicItem::addToPlaylist, this, [ = ](const QString & id) {
        emit musicAdd(id, musicItem->info());
    });
}


void MusicListWidget::setCurrentList(QSharedPointer<Playlist> palylist)
{
    if (m_palylist) {
        m_palylist.data()->disconnect(this);
    }
    m_palylist = palylist;
}

void MusicListWidget::onMusicAdded(QSharedPointer<Playlist> palylist, const MusicInfo &info)
{
    if (palylist != m_palylist) {
//        qWarning() << "check playlist failed!"
//                   << "m_palylist:" << m_palylist
//                   << "playlist:" << m_palylist;
        return;
    }
    addMusicInfo(m_musiclist, info);
}

void MusicListWidget::onMusiclistChanged(QSharedPointer<Playlist> playlist)
{
    if (playlist.isNull()) {
        qWarning() << "change to emptry playlist";
        return;
    }
    m_last = nullptr;
    this->setCurrentList(playlist);
    m_musiclist->clear();
    for (auto &info : playlist->allmusic()) {
        addMusicInfo(m_musiclist, info);
    }
}
