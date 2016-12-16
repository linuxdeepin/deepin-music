/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef HOVERFILTER_H
#define HOVERFILTER_H

#include <QObject>

class HoverFilter : public QObject
{
    Q_OBJECT
public:
    explicit HoverFilter(QObject *parent = 0);

    bool eventFilter(QObject *obj, QEvent *event);
signals:

public slots:
};

#endif // HOVERFILTER_H
