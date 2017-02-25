/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "ddropdown.h"

#include <QDebug>
#include <QMenu>
#include <QHBoxLayout>
#include <QStyle>
#include <QMouseEvent>
#include <QLabel>

#include <thememanager.h>

class DDropdownPrivate
{
public:
    DDropdownPrivate(DDropdown *parent) : q_ptr(parent) {}

    QMenu       *menu       = nullptr;
    QLabel      *text       = nullptr;
    QLabel      *dropdown   = nullptr;
    QString     status;

    DDropdown *q_ptr;
    Q_DECLARE_PUBLIC(DDropdown)
};

DDropdown::DDropdown(QWidget *parent) : QFrame(parent), d_ptr(new DDropdownPrivate(this))
{
    Q_D(DDropdown);

    setObjectName("DDropdown");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(15,0,15,0);
    d->menu = new QMenu;
    d->text = new QLabel("undefined");
    d->text->setObjectName("DDropdownText");
    d->dropdown = new QLabel;
    d->dropdown->setObjectName("DDropdownIcon");
    d->dropdown->setFixedSize(9,5);

    layout->addStretch();
    layout->addStretch();
    layout->addWidget(d->text, 0, Qt::AlignCenter);
    layout->addWidget(d->dropdown, 0, Qt::AlignCenter);

    connect(d->menu, &QMenu::triggered, this, [ = ](QAction * action) {
        d->text->setText(action->text());
        emit this->triggered(action);
    });

    connect(this, &DDropdown::requestContextMenu,
    this, [ = ]() {
        auto center = this->mapToGlobal(this->rect().topLeft());
        center.setY(center.y() + this->height() + 5);
        d->menu->setFixedWidth(this->width());
        d->menu->move(center);
        d->menu->exec();
    });

    ThemeManager::instance()->regisetrWidget(this);
}

DDropdown::~DDropdown()
{

}

QString DDropdown::status() const
{
    Q_D(const DDropdown);
    return d->status;
}

QList<QAction *> DDropdown::actions() const
{
    Q_D(const DDropdown);
    return d->menu->actions();
}

void DDropdown::setCurrentAction(QAction *action)
{
    Q_D(DDropdown);
    if (action) {
        for (auto action : d->menu->actions()) {
            action->setChecked(false);
        }
        d->text->setText(action->text());
        action->setChecked(true);
    }
}

QAction *DDropdown::addAction(const QString &item, const QVariant &var)
{
    Q_D(DDropdown);
    auto action = d->menu->addAction(item);
    action->setData(var);
    action->setCheckable(true);
    return action;
}

void DDropdown::setStatus(QString status)
{
    Q_D(DDropdown);
    d->status = status;
}

void DDropdown::enterEvent(QEvent *event)
{
    setStatus("hover");
    QFrame::enterEvent(event);

    this->style()->unpolish(this);
    this->style()->polish(this);
    update();
}

void DDropdown::leaveEvent(QEvent *event)
{
    setStatus("");
    QFrame::leaveEvent(event);

    this->style()->unpolish(this);
    this->style()->polish(this);
    update();
}

void DDropdown::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit requestContextMenu();
    }
    QFrame::mouseReleaseEvent(event);
}
