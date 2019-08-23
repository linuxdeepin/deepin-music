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

#include "musiclistdataview.h"

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
#include <DDesktopServices>
#include <DThemeManager>

#include "../../core/metasearchservice.h"
#include "../helper/widgethellper.h"

#include "delegate/musiclistdatadelegate.h"
#include "model/musiclistdatamodel.h"

DWIDGET_USE_NAMESPACE

class MusicListDataViewPrivate
{
public:
    MusicListDataViewPrivate(MusicListDataView *parent): q_ptr(parent) {}

    void addPlayMusicTypePtr(const PlayMusicTypePtr TypePtr);

    MusicListDataModel      *model        = nullptr;
    MusicListDataDelegate   *delegate     = nullptr;
    QString                 defaultCover = ":/common/image/cover_max.svg";

    MusicListDataView *q_ptr;
    Q_DECLARE_PUBLIC(MusicListDataView)
};


void MusicListDataViewPrivate::addPlayMusicTypePtr(const PlayMusicTypePtr TypePtr)
{
    QStandardItem *newItem = new QStandardItem;
    QIcon icon(defaultCover);
    if (!TypePtr->icon.isEmpty()) {
        icon = QIcon(QPixmap::fromImage(QImage::fromData(TypePtr->icon)));
    }
    newItem->setIcon(icon);
    model->appendRow(newItem);

    auto row = model->rowCount() - 1;
    QModelIndex index = model->index(row, 0, QModelIndex());
    model->setData(index, TypePtr->name);
}


MusicListDataView::MusicListDataView(QWidget *parent)
    : ListView(parent), d_ptr(new MusicListDataViewPrivate(this))
{
    Q_D(MusicListDataView);

    DThemeManager::instance()->registerWidget(this);

    setObjectName("MusicListDataView");

    d->model = new MusicListDataModel(0, 1, this);
    setModel(d->model);

    d->delegate = new MusicListDataDelegate;
    setItemDelegate(d->delegate);

    setViewModeFlag(QListView::ListMode);
    setResizeMode( QListView::Adjust );
    setMovement( QListView::Static );
}

MusicListDataView::~MusicListDataView()
{

}

PlaylistPtr MusicListDataView::playlist() const
{
    Q_D(const MusicListDataView);
    return d->model->playlist();
}

void MusicListDataView::setViewModeFlag(QListView::ViewMode mode)
{
    if (mode == QListView::IconMode) {
        setIconSize( QSize(150, 150) );
        setGridSize( QSize(150, 150) );
    } else {
        setIconSize( QSize(36, 36) );
        setGridSize( QSize(36, 36) );
    }
    setViewMode(mode);
}

void MusicListDataView::onMusiclistChanged(PlaylistPtr playlist)
{
    Q_D(MusicListDataView);

    if (playlist.isNull()) {
        qWarning() << "can not change to emptry playlist";
        return;
    }

    d->model->removeRows(0, d->model->rowCount());

    for (auto meta : playlist->playMusicTypePtrList()) {
        d->addPlayMusicTypePtr(meta);
    }

    d->model->setPlaylist(playlist);
    updateScrollbar();
}
