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

