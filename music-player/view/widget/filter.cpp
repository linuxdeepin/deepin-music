/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "filter.h"

#include <QDebug>

#include <QEvent>
#include <QApplication>
#include <QCursor>
#include <QWidget>

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


class HintFilterPrivate
{
public:
    HintFilterPrivate(HintFilter *parent) : q_ptr(parent) {}

    QWidget *hintWidget = nullptr;

    HintFilter *q_ptr;
    Q_DECLARE_PUBLIC(HintFilter);
};

HintFilter::HintFilter(QObject *parent)  : QObject(parent), d_ptr(new HintFilterPrivate(this))
{
    Q_D(HintFilter);
}

HintFilter::~HintFilter()
{

}

bool HintFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(HintFilter);
    switch (event->type()) {
    case QEvent::Enter: {
        auto w = qobject_cast<QWidget *>(obj);
        if (!w) {
            return true;
        }
        if (d->hintWidget) {
            d->hintWidget->hide();
        }
        d->hintWidget = w->property("HintWidget").value<QWidget *>();
        if (!d->hintWidget) {
            return true;
        }

        d->hintWidget->show();
        d->hintWidget->raise();

        auto centerPos = w->mapToGlobal(w->rect().center());
        auto sz = d->hintWidget->size();
        centerPos.setX(centerPos.x()  - sz.width() / 2);
        centerPos.setY(centerPos.y() - 32 - sz.height());
        centerPos = d->hintWidget->mapFromGlobal(centerPos);
        centerPos = d->hintWidget->mapToParent(centerPos);
        d->hintWidget->move(centerPos);

        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        return true;
    }
    case QEvent::Leave:
        qDebug() << obj << d->hintWidget << d->hintWidget->property("KeepShow");

        if (d->hintWidget) {
            if (!d->hintWidget->property("DelayHide").toBool()) {
                d->hintWidget->hide();
            } else {
                QMetaObject::invokeMethod(d->hintWidget, "deleyHide", Qt::DirectConnection);
            }
        }
        QApplication::restoreOverrideCursor();
        return true;
    default:
        return QObject::eventFilter(obj, event);
    }
}
