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

#include "musiclistmodel.h"

#include <QDebug>

class MusiclistModelPrivate
{
public:
    MusiclistModelPrivate(MusiclistModel *parent) : q_ptr(parent) {}

    MusiclistModel *q_ptr;
    Q_DECLARE_PUBLIC(MusiclistModel)
};

MusiclistModel::MusiclistModel(QObject *parent) :
    QStandardItemModel(parent), d_ptr(new MusiclistModelPrivate(this))
{

}

MusiclistModel::~MusiclistModel()
{

}

bool MusiclistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

Qt::DropActions MusiclistModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

Qt::ItemFlags MusiclistModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDropEnabled | defaultFlags;
    else
        return defaultFlags;
}

