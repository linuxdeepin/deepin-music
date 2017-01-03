/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musiclistwidget.h"

#include <QAction>
#include <QThread>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <thememanager.h>
#include <dcombobox.h>

#include "../core/music.h"
#include "../core/playlist.h"
#include "widget/musiclistview.h"
#include "widget/ddropdown.h"

DWIDGET_USE_NAMESPACE

class MusicListWidgetPrivate
{
public:
    MusicListWidgetPrivate(MusicListWidget *parent) : q_ptr(parent) {}

    void initConntion();
    void showEmptyHits(bool empty);

    QWidget             *actionBar      = nullptr;
    QPushButton         *btPlayAll      = nullptr;
    QLabel              *m_emptyHits    = nullptr;
    MusicListView       *m_musiclist    = nullptr;
    DDropdown           *m_dropdown    = nullptr;

    MusicListWidget *q_ptr;
    Q_DECLARE_PUBLIC(MusicListWidget)
};


void MusicListWidgetPrivate::initConntion()
{
    Q_Q(MusicListWidget);

    q->connect(m_dropdown, &DDropdown::triggered,
    q, [ = ](QAction * action) {
        qWarning() << "change to emptry playlist" << action->data();
        emit q->resort(m_musiclist->playlist(), action->data().value<Playlist::SortType>());
    });

    q->connect(btPlayAll, &QPushButton::clicked,
    q, [ = ](bool) {
        if (m_musiclist->playlist()) {
            emit q->playall(m_musiclist->playlist());
        }
    });
    q->connect(m_musiclist, &MusicListView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        emit q->requestCustomContextMenu(pos);
    });
    q->connect(m_musiclist, &MusicListView::removeMusicList,
    q, [ = ](const MusicMetaList & metalist) {
        emit q->musiclistRemove(m_musiclist->playlist(), metalist);
    });
    q->connect(m_musiclist, &MusicListView::deleteMusicList,
    q, [ = ](const MusicMetaList & metalist) {
        emit q->musiclistDelete(m_musiclist->playlist(), metalist);
    });
    q->connect(m_musiclist, &MusicListView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MusicMetaList metalist) {
        emit q->addToPlaylist(playlist, metalist);
    });
    q->connect(m_musiclist, &MusicListView::doubleClicked,
    q, [ = ](const QModelIndex & index) {
        auto model = qobject_cast<QStandardItemModel *>(m_musiclist->model());
        auto item = model->item(index.row(), 0);
        MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
        emit q->musicClicked(m_musiclist->playlist(), meta);
    });
    q->connect(m_musiclist, &MusicListView::play,
    q, [ = ](const MusicMeta & meta) {
        emit q->musicClicked(m_musiclist->playlist(), meta);
    });
}

void MusicListWidgetPrivate::showEmptyHits(bool empty)
{
    auto playlist = m_musiclist->playlist();
    if (playlist.isNull() || playlist->id() != SearchMusicListID) {
        m_emptyHits->setText(MusicListWidget::tr("No Music in list"));
    } else {
        m_emptyHits->setText(MusicListWidget::tr("No result found"));
    }
    actionBar->setVisible(!empty);
    m_musiclist->setVisible(!empty);
    m_emptyHits->setVisible(empty);
}

