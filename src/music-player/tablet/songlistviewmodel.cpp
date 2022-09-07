// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "songlistviewmodel.h"

#include <QDebug>

SongListViewModel::SongListViewModel(QObject *parent) :
    QStandardItemModel(parent)
{

}

SongListViewModel::~SongListViewModel()
{

}

bool SongListViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

Qt::ItemFlags SongListViewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDropEnabled | defaultFlags;
    else
        return defaultFlags;
}
