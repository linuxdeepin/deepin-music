// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QStandardItemModel>
#include <QScopedPointer>

#include <mediameta.h>

class PlaylistModel : public QStandardItemModel
{
    Q_OBJECT
public:
    PlaylistModel(int rows, int columns, QObject *parent = Q_NULLPTR);
    ~PlaylistModel() override;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
};

