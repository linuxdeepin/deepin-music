/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musiclistview.h"

#include <QDebug>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QScrollBar>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QStyle>
#include <QUrl>
#include <QProcess>

#include <dthememanager.h>

#include "../../musicapp.h"
#include "../../core/music.h"
#include "../../core/playlist.h"
#include "../../core/lyricservice.h"
#include "../helper/widgethellper.h"

#include "musicitemdelegate.h"
#include "infodialog.h"

DWIDGET_USE_NAMESPACE

MusicListView::MusicListView(QWidget *parent) : QTableView(parent)
{
    setObjectName("MusicListView");

    m_model = new QStandardItemModel(0, 5, this);
    this->setModel(m_model);

    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(false);

    setSelectionMode(QTableView::ExtendedSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setShowGrid(false);

    QHeaderView *verticalHeader = this->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(36);

    QHeaderView *headerView = this->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = +1; i < MusicItemDelegate::ColumnButt; ++i) {
        headerView->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    headerView->setSectionResizeMode(MusicItemDelegate::Number, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(MusicItemDelegate::Title, QHeaderView::Stretch);
    headerView->setSectionResizeMode(MusicItemDelegate::Artist, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(MusicItemDelegate::Album, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(MusicItemDelegate::Length, QHeaderView::ResizeToContents);

    this->setItemDelegate(new MusicItemDelegate);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListView::customContextMenuRequested,
            this, &MusicListView::requestCustomContextMenu);

    m_scrollBar = new QScrollBar(this);
    m_scrollBar->setObjectName("MusicViewScrollBar");
    m_scrollBar->setOrientation(Qt::Vertical);
    m_scrollBar->raise();

    connect(m_scrollBar, &QScrollBar::valueChanged,
    this, [ = ](int value) {
        verticalScrollBar()->setValue(value);
    });
    D_THEME_INIT_WIDGET(MusicListView);
}

void MusicListView::wheelEvent(QWheelEvent *event)
{
    QTableView::wheelEvent(event);

    m_scrollBar->setSliderPosition(verticalScrollBar()->sliderPosition());
}

void MusicListView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);

    auto size = event->size();
    auto scrollBarWidth = 8;
    m_scrollBar->resize(scrollBarWidth, size.height());
    m_scrollBar->move(size.width() - scrollBarWidth - 3, 0);
    m_scrollBar->setMaximum(verticalScrollBar()->maximum());
    m_scrollBar->setMinimum(verticalScrollBar()->minimum());
    m_scrollBar->setPageStep(verticalScrollBar()->pageStep());
}


void MusicListView::showContextMenu(const QPoint &pos,
                                    PlaylistPtr selectedPlaylist,
                                    PlaylistPtr favPlaylist,
                                    QList<PlaylistPtr > newPlaylists)
{
    QItemSelectionModel *selection = this->selectionModel();

    QPoint globalPos = this->mapToGlobal(pos);

    QMenu playlistMenu;
    bool hasAction = false;

    if (selectedPlaylist != favPlaylist) {
        hasAction = true;
        auto act = playlistMenu.addAction(favPlaylist->displayName());
        act->setData(QVariant::fromValue(favPlaylist));
    }

    for (auto playlist : newPlaylists) {
        auto act = playlistMenu.addAction(playlist->displayName());
        act->setData(QVariant::fromValue(playlist));
        hasAction = true;
    }

    if (hasAction) {
        playlistMenu.addSeparator();
    }
    auto newvar = QVariant::fromValue(PlaylistPtr());
    playlistMenu.addAction(tr("New playlist"))->setData(newvar);

    connect(&playlistMenu, &QMenu::triggered, this, [ = ](QAction * action) {
        auto playlist = action->data().value<PlaylistPtr >();
        MusicMetaList metalist;
        for (auto &index : selection->selectedRows()) {
            auto item = this->model()->item(index.row(), index.column());
            if (item) {
                metalist << qvariant_cast<MusicMeta>(item->data());
            }
        }
        emit addToPlaylist(playlist, metalist);
    });

    bool singleSelect = (1 == selection->selectedRows().length());

    QMenu myMenu;

    if (singleSelect) {
        myMenu.addAction(tr("Play"));
    }
    myMenu.addAction(tr("Add to playlist"))->setMenu(&playlistMenu);
    myMenu.addSeparator();

    if (singleSelect) {
        myMenu.addAction(tr("Display in file manager"));
    }
    myMenu.addAction(tr("Remove from list"));
    myMenu.addAction(tr("Delete"));

    if (singleSelect) {
        myMenu.addSeparator();
        myMenu.addAction(tr("Song info"));
    }

    connect(&myMenu, &QMenu::triggered, this, [ = ](QAction * action) {
        if (action->text() == tr("Play")) {
            auto index = selection->selectedRows().first();
            auto item = this->model()->item(index.row(), index.column());
            MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
            emit play(meta);
        }

        if (action->text() == tr("Display in file manager")) {
            auto index = selection->selectedRows().first();
            auto item = this->model()->item(index.row(), index.column());
            MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
            auto dirUrl = QUrl::fromLocalFile(QFileInfo(meta.localPath).absoluteDir().absolutePath());
            QFileInfo ddefilemanger("/usr/bin/dde-file-manager");
            if (ddefilemanger.exists()) {
                auto dirFile = QUrl::fromLocalFile(QFileInfo(meta.localPath).absoluteFilePath());
                auto url = QString("%1?selectUrl=%2").arg(dirUrl.toString()).arg(dirFile.toString());
                QProcess::startDetached("dde-file-manager" , QStringList() << url);
            } else {
                QProcess::startDetached("gvfs-open" , QStringList() << dirUrl.toString());
            }
        }

        if (action->text() == tr("Remove from list")) {
            MusicMetaList metalist;
            for (auto index : selection->selectedRows()) {
                auto item = this->model()->item(index.row(), index.column());
                MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
                metalist << meta;
            }
            emit removeMusicList(metalist);
        }

        if (action->text() == tr("Delete")) {
            MusicMetaList metalist;
            for (auto index : selection->selectedRows()) {
                auto item = this->model()->item(index.row(), index.column());
                MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
                metalist << meta;
            }

            DDialog warnDlg;
            warnDlg.setTextFormat(Qt::AutoText);
            warnDlg.addButtons(QStringList() << tr("Cancel") << tr("Delete"));

            auto coverPath = QString(":/image/cover_max.png");
            if (1 == metalist.length()) {
                auto meta = metalist.first();
                QFileInfo coverfi(LyricService::coverPath(meta));
                if (coverfi.exists()) {
                    coverPath = coverfi.absoluteFilePath();
                }
                warnDlg.setMessage(
                    QString(tr("Are you sure to delete %1?")).arg(meta.title));
            } else {
                warnDlg.setMessage(
                    QString(tr("TODO: Are you sure to delete %1 songs?")).arg(metalist.length()));
            }

            auto cover = WidgetHelper::coverPixmap(coverPath, QSize(64, 64));

            warnDlg.setIcon(QIcon(cover));
            if (0 == warnDlg.exec()) {
                return;
            }
            emit deleteMusicList(metalist);
        }

        if (action->text() == tr("Song info")) {
            auto index = selection->selectedRows().first();
            auto item = this->model()->item(index.row(), index.column());
            MusicMeta meta = qvariant_cast<MusicMeta>(item->data());
            auto coverPath = QString(":/image/info_cover.png");
            QFileInfo coverfi(LyricService::coverPath(meta));
            if (coverfi.exists()) {
                coverPath = coverfi.absoluteFilePath();
            }
            auto cover = WidgetHelper::coverPixmap(coverPath, QSize(140, 140));
            InfoDialog dlg(meta, cover, this);
            dlg.exec();
        }
    });

    myMenu.exec(globalPos);
}
