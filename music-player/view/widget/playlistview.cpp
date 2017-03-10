/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlistview.h"

#include <QDebug>
#include <QMenu>
#include <QScrollBar>
#include <QStyleFactory>

#include <thememanager.h>

#include "playlistitem.h"

class PlayListViewPrivate
{
public:
    PlayListViewPrivate(PlayListView *parent) : q_ptr(parent) {}

    void checkScrollbarSize();

    QScrollBar          *vscrollBar   = nullptr;

    PlayListView *q_ptr;
    Q_DECLARE_PUBLIC(PlayListView)
};

void PlayListViewPrivate::checkScrollbarSize()
{
    Q_Q(PlayListView);

    auto itemCount = q->model()->rowCount();
    auto size = q->size();
    auto scrollBarWidth = 8;
    auto itemHeight = 56;
    vscrollBar->resize(scrollBarWidth, size.height() - 2);
    vscrollBar->move(size.width() - scrollBarWidth , 0);
    vscrollBar->setSingleStep(1);
    vscrollBar->setPageStep(size.height() / itemHeight);

    if (itemCount > size.height() / itemHeight) {
        vscrollBar->show();
        vscrollBar->setMaximum(itemCount - size.height() / itemHeight);
    } else {
        vscrollBar->hide();
        vscrollBar->setMaximum(0);
    }
}

PlayListView::PlayListView(QWidget *parent) : QListWidget(parent), d_ptr(new PlayListViewPrivate(this))
{
    Q_D(PlayListView);

    setObjectName("PlayListView");
    ThemeManager::instance()->regisetrWidget(this);

    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::DragOnly);

    setSelectionMode(QListView::SingleSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    d->vscrollBar = new QScrollBar(this);
    d->vscrollBar->setObjectName("MusicListViewScrollBar");
    d->vscrollBar->setOrientation(Qt::Vertical);
    d->vscrollBar->raise();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &PlayListView::customContextMenuRequested,
            this, &PlayListView::showContextMenu);

    connect(d->vscrollBar, &QScrollBar::valueChanged,
    this, [ = ](int value) {
        verticalScrollBar()->setValue(value);
    });
}

PlayListView::~PlayListView()
{

}

void PlayListView::wheelEvent(QWheelEvent *event)
{
    Q_D(PlayListView);
    QListWidget::wheelEvent(event);
    d->vscrollBar->setSliderPosition(verticalScrollBar()->sliderPosition());
}

void PlayListView::resizeEvent(QResizeEvent *event)
{
    Q_D(PlayListView);
    QListWidget::resizeEvent(event);
    d->checkScrollbarSize();
}

void PlayListView::startDrag(Qt::DropActions supportedActions)
{
    QListWidget::startDrag(supportedActions);
    qDebug() << "drag end";

    QStringList uuids;

    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem *item = this->item(i);
        auto playlistItem = qobject_cast<PlayListItem *>(this->itemWidget(item));
        uuids << playlistItem->data()->id();
    }
    emit customResort(uuids);
}

void PlayListView::updateScrollbar()
{
    Q_D(PlayListView);
    d->checkScrollbarSize();
}


void PlayListView::showContextMenu(const QPoint &pos)
{
    //find parent
    auto playlistWidget = this->parentWidget();
    for (int i = 0; i < 10; ++i) {
        if (!playlistWidget) {
            break;
        }

        if (playlistWidget->objectName() == "PlaylistWidget") {
            break;
        }
        playlistWidget = playlistWidget->parentWidget();
    }

    if (playlistWidget && !playlistWidget->isEnabled()) {
        return;
    }

    // get select
    auto items = this->selectedItems();
    if (items.length() != 1) {
        return;
    }

    auto item = qobject_cast<PlayListItem *>(itemWidget(items.first()));
    if (!item) {
        return;
    }
    auto m_data = item->data();
    if (!m_data) {
        return;
    }

    auto itemPos = item->mapFromParent(pos);
    if (!item->rect().contains(itemPos)) {
        return;
    }

    QPoint globalPos = this->mapToGlobal(pos);

    QMenu menu;
    menu.setStyle(QStyleFactory::create("dlight"));
    auto playact = menu.addAction(tr("Play"));
    playact->setDisabled(0 == m_data->length());

    if (m_data->id() != AllMusicListID && m_data->id() != FavMusicListID) {
        menu.addAction(tr("Rename"));
        menu.addAction(tr("Delete"));
    }

    connect(&menu, &QMenu::triggered, this, [ = ](QAction * action) {
        if (action->text() == tr("Play")) {
            emit item->playall(m_data);
        }
        if (action->text() == tr("Rename")) {
            item->onRename();

        }
        if (action->text() == tr("Delete")) {
            item->onDelete();
        }
    });

    menu.exec(globalPos);
}
