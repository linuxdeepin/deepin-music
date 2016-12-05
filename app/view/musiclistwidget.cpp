/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musiclistwidget.h"

#include <QThread>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <dthememanager.h>

#include "musiclistview.h"
#include "../model/musiclistmodel.h"
#include "../core/playlist.h"

DWIDGET_USE_NAMESPACE

MusicListWidget::MusicListWidget(QWidget *parent) : QFrame(parent)
{
    setObjectName("MusicListWidget");

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *actionBar = new QFrame;
    actionBar->setFixedHeight(40);
    actionBar->setObjectName("MusicListActionBar");
    actionBar->hide();

    auto actionBarLayout = new QHBoxLayout(actionBar);
    actionBarLayout->setContentsMargins(10, 0, 20, 0);
    actionBarLayout->setSpacing(0);

    auto btPlayAll = new QPushButton;
    btPlayAll->setObjectName("MusicListPlayAll");
    btPlayAll->setText(tr("Play All"));
    btPlayAll->setFixedHeight(28);
    btPlayAll->setFocusPolicy(Qt::NoFocus);

    m_sortCombo = new DComboBox;
    m_sortCombo->setFixedHeight(24);
    m_sortCombo->setObjectName("MusicListSort");
    m_sortCombo->addItem(tr("Time added"));
    m_sortCombo->addItem(tr("Title"));
    m_sortCombo->addItem(tr("Artist"));
    m_sortCombo->addItem(tr("Album name"));

    m_emptyHits = new QLabel(tr("No Music in list"));
    m_emptyHits->setObjectName("MusicListEmptyHits");
    m_emptyHits->hide();

    actionBarLayout->addWidget(btPlayAll, 0, Qt::AlignCenter);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(m_sortCombo, 0, Qt::AlignCenter);
    m_musiclist = new MusicListView;
    m_model = m_musiclist->model();
    m_musiclist->hide();

    layout->addWidget(actionBar, 0, Qt::AlignTop);
    layout->addWidget(m_musiclist, 0, Qt::AlignTop);
    layout->addWidget(m_emptyHits, 0, Qt::AlignCenter);

    D_THEME_INIT_WIDGET(MusicListWidget);
    m_sortCombo->setStyleSheet(
        DThemeManager::instance()->getQssForWidget("Widget/ComboBox")
    );

    connect(m_sortCombo, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
    this, [ = ](int sortType) {

        qWarning() << "change to emptry playlist" << sortType;
        emit this->resort(m_playlist, sortType);
    });

    connect(btPlayAll, &QPushButton::clicked, this, [ = ](bool) {
        if (m_playlist) {
            emit this->playall(m_playlist);
        }
    });

    connect(this, &MusicListWidget::showEmptyHits, this, [ = ](bool isEmpty) {
        actionBar->setVisible(!isEmpty);
        m_musiclist->setVisible(!isEmpty);
        m_emptyHits->setVisible(isEmpty);
    });

    connect(m_musiclist, &MusicListView::requestCustomContextMenu,
    this, [ = ](const QPoint & pos) {
        emit this->requestCustomContextMenu(pos);
    });


    connect(m_musiclist, &MusicListView::removeMusicList,
    this, [ = ](const MusicMetaList & metalist) {
        emit this->musicListRemove(m_playlist, metalist);
    });
    connect(m_musiclist, &MusicListView::deleteMusicList,
    this, [ = ](const MusicMetaList & metalist) {
        emit this->musicListDelete(m_playlist, metalist);
    });

    connect(m_musiclist, &MusicListView::addToPlaylist,
    this, [ = ](QSharedPointer<Playlist> playlist, const MusicMetaList metalist) {
        emit this->musicAdd(playlist, metalist);
    });

    connect(m_musiclist, &MusicListView::doubleClicked,
    this, [ = ](const QModelIndex & index) {
        auto item = m_musiclist->model()->item(index.row(), 0);
        MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
        emit musicClicked(m_playlist, meta);
    });
    connect(m_musiclist, &MusicListView::play,
    this, [ = ](const MusicMeta & meta) {
        emit musicClicked(m_playlist, meta);
    });
}

void MusicListWidget::resizeEvent(QResizeEvent *event)
{
    // TODO: remove resize
    QWidget::resizeEvent(event);
    m_musiclist->resize(event->size());
    m_musiclist->setFixedHeight(event->size().height() - 40);
}

