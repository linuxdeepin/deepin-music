/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "hoverfilter.h"

#include <QDebug>

#include <QEvent>
#include <QApplication>
#include <QCursor>

HoverFilter::HoverFilter(QObject *parent) : QObject(parent)
{

}

bool HoverFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        return true;
    case QEvent::Leave:
        QApplication::restoreOverrideCursor();
        return true;
    default:
        return QObject::eventFilter(obj, event);
    }
}
