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
#include <QMenu>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QStyleFactory>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QScrollBar>
#include <ddialog.h>

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"
#include "../helper/thememanager.h"

#include "delegate/musicitemdelegate.h"
#include "model/musiclistmodel.h"

class MusicListViewPrivate
{
public:
    MusicListViewPrivate(MusicListView *parent): q_ptr(parent) {}

    void addMedia(const MetaPtr meta);
    void removeSelection(QItemSelectionModel *selection);

    MusiclistModel      *model        = nullptr;
    MusicItemDelegate   *delegate     = nullptr;

    MusicListView *q_ptr;
    Q_DECLARE_PUBLIC(MusicListView)
};

MusicListView::MusicListView(QWidget *parent)
    : ListView(parent), d_ptr(new MusicListViewPrivate(this))
{
    Q_D(MusicListView);

    ThemeManager::instance()->regisetrWidget(this);

    setObjectName("MusicListView");

    d->model = new MusiclistModel(0, 1, this);
    setModel(d->model);

    d->delegate = new MusicItemDelegate;
    setItemDelegate(d->delegate);

    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropOverwriteMode(false);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::InternalMove);
    setMovement(QListView::Free);

    setSelectionMode(QListView::ExtendedSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListView::customContextMenuRequested,
            this, &MusicListView::requestCustomContextMenu);

    connect(this, &MusicListView::doubleClicked,
    this, [ = ](const QModelIndex & index) {
        MetaPtr meta = d->model->meta(index);
        emit playMedia(meta);
    });

    // For debug
//    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
//    this, [ = ](const QItemSelection & /*selected*/, const QItemSelection & deselected) {
//        if (!deselected.isEmpty()) {
//            qDebug() << "cancel" << deselected;
//        }
//    });
}

MusicListView::~MusicListView()
{

}

MetaPtr MusicListView::activingMeta() const
{
    Q_D(const MusicListView);

    if (d->model->playlist().isNull()) {
        return MetaPtr();
    }

    return d->model->playlist()->playing();
}

PlaylistPtr MusicListView::playlist() const
{
    Q_D(const MusicListView);
    return d->model->playlist();
}

QModelIndex MusicListView::findIndex(const MetaPtr meta)
{
    Q_ASSERT(!meta.isNull());
    Q_D(MusicListView);

    return d->model->findIndex(meta);
}

void MusicListView::onMusicListRemoved(const MetaPtrList metalist)
{
    Q_D(MusicListView);

    setAutoScroll(false);
    for (auto meta : metalist) {
        if (meta.isNull()) {
            continue;
        }

        for (int i = 0; i < d->model->rowCount(); ++i) {
            auto index = d->model->index(i, 0);
            auto itemHash = d->model->data(index).toString();
            if (itemHash == meta->hash) {
                d->model->removeRow(i);
            }
        }
    }
    updateScrollbar();
    setAutoScroll(true);
}

void MusicListView::onMusicError(const MetaPtr meta, int /*error*/)
{
    Q_ASSERT(!meta.isNull());
//    Q_D(MusicListView);

//    qDebug() << error;
//    QModelIndex index = findIndex(meta);

//    auto indexData = index.data().value<MetaPtr>();
//    indexData.invalid = (error != 0);
//    d->m_model->setData(index, QVariant::fromValue<MetaPtr>(indexData));

    update();
}

void MusicListView::onMusicListAdded(const MetaPtrList metalist)
{
    Q_D(MusicListView);
    for (auto meta : metalist) {
        d->addMedia(meta);
    }
    updateScrollbar();
}

void MusicListView::onLocate(const MetaPtr meta)
{
    QModelIndex index = findIndex(meta);
    if (!index.isValid()) {
        return;
    }

    clearSelection();

    auto viewRect = QRect(QPoint(0, 0), size());
    if (!viewRect.intersects(visualRect(index))) {
        scrollTo(index, MusicListView::PositionAtCenter);
    }
    setCurrentIndex(index);
}

void MusicListView::onMusiclistChanged(PlaylistPtr playlist)
{
    Q_D(MusicListView);

    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    d->model->removeRows(0, d->model->rowCount());
    for (auto meta : playlist->allmusic()) {
//        qDebug() << meta->hash << meta->title;
        d->addMedia(meta);
    }

    d->model->setPlaylist(playlist);
    updateScrollbar();
}

