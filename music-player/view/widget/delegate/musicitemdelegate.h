/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QStyledItemDelegate>

class MusicItemDelegatePrivate;
class MusicItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum MusicColumn {
        Number  = 0,
        Title,
        Artist,
        Album,
        Length,

        ColumnButt,
    };
    Q_ENUM(MusicColumn)

    enum MusicItemDataRole {
        PlayStatusRole = Qt::UserRole + 100,
    };
    Q_ENUM(MusicItemDataRole)

    enum PlayStatus {
        Playing = 0x0098,
        Strop,
    };
    Q_ENUM(PlayStatus)

    MusicItemDelegate(QWidget *parent = 0);
    ~MusicItemDelegate();

public:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
    QScopedPointer<MusicItemDelegatePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), MusicItemDelegate)
};

