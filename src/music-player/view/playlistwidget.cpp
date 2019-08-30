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

#include "playlistwidget.h"

#include <QDebug>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMimeData>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <DThemeManager>

#include "../core/music.h"
#include "../core/playlist.h"
#include "widget/playlistview.h"
#include "widget/ddropdown.h"

DWIDGET_USE_NAMESPACE

class PlayListWidgetPrivate
{
public:
    PlayListWidgetPrivate(PlayListWidget *parent) : q_ptr(parent) {}

    void initData(PlaylistPtr playlist);
    void initConntion();
    void showEmptyHits(bool empty);

    QLabel              *emptyHits      = nullptr;
    QWidget             *actionBar      = nullptr;
    QPushButton         *btPlayAll      = nullptr;
    DDropdown           *dropdown       = nullptr;
    PlayListView        *playListView   = nullptr;
    QAction             *customAction   = nullptr;

    PlayListWidget *q_ptr;
    Q_DECLARE_PUBLIC(PlayListWidget)
};


void PlayListWidgetPrivate::initData(PlaylistPtr playlist)
{
    Q_Q(PlayListWidget);
    btPlayAll->setText(playlist->displayName());
    playListView->onMusiclistChanged(playlist);

    if (playlist->sortType() == Playlist::SortByCustom) {
        q->setCustomSortType();
    } else {
        for (auto action : dropdown->actions()) {
            if (action->data().toInt() == playlist->sortType()) {
                dropdown->setCurrentAction(action);
            }
        }
    }
    showEmptyHits(playListView->model()->rowCount() == 0);
}

void PlayListWidgetPrivate::initConntion()
{
    Q_Q(PlayListWidget);

    q->connect(dropdown, &DDropdown::triggered,
    q, [ = ](QAction * action) {
        dropdown->setCurrentAction(action);
        Q_EMIT q->resort(playListView->playlist(), action->data().value<Playlist::SortType>());
    });

    q->connect(btPlayAll, &QPushButton::clicked,
    q, [ = ](bool) {
        if (playListView->playlist()) {
            Q_EMIT q->playall(playListView->playlist());
        }
    });

    q->connect(playListView, &PlayListView::customSort,
    q, [ = ]() {
        q->setCustomSortType();
        Q_EMIT q->resort(playListView->playlist(), Playlist::SortByCustom);
    });

    q->connect(playListView, &PlayListView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        Q_EMIT q->requestCustomContextMenu(pos);
    });
    q->connect(playListView, &PlayListView::removeMusicList,
    q, [ = ](const MetaPtrList  & metalist) {
        Q_EMIT q->musiclistRemove(playListView->playlist(), metalist);
    });
    q->connect(playListView, &PlayListView::deleteMusicList,
    q, [ = ](const MetaPtrList & metalist) {
        Q_EMIT q->musiclistDelete(playListView->playlist(), metalist);
    });
    q->connect(playListView, &PlayListView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MetaPtrList  metalist) {
        Q_EMIT q->addToPlaylist(playlist, metalist);
    });
    q->connect(playListView, &PlayListView::playMedia,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->playMedia(playListView->playlist(), meta);
    });
    q->connect(playListView, &PlayListView::showInfoDialog,
    q, [ = ](const MetaPtr meta) {
        Q_EMIT q->showInfoDialog(meta);
    });
    q->connect(playListView, &PlayListView::updateMetaCodec,
    q, [ = ](const MetaPtr  meta) {
        Q_EMIT q->updateMetaCodec(meta);
    });
}

void PlayListWidgetPrivate::showEmptyHits(bool empty)
{
    auto playlist = playListView->playlist();
    if (playlist.isNull() || playlist->id() != SearchMusicListID) {
        emptyHits->setText(PlayListWidget::tr("No Music"));
    } else {
        emptyHits->setText(PlayListWidget::tr("No result found"));
    }
    actionBar->setVisible(!empty);
    playListView->setVisible(!empty);
    emptyHits->setVisible(empty);
}

