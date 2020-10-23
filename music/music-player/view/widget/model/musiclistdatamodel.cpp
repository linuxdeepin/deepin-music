/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include "musiclistdatamodel.h"

#include <QDebug>
#include "core/medialibrary.h"

class MusicListDataModelPrivate
{
public:
    explicit MusicListDataModelPrivate(MusicListDataModel *parent) : q_ptr(parent) {}

    PlaylistPtr playlist;

    MusicListDataModel *q_ptr;
    Q_DECLARE_PUBLIC(MusicListDataModel)
};

MusicListDataModel::MusicListDataModel(QObject *parent) :
    QStandardItemModel(parent), d_ptr(new MusicListDataModelPrivate(this))
{

}

MusicListDataModel::MusicListDataModel(int rows, int columns, QObject *parent):
    QStandardItemModel(rows, columns, parent), d_ptr(new MusicListDataModelPrivate(this))
{

}

MusicListDataModel::~MusicListDataModel()
{

}

PlaylistPtr MusicListDataModel::playlist() const
{
    Q_D(const MusicListDataModel);
    return d->playlist;
}

void MusicListDataModel::setPlaylist(PlaylistPtr playlist)
{
    Q_D(MusicListDataModel);
    d->playlist = playlist;
}

bool MusicListDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

Qt::DropActions MusicListDataModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags MusicListDataModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}
