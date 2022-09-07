// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicbaseandsonglistmodel.h"

#include <QDebug>
#include <QMimeData>
#include <DStandardItem>

DWIDGET_USE_NAMESPACE

MusicBaseAndSonglistModel::MusicBaseAndSonglistModel(QObject *parent) :
    QStandardItemModel(parent)
{

}

MusicBaseAndSonglistModel::~MusicBaseAndSonglistModel()
{

}

bool MusicBaseAndSonglistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

Qt::DropActions MusicBaseAndSonglistModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags MusicBaseAndSonglistModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    return Qt::ItemIsDragEnabled | defaultFlags;
}

