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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMimeData>
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

    void initData(PlaylistPtr playlist);
    void initConntion();
    void showEmptyHits(bool empty);

    QLabel              *emptyHits      = nullptr;
    QWidget             *actionBar      = nullptr;
    QPushButton         *btPlayAll      = nullptr;
    DDropdown           *dropdown       = nullptr;
    MusicListView       *musiclist      = nullptr;
    QAction             *customAction   = nullptr;

    MusicListWidget *q_ptr;
    Q_DECLARE_PUBLIC(MusicListWidget)
};


void MusicListWidgetPrivate::initData(PlaylistPtr playlist)
{
    btPlayAll->setText(playlist->displayName());
    musiclist->onMusiclistChanged(playlist);

    for (auto action : dropdown->actions()) {
        if (action->data().toInt() == playlist->sortType()) {
            dropdown->setCurrentAction(action);
        }
    }
    showEmptyHits(musiclist->model()->rowCount() == 0);
}

void MusicListWidgetPrivate::initConntion()
{
    Q_Q(MusicListWidget);

    q->connect(dropdown, &DDropdown::triggered,
    q, [ = ](QAction * action) {
        dropdown->setCurrentAction(action);
        emit q->resort(musiclist->playlist(), action->data().value<Playlist::SortType>());
    });

    q->connect(btPlayAll, &QPushButton::clicked,
    q, [ = ](bool) {
        if (musiclist->playlist()) {
            emit q->playall(musiclist->playlist());
        }
    });

    q->connect(musiclist, &MusicListView::customSort,
    q, [ = ]() {
        dropdown->setCurrentAction(customAction);
        emit q->resort(musiclist->playlist(), customAction->data().value<Playlist::SortType>());
    });

    q->connect(musiclist, &MusicListView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        emit q->requestCustomContextMenu(pos);
    });
    q->connect(musiclist, &MusicListView::removeMusicList,
    q, [ = ](const MetaPtrList  & metalist) {
        emit q->musiclistRemove(musiclist->playlist(), metalist);
    });
    q->connect(musiclist, &MusicListView::deleteMusicList,
    q, [ = ](const MetaPtrList & metalist) {
        emit q->musiclistDelete(musiclist->playlist(), metalist);
    });
    q->connect(musiclist, &MusicListView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        emit q->addToPlaylist(playlist, metalist);
    });
    q->connect(musiclist, &MusicListView::playMedia,
    q, [ = ](const MetaPtr meta) {
        emit q->playMedia(musiclist->playlist(), meta);
    });
    q->connect(musiclist, &MusicListView::showInfoDialog,
    q, [ = ](const MetaPtr meta) {
        emit q->showInfoDialog(meta);
    });
    q->connect(musiclist, &MusicListView::updateMetaCodec,
    q, [ = ](const MetaPtr  meta) {
        emit q->updateMetaCodec(meta);
    });
}

void MusicListWidgetPrivate::showEmptyHits(bool empty)
{
    auto playlist = musiclist->playlist();
    if (playlist.isNull() || playlist->id() != SearchMusicListID) {
        emptyHits->setText(MusicListWidget::tr("No Music"));
    } else {
        emptyHits->setText(MusicListWidget::tr("No result found"));
    }
    actionBar->setVisible(!empty);
    musiclist->setVisible(!empty);
    emptyHits->setVisible(empty);
}

