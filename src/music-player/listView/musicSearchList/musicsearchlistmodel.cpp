// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicsearchlistmodel.h"
#include <QDebug>

MusicSearchListModel::MusicSearchListModel(int rows, int columns, QObject *parent):
    QStandardItemModel(rows, columns, parent)
{

}

MusicSearchListModel::~MusicSearchListModel()
{

}

bool MusicSearchListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

//Qt::DropActions MusicSearchListModel::supportedDropActions() const
//{
//    return Qt::CopyAction | Qt::MoveAction;
//}

Qt::ItemFlags MusicSearchListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}