void MusicListView::keyPressEvent(QKeyEvent *event)
{
    Q_D(MusicListView);
    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            QItemSelectionModel *selection = this->selectionModel();
            d->removeSelection(selection);
            break;
        }
        break;
    case Qt::ShiftModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            break;
        }
        break;
    default: break;
    }

    QAbstractItemView::keyPressEvent(event);
}

void MusicListViewPrivate::addMedia(const MetaPtr meta)
{
    QStandardItem *newItem = new QStandardItem;
    model->appendRow(newItem);

    auto row = model->rowCount() - 1;
    QModelIndex index = model->index(row, 0, QModelIndex());
    model->setData(index, meta->hash);
}

void MusicListViewPrivate::removeSelection(QItemSelectionModel *selection)
{
    Q_ASSERT(selection != nullptr);
    Q_Q(MusicListView);

    MetaPtrList metalist;
    for (auto index : selection->selectedRows()) {
        auto meta = model->meta(index);
        metalist << meta;
    }
    emit q->removeMusicList(metalist);
}

void MusicListView::showContextMenu(const QPoint &pos,
                                    PlaylistPtr selectedPlaylist,
                                    PlaylistPtr favPlaylist,
                                    QList<PlaylistPtr> newPlaylists)
{
    Q_D(MusicListView);
    QItemSelectionModel *selection = this->selectionModel();

    if (selection->selectedRows().length() <= 0) {
        return;
    }

    QPoint globalPos = this->mapToGlobal(pos);

    QMenu playlistMenu;
    playlistMenu.setStyle(QStyleFactory::create("dlight"));

    auto newvar = QVariant::fromValue(PlaylistPtr());

    auto createPlaylist = playlistMenu.addAction(tr("New playlist"));
    createPlaylist->setData(newvar);
    createPlaylist->setIcon(QIcon(":/light/image/plus.png"));

    playlistMenu.addSeparator();

    if (selectedPlaylist != favPlaylist) {
        auto act = playlistMenu.addAction(favPlaylist->displayName());
        act->setData(QVariant::fromValue(favPlaylist));
    }

    for (auto playlist : newPlaylists) {
        QFont font(playlistMenu.font());
        QFontMetrics fm(font);
        auto text = fm.elidedText(QString(playlist->displayName().replace("&", "&&")),
                                  Qt::ElideMiddle, 160);
        auto act = playlistMenu.addAction(text);
        act->setData(QVariant::fromValue(playlist));
    }

    connect(&playlistMenu, &QMenu::triggered, this, [ = ](QAction * action) {
        auto playlist = action->data().value<PlaylistPtr >();
        qDebug() << playlist;
        MetaPtrList metalist;
        for (auto &index : selection->selectedRows()) {
            auto meta = d->model->meta(index);
            if (!meta.isNull()) {
                metalist << meta;
            }
        }
        emit addToPlaylist(playlist, metalist);
    });

    bool singleSelect = (1 == selection->selectedRows().length());

    QMenu myMenu;
    myMenu.setStyle(QStyleFactory::create("dlight"));

    QAction *playAction = nullptr;
    if (singleSelect) {
        playAction = myMenu.addAction(tr("Play"));
    }
    myMenu.addAction(tr("Add to playlist"))->setMenu(&playlistMenu);
    myMenu.addSeparator();

    QAction *displayAction = nullptr;
    if (singleSelect) {
        displayAction = myMenu.addAction(tr("Display in file manager"));
    }

    auto removeAction = myMenu.addAction(tr("Remove from playlist"));
    auto deleteAction = myMenu.addAction(tr("Delete from local disk"));

    QAction *songAction = nullptr;

    QMenu textCodecMenu;
    textCodecMenu.setStyle(QStyleFactory::create("dlight"));

    if (singleSelect) {
        auto index = selection->selectedRows().first();
        auto meta = d->model->meta(index);
        QList<QByteArray> codecList = DMusic::detectMetaEncodings(meta);
//        codecList << "utf-8" ;
        for (auto codec : codecList) {
            auto act = textCodecMenu.addAction(codec);
            act->setData(QVariant::fromValue(codec));
        }

        if (codecList.length() > 1) {
            myMenu.addSeparator();
            myMenu.addAction(tr("Encoding"))->setMenu(&textCodecMenu);
        }

        myMenu.addSeparator();
        songAction = myMenu.addAction(tr("Song info"));

        connect(&textCodecMenu, &QMenu::triggered, this, [ = ](QAction * action) {
            auto codec = action->data().toByteArray();
            meta->updateCodec(codec);
            emit updateMetaCodec(meta);
        });
    }

    if (playAction) {
        connect(playAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            emit playMedia(d->model->meta(index));
        });
    }

    if (displayAction) {
        connect(displayAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            auto meta = d->model->meta(index);

            auto dirUrl = QUrl::fromLocalFile(QFileInfo(meta->localPath).absoluteDir().absolutePath());
            QFileInfo ddefilemanger("/usr/bin/dde-file-manager");
            if (ddefilemanger.exists()) {
                auto dirFile = QUrl::fromLocalFile(QFileInfo(meta->localPath).absoluteFilePath());
                auto url = QString("%1?selectUrl=%2").arg(dirUrl.toString()).arg(dirFile.toString());
                QProcess::startDetached("dde-file-manager" , QStringList() << url);
            } else {
                QProcess::startDetached("gvfs-open" , QStringList() << dirUrl.toString());
            }
        });
    }

    if (removeAction) {
        connect(removeAction, &QAction::triggered, this, [ = ](bool) {
            d->removeSelection(selection);
        });
    }

    if (deleteAction) {
        connect(deleteAction, &QAction::triggered, this, [ = ](bool) {
            bool containsCue = false;
            MetaPtrList metalist;
            for (auto index : selection->selectedRows()) {
                auto meta = d->model->meta(index);
                if (!meta->cuePath.isEmpty()) {
                    containsCue = true;
                }
                metalist << meta;
            }

            Dtk::Widget::DDialog warnDlg(this);
            warnDlg.setStyle(QStyleFactory::create("dlight"));
            warnDlg.setTextFormat(Qt::RichText);
            warnDlg.addButton(tr("Cancel"), true , Dtk::Widget::DDialog::ButtonWarning);
            warnDlg.addButton(tr("Delete"), false, Dtk::Widget::DDialog::ButtonNormal);

            auto cover = QImage(QString(":/common/image/del_notify.png"));
            if (1 == metalist.length()) {
                auto meta = metalist.first();
                auto coverData = MetaSearchService::coverData(meta);
                if (coverData.length() > 0) {
                    cover = QImage::fromData(coverData);
                }
                warnDlg.setMessage(QString(tr("Are you sure to delete %1?")).arg(meta->title));
            } else {
                warnDlg.setMessage(QString(tr("Are you sure to delete the selected %1 songs?")).arg(metalist.length()));
            }

            if (containsCue) {
                warnDlg.setTitle(tr("Are you sure to delete the selected %1 songs?").arg(metalist.length()));
                warnDlg.setMessage(tr("Deleting the current song will also delete the song files contained"));
            }
            auto coverPixmap =  QPixmap::fromImage(WidgetHelper::cropRect(cover, QSize(64, 64)));

            warnDlg.setIcon(QIcon(coverPixmap));
            if (0 == warnDlg.exec()) {
                return;
            }
            emit deleteMusicList(metalist);
        });
    }

    if (songAction) {
        connect(songAction, &QAction::triggered, this, [ = ](bool) {
            auto index = selection->selectedRows().first();
            auto meta = d->model->meta(index);
            emit showInfoDialog(meta);
        });
    }

    myMenu.exec(globalPos);
}

void MusicListView::dragEnterEvent(QDragEnterEvent *event)
{
    ListView::dragEnterEvent(event);
}

void MusicListView::startDrag(Qt::DropActions supportedActions)
{
    Q_D(MusicListView);

    MetaPtrList list;
    for (auto index : selectionModel()->selectedIndexes()) {
        list << d->model->meta(index);
    }

    setAutoScroll(false);
    ListView::startDrag(supportedActions);
    setAutoScroll(true);

    QMap<QString, int> hashIndexs;
    for (int i = 0; i < d->model->rowCount(); ++i) {
        auto index = d->model->index(i, 0);
        auto hash = d->model->data(index).toString();
        Q_ASSERT(!hash.isEmpty());
        hashIndexs.insert(hash, i);
    }
    d->model->playlist()->saveSort(hashIndexs);
    emit customSort();

    QItemSelection selection;
    for (auto meta : list) {
        if (!meta.isNull()) {
            auto index = this->findIndex(meta);
            selection.append(QItemSelectionRange(index));
        }
    }
    if (!selection.isEmpty()) {
        selectionModel()->select(selection, QItemSelectionModel::Select);
    }
}