MusicListWidget::MusicListWidget(QWidget *parent) :
    QFrame(parent), d_ptr(new MusicListWidgetPrivate(this))
{
    Q_D(MusicListWidget);

    setObjectName("MusicListWidget");
    setAcceptDrops(true);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->actionBar = new QFrame;
    d->actionBar->setFixedHeight(40);
    d->actionBar->setObjectName("MusicListActionBar");
    d->actionBar->hide();

    auto actionBarLayout = new QHBoxLayout(d->actionBar);
    actionBarLayout->setContentsMargins(10, 0, 8, 0);
    actionBarLayout->setSpacing(0);

    d->btPlayAll = new QPushButton;
    d->btPlayAll->setObjectName("MusicListPlayAll");
    d->btPlayAll->setText(tr("Play All"));
    d->btPlayAll->setFixedHeight(28);
    d->btPlayAll->setFocusPolicy(Qt::NoFocus);

    d->dropdown = new DDropdown;
    d->dropdown->setFixedHeight(28);
    d->dropdown->setMinimumWidth(130);
    d->dropdown->setObjectName("MusicListSort");
    d->dropdown->addAction(tr("Time added"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAddTime));
    d->dropdown->addAction(tr("Title"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByTitle));
    d->dropdown->addAction(tr("Artist"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByArtist));
    d->dropdown->addAction(tr("Album name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAblum));
    d->customAction = d->dropdown->addAction(tr("Custom"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByCustom));
    d->customAction->setDisabled(true);

    d->emptyHits = new QLabel();
    d->emptyHits->setObjectName("MusicListEmptyHits");
    d->emptyHits->hide();

    actionBarLayout->addWidget(d->btPlayAll, 0, Qt::AlignCenter);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(d->dropdown, 0, Qt::AlignCenter);

    d->musiclist = new MusicListView;
    d->musiclist->hide();

    layout->addWidget(d->actionBar, 0, Qt::AlignTop);
    layout->addWidget(d->musiclist, 100, Qt::AlignTop);
    layout->addStretch();
    layout->addWidget(d->emptyHits, 0, Qt::AlignCenter);
    layout->addStretch();

    ThemeManager::instance()->regisetrWidget(this);

    d->initConntion();
}

MusicListWidget::~MusicListWidget()
{
}

void MusicListWidget::setCustomSortType()
{
    Q_D(MusicListWidget);
    d->dropdown->setCurrentAction(d->customAction);
}

void MusicListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QFrame::dragEnterEvent(event);
    if (event->mimeData()->hasFormat("text/uri-list")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }
}

void MusicListWidget::dropEvent(QDropEvent *event)
{
    QFrame::dropEvent(event);
    Q_D(MusicListWidget);

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << url.toLocalFile();
    }

    if (!localpaths.isEmpty() && !d->musiclist->playlist().isNull()) {
        emit importSelectFiles(d->musiclist->playlist(), localpaths);
    }
}

void MusicListWidget::resizeEvent(QResizeEvent *event)
{
    Q_D(MusicListWidget);
    QFrame::resizeEvent(event);
    auto viewrect = QFrame::rect();
    auto viewsize = viewrect.marginsRemoved(contentsMargins()).size();
    d->musiclist->setFixedSize(viewsize.width(), viewsize.height() - 40);
    d->emptyHits->setFixedSize(viewsize.width(), viewsize.height());
}

void MusicListWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(MusicListWidget);

    if (playlist != d->musiclist->playlist() || meta.isNull()) {
        return;
    }

    QModelIndex index = d->musiclist->findIndex(meta);
    if (!index.isValid()) {
        return;
    }

    auto selectedIndexes = d->musiclist->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() > 1) {
        d->musiclist->update();
        return;
    }

    d->musiclist->clearSelection();
    d->musiclist->setCurrentIndex(index);
    d->musiclist->scrollTo(index);
    d->musiclist->update();
}

void MusicListWidget::onMusicPause(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(MusicListWidget);
    if (playlist != d->musiclist->playlist() || meta.isNull()) {
        return;
    }
    d->musiclist->update();
}

void MusicListWidget::onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(MusicListWidget);

    if (playlist != d->musiclist->playlist()) {
        return;
    }

    d->musiclist->onMusicListRemoved(metalist);
    d->showEmptyHits(d->musiclist->model()->rowCount() == 0);
}

void MusicListWidget::onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error)
{
    Q_D(MusicListWidget);
    Q_UNUSED(playlist);
    d->musiclist->onMusicError(meta, error);
}

void MusicListWidget::onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(MusicListWidget);

    if (playlist != d->musiclist->playlist()) {
        return;
    }

    d->musiclist->onMusicListAdded(metalist);
    d->showEmptyHits(metalist.length() == 0);
}

void MusicListWidget::onLocate(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(MusicListWidget);
    if (d->musiclist->playlist() != playlist) {
        d->initData(playlist);
    }
    d->musiclist->onLocate(meta);
}

void MusicListWidget::onMusiclistChanged(PlaylistPtr playlist)
{
    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    Q_D(MusicListWidget);

    d->initData(playlist);
}

void MusicListWidget::onCustomContextMenuRequest(const QPoint &pos,
        PlaylistPtr selectedlist,
        PlaylistPtr favlist,
        QList<PlaylistPtr > newlists)
{
    Q_D(MusicListWidget);
    d->musiclist->showContextMenu(pos, selectedlist, favlist, newlists);
}