PlayListWidget::PlayListWidget(QWidget *parent) :
    QFrame(parent), d_ptr(new PlayListWidgetPrivate(this))
{
    Q_D(PlayListWidget);

    DThemeManager::instance()->registerWidget(this);

    setObjectName("PlayListWidget");
    setAcceptDrops(true);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->actionBar = new QFrame;
    d->actionBar->setFixedHeight(40);
    d->actionBar->setObjectName("PlayListActionBar");
    d->actionBar->hide();

    auto actionBarLayout = new QHBoxLayout(d->actionBar);
    actionBarLayout->setContentsMargins(10, 0, 8, 0);
    actionBarLayout->setSpacing(0);

    d->btPlayAll = new QPushButton;
    d->btPlayAll->setObjectName("PlayListPlayAll");
    d->btPlayAll->setText(tr("Play All"));
    d->btPlayAll->setFixedHeight(28);
    d->btPlayAll->setFocusPolicy(Qt::NoFocus);

    d->dropdown = new DDropdown;
    d->dropdown->setFixedHeight(28);
    d->dropdown->setMinimumWidth(130);
    d->dropdown->setObjectName("PlayListSort");
    d->dropdown->addAction(tr("Time added"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAddTime));
    d->dropdown->addAction(tr("Title"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByTitle));
    d->dropdown->addAction(tr("Artist name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByArtist));
    d->dropdown->addAction(tr("Album name"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByAblum));
//    d->customAction = d->dropdown->addAction(tr("Custom"), QVariant::fromValue<Playlist::SortType>(Playlist::SortByCustom));
//    d->customAction->setDisabled(true);

    d->emptyHits = new QLabel();
    d->emptyHits->setObjectName("PlayListEmptyHits");
    d->emptyHits->hide();

    actionBarLayout->addWidget(d->btPlayAll, 0, Qt::AlignCenter);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(d->dropdown, 0, Qt::AlignCenter);

    d->playListView = new PlayListView;
    d->playListView->hide();

    layout->addWidget(d->actionBar, 0, Qt::AlignTop);
    layout->addWidget(d->playListView, 100, Qt::AlignTop);
    layout->addStretch();
    layout->addWidget(d->emptyHits, 0, Qt::AlignCenter);
    layout->addStretch();

    d->initConntion();
}

PlayListWidget::~PlayListWidget()
{
}

void PlayListWidget::setCustomSortType()
{
    Q_D(PlayListWidget);
    d->dropdown->setCurrentAction(nullptr);
    d->dropdown->setText(tr("Custom"));
}

void PlayListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QFrame::dragEnterEvent(event);
    if (event->mimeData()->hasFormat("text/uri-list")) {
        qDebug() << "acceptProposedAction" << event;
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }
}

void PlayListWidget::dropEvent(QDropEvent *event)
{
    QFrame::dropEvent(event);
    Q_D(PlayListWidget);

    if (!event->mimeData()->hasFormat("text/uri-list")) {
        return;
    }

    auto urls = event->mimeData()->urls();
    QStringList localpaths;
    for (auto &url : urls) {
        localpaths << url.toLocalFile();
    }

    if (!localpaths.isEmpty() && !d->playListView->playlist().isNull()) {
        Q_EMIT importSelectFiles(d->playListView->playlist(), localpaths);
    }
}

void PlayListWidget::resizeEvent(QResizeEvent *event)
{
    Q_D(PlayListWidget);
    QFrame::resizeEvent(event);
    auto viewrect = QFrame::rect();
    auto viewsize = viewrect.marginsRemoved(contentsMargins()).size();
    d->playListView->setFixedSize(viewsize.width(), viewsize.height() - 40);
    d->emptyHits->setFixedSize(viewsize.width(), viewsize.height());
}

void PlayListWidget::onMusicPlayed(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(PlayListWidget);

    if (playlist != d->playListView->playlist()) {
        d->initData(playlist);
    }

    if (playlist != d->playListView->playlist() || meta.isNull()) {
        return;
    }

    QModelIndex index = d->playListView->findIndex(meta);
    if (!index.isValid()) {
        return;
    }

    auto selectedIndexes = d->playListView->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() > 1) {
        d->playListView->update();
        return;
    }

    d->playListView->clearSelection();
    d->playListView->setCurrentIndex(index);
    d->playListView->scrollTo(index);
    d->playListView->update();
}

void PlayListWidget::onMusicPause(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(PlayListWidget);
    if (playlist != d->playListView->playlist() || meta.isNull()) {
        return;
    }
    d->playListView->update();
}

void PlayListWidget::onMusicListRemoved(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(PlayListWidget);

    if (playlist != d->playListView->playlist()) {
        return;
    }

    d->playListView->onMusicListRemoved(metalist);
    d->showEmptyHits(d->playListView->model()->rowCount() == 0);
}

void PlayListWidget::onMusicError(PlaylistPtr playlist, const MetaPtr meta, int error)
{
    Q_D(PlayListWidget);
    Q_UNUSED(playlist);
    d->playListView->onMusicError(meta, error);
}

void PlayListWidget::onMusicListAdded(PlaylistPtr playlist, const MetaPtrList metalist)
{
    Q_D(PlayListWidget);

    if (playlist != d->playListView->playlist()) {
        return;
    }

    d->playListView->onMusicListAdded(metalist);
    d->showEmptyHits(metalist.length() == 0);
}

void PlayListWidget::onLocate(PlaylistPtr playlist, const MetaPtr meta)
{
    Q_D(PlayListWidget);
    if (d->playListView->playlist() != playlist) {
        d->initData(playlist);
    }
    d->playListView->onLocate(meta);
}

void PlayListWidget::onMusiclistChanged(PlaylistPtr playlist)
{
    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    Q_D(PlayListWidget);

    d->initData(playlist);
}

void PlayListWidget::onCustomContextMenuRequest(const QPoint &pos,
                                                PlaylistPtr selectedlist,
                                                PlaylistPtr favlist,
                                                QList<PlaylistPtr > newlists)
{
    Q_D(PlayListWidget);
    d->playListView->showContextMenu(pos, selectedlist, favlist, newlists);
}

