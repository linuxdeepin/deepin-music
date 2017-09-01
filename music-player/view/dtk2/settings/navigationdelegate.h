/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QScopedPointer>
#include <QStyledItemDelegate>

class NavigationDelegatePrivate;
class NavigationDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    NavigationDelegate(QWidget *parent = 0);
    ~NavigationDelegate();


    enum DataRole {
        NavLevelRole = Qt::UserRole + 100,
        NavKeyRole,
    };

    enum NavLevel {
        Split = 1001,
        Level1,
        Level2,
        Level3
    };

    virtual void initStyleOption(QStyleOptionViewItem *option,
                                 const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private:
    QScopedPointer<NavigationDelegatePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), NavigationDelegate)
};