void MusicListWidget::onMusicPlayed(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    if (playlist != m_playlist) {
        qWarning() << "check playlist failed!"
                   << "m_playlist:" << m_playlist
                   << "playlist:" << m_playlist;
        return;
    }

    // TODO: quick find
    QModelIndex index;
    QStandardItem *item = nullptr;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        index = m_model->index(i, 0);
        item = m_model->item(i, 0);
        MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
        if (meta.hash == info.hash) {
            break;
        }
    }
    if (nullptr == item) {
        return;
    }
    m_musiclist->clearSelection();
    m_musiclist->setCurrentIndex(index);
    m_musiclist->scrollTo(index);
}

void MusicListWidget::onMusicRemoved(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    if (playlist != m_playlist) {
        qWarning() << "check playlist failed!"
                   << "m_playlist:" << m_playlist
                   << "playlist:" << m_playlist;
        return;
    }

    // TODO: quick find
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto item = m_model->item(i, 0);
        MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
        if (meta.hash == info.hash) {
            m_model->removeRow(i);
            break;
        }
    }


    showEmptyHits(playlist->length() == 0);
}

void MusicListWidget::addMusicInfo(MusicListView *m_musiclist, const MusicMeta &info)
{
    Q_ASSERT(m_musiclist);

    QStandardItem *newItem = new QStandardItem;
    newItem->setData(QVariant::fromValue<MusicMeta>(info));
    m_model->appendRow(newItem);
    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());
    m_model->setData(index, row);
    index = m_model->index(row, 1, QModelIndex());
    m_model->setData(index, info.title);
    index = m_model->index(row, 2, QModelIndex());
    auto artist = info.artist.isEmpty() ? tr("Unknow Artist") : info.artist;
    m_model->setData(index, artist);
    index = m_model->index(row, 3, QModelIndex());
    auto album = info.album.isEmpty() ? tr("Unknow Album") : info.album;
    m_model->setData(index, album);

    index = m_model->index(row, 4, QModelIndex());
    m_model->setData(index, lengthString(info.length));

    showEmptyHits(false);
}


void MusicListWidget::setCurrentList(QSharedPointer<Playlist> playlist)
{
    if (m_playlist) {
        m_playlist.data()->disconnect(this);
    }
    m_playlist = playlist;
}

void MusicListWidget::onMusicAdded(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    if (playlist != m_playlist) {
//        qWarning() << "check playlist failed!"
//                   << "m_playlist:" << m_playlist
//                   << "playlist:" << m_playlist;
        return;
    }
    addMusicInfo(m_musiclist, info);
}

void MusicListWidget::onMusicListAdded(QSharedPointer<Playlist> playlist, const MusicMetaList &infolist)
{
    for (auto &meta : infolist) {
        onMusicAdded(playlist, meta);
    }
}

void MusicListWidget::onLocate(QSharedPointer<Playlist> playlist, const MusicMeta &info)
{
    if (playlist.isNull()) {
        return;
    }

    if (playlist != m_playlist) {
        onMusiclistChanged(playlist);
    }

    QModelIndex index;
    QStandardItem *item = nullptr;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        index = m_model->index(i, 0);
        item = m_model->item(i, 0);
        MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
        if (meta.hash == info.hash) {
            break;
        }
    }
    if (nullptr == item) {
        return;
    }
    m_musiclist->clearSelection();
    m_musiclist->setCurrentIndex(index);

    auto viewRect = QRect(QPoint(0,0), size());
    if (!viewRect.intersects(m_musiclist->visualRect(index))) {
        m_musiclist->scrollTo(index, QListView::PositionAtCenter);
    }
}

void MusicListWidget::onMusiclistChanged(QSharedPointer<Playlist> playlist)
{
    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }
    this->setCurrentList(playlist);
    m_sortCombo->setCurrentIndex(playlist->sorttype());
    m_model->removeRows(0, m_model->rowCount());
    for (auto &info : playlist->allmusic()) {
        addMusicInfo(m_musiclist, info);
    }

    showEmptyHits(playlist->length() == 0);
}

void MusicListWidget::onCustomContextMenuRequest(const QPoint &pos,
        QSharedPointer<Playlist> selectedlist,
        QSharedPointer<Playlist> favlist,
        QList<QSharedPointer<Playlist> > newlists)
{
//    Q_ASSERT(item != nullptr);
    m_musiclist->showContextMenu(pos, selectedlist, favlist, newlists);
}
