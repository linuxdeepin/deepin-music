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

#include "musiclistinfoview.h"

#include <QDebug>
#include <DMenu>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QStyleFactory>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include <DDialog>
#include <DDesktopServices>
#include <DScrollBar>
#include <DLabel>

#include "player.h"
#include "global.h"

DWIDGET_USE_NAMESPACE

MusicListInfoView::MusicListInfoView(QWidget *parent)
    : QListView(parent)
{
    setFrameShape(QFrame::NoFrame);

    setAutoFillBackground(true);
    auto palette = this->palette();
    QColor BackgroundColor("#FFFFFF");
    palette.setColor(DPalette::Background, BackgroundColor);
    setPalette(palette);
    setIconSize(QSize(36, 36));
    //    setGridSize( QSize(36, 36) );
    m_model = new MusiclistInfomodel(0, 1, this);
    setModel(m_model);

    delegate = new MusicInfoItemDelegate;
    setItemDelegate(delegate);

    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropOverwriteMode(false);
    //setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    //setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Free);
    setLayoutMode(QListView::Batched);

    setSelectionMode(QListView::ExtendedSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MusicListInfoView::customContextMenuRequested,
            this, &MusicListInfoView::requestCustomContextMenu);

    connect(this, &MusicListInfoView::doubleClicked, this, &MusicListInfoView::onDoubleClicked);
}

MusicListInfoView::~MusicListInfoView()
{

}

//MetaPtr MusicListInfoView::activingMeta() const
//{
//    Q_D(const MusicListInfoView);

//    if (d->model->playlist().isNull()) {
//        return MetaPtr();
//    }

//    return d->model->playlist()->playing();
//}

//MetaPtr MusicListInfoView::firstMeta() const
//{
//    MetaPtr curMeta = nullptr;
//    for (int i = 0; i < m_model->rowCount(); ++i) {
//        auto index = m_model->index(i, 0);
//        MetaPtr meta = m_model->meta(index);
//        if (!meta->invalid) {
//            curMeta = meta;
//            break;
//        } else {
//            if (QFile::exists(meta->localPath)) {
//                curMeta = meta;
//                break;
//            }
//        }
//    }
//    return curMeta;
//}

//PlaylistPtr MusicListInfoView::playlist() const
//{
//    Q_D(const MusicListInfoView);
//    return d->model->playlist();
//}


//QString MusicListInfoView::curName() const
//{
//    Q_D(const MusicListInfoView);
//    return d->curName;
//}

//MetaPtr MusicListInfoView::playing() const
//{
//    return playing;
//}

void MusicListInfoView::setThemeType(int type)
{
    themeType = type;
}

int MusicListInfoView::getThemeType() const
{
    return themeType;
}

void MusicListInfoView::setPlayPixmap(QPixmap pixmap, QPixmap sidebarPixmap)
{
    playingPixmap = pixmap;
    sidebarPixmap = sidebarPixmap;
//    auto index = d->model->findIndex(d->playing);
//    if (index.isValid())
//        update(index);

    viewport()->update();
}

QPixmap MusicListInfoView::getPlayPixmap() const
{
    return playingPixmap;
}

QPixmap MusicListInfoView::getSidebarPixmap() const
{
    return sidebarPixmap;
}

QStringList MusicListInfoView::allMetaNames() const
{
    QStringList allMetaNames;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto index = m_model->index(i, 0);
        auto hash = m_model->data(index).toString();
        if (!hash.isEmpty()) {
            allMetaNames.append(hash);
        }
    }
    return allMetaNames;
}

void MusicListInfoView::setMusicListView(QMap<QString, MediaMeta> musicinfos)
{
    m_model->removeRows(0, m_model->rowCount());
    for (auto item : musicinfos) {
        addMedia(item);
    }
}

//void MusicListInfoView::onMusiclistChanged(PlaylistPtr playlist, const QString name)
//{
//    Q_D(MusicListInfoView);

//    if (playlist.isNull()) {
//        qWarning() << "can not change to emptry playlist";
//        return;
//    }

//    d->curName     = name;

//    d->model->removeRows(0, d->model->rowCount());
//    for (auto TypePtr : playlist->playMusicTypePtrList()) {
//        if (TypePtr->name == name) {
//            for (auto metaHash : TypePtr->playlistMeta.sortMetas) {
//                if (TypePtr->playlistMeta.metas.contains(metaHash))
//                    d->addMedia(TypePtr->playlistMeta.metas[metaHash]);
//            }
//        }
//    }

//    d->model->setPlaylist(playlist);
//    //updateScrollbar();
//}

void MusicListInfoView::keyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            QItemSelectionModel *selection = this->selectionModel();
//            removeSelection(selection);
            break;
        }
        break;
    case Qt::ShiftModifier:
        switch (event->key()) {
        case Qt::Key_Delete:
            break;
        }
        break;
    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_I:
            QItemSelectionModel *selection = this->selectionModel();
            if (selection->selectedRows().length() <= 0) {
                return;
            }
            auto index = selection->selectedRows().first();
//            auto meta = m_model->meta(index);
//            Q_EMIT showInfoDialog(meta);
            break;
        }
        break;
    //    case Qt::ControlModifier:
    //        switch (event->key()) {
    //        case Qt::Key_K:
    //            QItemSelectionModel *selection = this->selectionModel();
    //            if (selection->selectedRows().length() > 0) {
    //                MetaPtrList metalist;
    //                for (auto index : selection->selectedRows()) {
    //                    auto meta = d->model->meta(index);
    //                    metalist << meta;
    //                }
    //                if (!metalist.isEmpty())
    //                    Q_EMIT addMetasFavourite(metalist);
    //            }
    //            break;
    //        }
    //        break;
    //    case Qt::ControlModifier | Qt::ShiftModifier:
    //        switch (event->key()) {
    //        case Qt::Key_K:
    //            QItemSelectionModel *selection = this->selectionModel();
    //            if (selection->selectedRows().length() > 0) {
    //                MetaPtrList metalist;
    //                for (auto index : selection->selectedRows()) {
    //                    auto meta = d->model->meta(index);
    //                    metalist << meta;
    //                }
    //                if (!metalist.isEmpty())
    //                    Q_EMIT removeMetasFavourite(metalist);
    //            }
    //            break;
    //        }
    //        break;
    default:
        break;
    }

    QAbstractItemView::keyPressEvent(event);
}

void MusicListInfoView::keyboardSearch(const QString &search)
{
    Q_UNUSED(search);
// Disable keyborad serach
//    qDebug() << search;
//    QAbstractItemView::keyboardSearch(search);
}

void MusicListInfoView::addMedia(MediaMeta meta)
{
    QStandardItem *newItem = new QStandardItem;
    QFileInfo coverInfo(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
    if (coverInfo.exists()) {
        QIcon icon = QIcon(Global::cacheDir() + "/images/" + meta.hash + ".jpg");
        newItem->setIcon(icon);
    } else {
        newItem->setIcon(m_icon);
    }
    m_model->appendRow(newItem);

    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());
    QVariant metaVar;
    metaVar.setValue(meta);
    m_model->setData(index, metaVar, Qt::UserRole);
}

void MusicListInfoView::onDoubleClicked(const QModelIndex &index)
{
    MediaMeta meta = index.data(Qt::UserRole).value<MediaMeta>();

    Player::instance()->playMeta(meta);
}

void MusicListInfoView::dragEnterEvent(QDragEnterEvent *event)
{
    QListView::dragEnterEvent(event);
}

void MusicListInfoView::startDrag(Qt::DropActions supportedActions)
{

}

