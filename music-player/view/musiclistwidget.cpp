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
#include <dcombobox.h>

#include "../core/music.h"
#include "../core/playlist.h"
#include "widget/musiclistview.h"
#include "viewpresenter.h"

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
    DComboBox           *m_sortCombo    = nullptr;

    MusicListWidget *q_ptr;
    Q_DECLARE_PUBLIC(MusicListWidget);
};


void MusicListWidgetPrivate::initConntion()
{
    Q_Q(MusicListWidget);

    q->connect(m_sortCombo, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
    q, [ = ](int sortType) {
        qWarning() << "change to emptry playlist" << sortType;
        emit ViewPresenter::instance()->resort(m_musiclist->playlist(), sortType);
    });

    q->connect(btPlayAll, &QPushButton::clicked,
    q, [ = ](bool) {
        if (m_musiclist->playlist()) {
            emit ViewPresenter::instance()->playall(m_musiclist->playlist());
        }
    });
    q->connect(m_musiclist, &MusicListView::requestCustomContextMenu,
    q, [ = ](const QPoint & pos) {
        emit ViewPresenter::instance()->requestCustomContextMenu(pos);
    });
    q->connect(m_musiclist, &MusicListView::removeMusicList,
    q, [ = ](const MusicMetaList & metalist) {
        emit ViewPresenter::instance()->musicListRemove(m_musiclist->playlist(), metalist);
    });
    q->connect(m_musiclist, &MusicListView::deleteMusicList,
    q, [ = ](const MusicMetaList & metalist) {
        emit ViewPresenter::instance()->musicListDelete(m_musiclist->playlist(), metalist);
    });
    q->connect(m_musiclist, &MusicListView::addToPlaylist,
    q, [ = ](PlaylistPtr playlist, const MusicMetaList metalist) {
        emit ViewPresenter::instance()->addToPlaylist(playlist, metalist);
    });
    q->connect(m_musiclist, &MusicListView::doubleClicked,
    q, [ = ](const QModelIndex & index) {
        auto model = qobject_cast<QStandardItemModel *>(m_musiclist->model());
        auto item = model->item(index.row(), 0);
        MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
        emit ViewPresenter::instance()->musicClicked(m_musiclist->playlist(), meta);
    });
    q->connect(m_musiclist, &MusicListView::play,
    q, [ = ](const MusicMeta & meta) {
        emit ViewPresenter::instance()->musicClicked(m_musiclist->playlist(), meta);
    });
    q->connect(ViewPresenter::instance(), &ViewPresenter::musicAdded,
               q, &MusicListWidget::onMusicAdded);
}

void MusicListWidgetPrivate::showEmptyHits(bool empty)
{
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

    d->m_sortCombo = new DComboBox;
    d->m_sortCombo->setFixedHeight(24);
    d->m_sortCombo->setObjectName("MusicListSort");
    d->m_sortCombo->addItem(tr("Time added"));
    d->m_sortCombo->addItem(tr("Title"));
    d->m_sortCombo->addItem(tr("Artist"));
    d->m_sortCombo->addItem(tr("Album name"));

    d->m_emptyHits = new QLabel(tr("No Music in list"));
    d->m_emptyHits->setObjectName("MusicListEmptyHits");
    d->m_emptyHits->hide();

    actionBarLayout->addWidget(d->btPlayAll, 0, Qt::AlignCenter);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(d->m_sortCombo, 0, Qt::AlignCenter);

    d->m_musiclist = new MusicListView;
    d->m_musiclist->hide();

    layout->addWidget(d->actionBar, 0, Qt::AlignTop);
    layout->addWidget(d->m_musiclist, 0, Qt::AlignTop);
    layout->addWidget(d->m_emptyHits, 0, Qt::AlignCenter);

    D_THEME_INIT_WIDGET(MusicListWidget);
    d->m_sortCombo->setStyleSheet(
        DThemeManager::instance()->getQssForWidget("Widget/ComboBox")
    );

    d->initConntion();
}

MusicListWidget::~MusicListWidget()
{

}

void MusicListWidget::resizeEvent(QResizeEvent *event)
{
    Q_D(MusicListWidget);
    // TODO: remove resize
    QWidget::resizeEvent(event);
    d->m_musiclist->setFixedSize(event->size().width(), event->size().height() - 40);
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
    d->m_sortCombo->setCurrentIndex(playlist->sorttype());
    d->showEmptyHits(d->m_musiclist->model()->rowCount() == 0);
}

void MusicListWidget::onMusicAdded(PlaylistPtr playlist, const MusicMeta &meta)
{
    Q_D(MusicListWidget);
    d->m_musiclist->onMusicAdded(playlist, meta);
    d->showEmptyHits(false);
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
