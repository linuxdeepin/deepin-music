/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QScopedPointer>

class HoverFilter : public QObject
{
    Q_OBJECT
public:
    explicit HoverFilter(QObject *parent = 0);
    bool eventFilter(QObject *obj, QEvent *event);
};

class HoverShadowFilter : public QObject
{
    Q_OBJECT
public:
    explicit HoverShadowFilter(QObject *parent = 0);
    bool eventFilter(QObject *obj, QEvent *event);
};


class HintFilterPrivate;
class HintFilter: public QObject
{
    Q_OBJECT
public:
    HintFilter(QObject *parent = 0);
    ~HintFilter();

    void hideAll();
    bool eventFilter(QObject *obj, QEvent *event);
    void showHitsFor(QWidget *w, QWidget *hint);
private:
    QScopedPointer<HintFilterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), HintFilter)
};
