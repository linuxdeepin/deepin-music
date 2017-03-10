/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QListWidget>
#include <QScopedPointer>

class PlayListViewPrivate;
class PlayListView : public QListWidget
{
    Q_OBJECT
public:
    explicit PlayListView(QWidget *parent = 0);
    ~PlayListView();

    void updateScrollbar();
    void showContextMenu(const QPoint &pos);

signals:
    void customResort(const QStringList &uuids);

protected:
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;

private:
    QScopedPointer<PlayListViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), PlayListView)
};