MusicListWidget::MusicListWidget(QWidget *parent) :
    QFrame(parent), d_ptr(new MusicListWidgetPrivate(this))
{
    Q_D(MusicListWidget);

    setObjectName("MusicListWidget");

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->actionBar = new QFrame;
    d->actionBar->setFixedHeight(40);
    d->actionBar->setObjectName("MusicListActionBar");
    d->actionBar->hide();

    auto actionBarLayout = new QHBoxLayout(d->actionBar);
    actionBarLayout->setContentsMargins(10, 0, 20, 0);
    actionBarLayout->setSpacing(0);

    d->btPlayAll = new QPushButton;
    d->btPlayAll->setObjectName("MusicListPlayAll");
    d->btPlayAll->setText(tr("Play All"));
    d->btPlayAll->setFixedHeight(28);
    d->btPlayAll->setFocusPolicy(Qt::NoFocus);

    d->m_dropdown = new DDropdown;
    d->m_dropdown->setFixedHeight(28);
    d->m_dropdown->setMinimumWidth(130);
    d->m_dropdown->setObjectName("MusicListSort");
    d->m_dropdown->addAction(tr("Time added"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAddTime));
    d->m_dropdown->addAction(tr("Title"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByTitle));
    d->m_dropdown->addAction(tr("Artist"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByArtist));
    d->m_dropdown->addAction(tr("Album name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAblum));

    d->m_emptyHits = new QLabel();
    d->m_emptyHits->setObjectName("MusicListEmptyHits");
    d->m_emptyHits->hide();

    actionBarLayout->addWidget(d->btPlayAll, 0, Qt::AlignCenter);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(d->m_dropdown, 0, Qt::AlignCenter);

    d->m_musiclist = new MusicListView;
    d->m_musiclist->hide();

    layout->addWidget(d->actionBar, 0, Qt::AlignTop);
    layout->addWidget(d->m_musiclist, 100, Qt::AlignTop);
    layout->addStretch();
    layout->addWidget(d->m_emptyHits, 0, Qt::AlignCenter);
    layout->addStretch();

    ThemeManager::instance()->regisetrWidget(this);

    d->initConntion();
}

MusicListWidget::~MusicListWidget()
{

}

void MusicListWidget::resizeEvent(QResizeEvent *event)
{
    Q_D(MusicListWidget);
    QFrame::resizeEvent(event);
    auto viewrect = QFrame::rect();
    auto viewsize = viewrect.marginsRemoved(contentsMargins()).size();
    d->m_musiclist->setFixedSize(viewsize.width(), viewsize.height() - 40);
}

void MusicListWidget::onMusicPlayed(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusicPlayed(playlist, meta);
}

void MusicListWidget::onMusicPause(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusicPause(playlist, meta);
}

void MusicListWidget::onMusicRemoved(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusicRemoved(playlist, meta);
    d->showEmptyHits(d->m_musiclist->model()->rowCount() == 0);
}

void MusicListWidget::initData(PlaylistPtr playlist)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusiclistChanged(playlist);
    // FIXME: dd

    for (auto action : d->m_dropdown->actions()) {
        if (action->data().toInt() == playlist->sorttype()) {
            d->m_dropdown->setCurrentAction(action);
        }
    }
    d->showEmptyHits(d->m_musiclist->model()->rowCount() == 0);
}

void MusicListWidget::onMusicAdded(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusicAdded(playlist, meta);
    d->showEmptyHits(false);
}

void MusicListWidget::onMusicError(PlaylistPtr playlist, const MusicMeta &meta, int error)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusicError(playlist, meta, error);
}

void MusicListWidget::onMusicListAdded(PlaylistPtr playlist, const MusicMetaList &infolist)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusicListAdded(playlist, infolist);
    d->showEmptyHits(infolist.length() == 0);
}

void MusicListWidget::onLocate(PlaylistPtr playlist, const MusicMeta &info)
{
    Q_D(MusicListWidget);
    initData(playlist);
    d->m_musiclist->onLocate(playlist, info);
}

void MusicListWidget::onMusiclistChanged(PlaylistPtr playlist)
{
    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    Q_D(MusicListWidget);

    initData(playlist);
}

void MusicListWidget::onCustomContextMenuRequest(const QPoint &pos,
        PlaylistPtr selectedlist,
        PlaylistPtr favlist,
        QList<PlaylistPtr > newlists)
{
    Q_D(MusicListWidget);
    d->m_musiclist->showContextMenu(pos, selectedlist, favlist, newlists);
}
