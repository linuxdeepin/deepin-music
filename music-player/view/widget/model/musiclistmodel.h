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

#pragma once

#include <QStandardItemModel>
#include <QScopedPointer>

#include <mediameta.h>
#include "core/playlist.h"

class MusiclistModelPrivate;
class MusiclistModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit MusiclistModel(QObject *parent = 0);
    MusiclistModel(int rows, int columns, QObject *parent = Q_NULLPTR);
    ~MusiclistModel();

    PlaylistPtr playlist() const;
    void setPlaylist(PlaylistPtr playlist);

    MetaPtr meta(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QMap<QString, int> hashIndexs();
    QModelIndex findIndex(const MetaPtr meta);

//    Q_INVOKABLE virtual QModelIndex index(int row, int column,
//                              const QModelIndex &parent = QModelIndex()) const = 0;
//    Q_INVOKABLE virtual QModelIndex parent(const QModelIndex &child) const = 0;

//    Q_INVOKABLE virtual QModelIndex sibling(int row, int column, const QModelIndex &idx) const;
//    Q_INVOKABLE virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
//    Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
//    Q_INVOKABLE virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
//    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    virtual Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    QScopedPointer<MusiclistModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusiclistModel)
};

