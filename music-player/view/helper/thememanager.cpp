/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "thememanager.h"

#include <QDebug>
#include <QFile>
#include <QMetaProperty>
#include <QStyleFactory>
#include <QStyle>
#include <QWidget>
#include <QApplication>
#include <dthememanager.h>

class ThemeManagerPrivate
{
public:
    ThemeManagerPrivate(ThemeManager *parent) : q_ptr(parent) {}

    QString getQssForWidget(QString theme, QString className);

    QString                     prefix      = ":";
    QString                     activeTheme;
    QList<QPointer<QWidget> >   widgetList;

    ThemeManager *q_ptr;
    Q_DECLARE_PUBLIC(ThemeManager)
};

QString ThemeManagerPrivate::getQssForWidget(QString theme, QString className)
{
    QString qss;

    if (theme.isEmpty()) {
        theme = activeTheme;
    }
    auto filename = QString("%1/%2/%3.theme").arg(prefix).arg(theme).arg(className);
    QFile themeFile(filename);

//    qDebug() << "load: " <<filename;
    if (themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qss = themeFile.readAll();
        themeFile.close();
    } else {
        qWarning() << "load theme file faild!" << filename;
    }

    return qss;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent), d_ptr(new ThemeManagerPrivate(this))
{

}

ThemeManager::~ThemeManager()
{

}

void ThemeManager::regisetrWidget(QPointer<QWidget> widget, QStringList propertys)
{
    Q_D(ThemeManager);
    d->widgetList.push_back(widget);

    // set theme
    auto meta = widget->metaObject();
    auto qssFilename = widget->property("_d_QSSFilename").toString();
    auto themename = widget->property("_d_QSSThemename").toString();

    if (qssFilename.isEmpty()) {
        qssFilename = widget->objectName();
    }

    if (qssFilename.isEmpty()) {
        qssFilename = meta->className();
    }

//    qDebug() << qssFilename;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->setStyleSheet(d->getQssForWidget(themename, qssFilename));

//    qDebug() << widget;
    connect(this, &ThemeManager::themeChanged, this, [ = ](QString theme) {
        if (widget.isNull()) {
            return ;
        }

        widget->setStyleSheet(d->getQssForWidget(theme, qssFilename));
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    });

    for (auto property : propertys) {
        auto propertyIndex = meta->indexOfProperty(property.toLatin1().data());
//        qDebug() << propertyIndex << property << meta->propertyCount();
//        for (int i =0; i < meta->propertyCount(); ++i){
//            qDebug() << meta->property(i).name();
//        }
        if (-1 == propertyIndex) {
            continue;
        }

//        qDebug() << "connect " << meta->property(propertyIndex).notifySignal().name();
        connect(widget, meta->property(propertyIndex).notifySignal(),
                this, metaObject()->method(metaObject()->indexOfMethod("updateQss()")));
    }
}

QString ThemeManager::theme() const
{
    Q_D(const ThemeManager);
    return d->activeTheme;
}

void ThemeManager::setTheme(const QString theme)
{
    Q_D(ThemeManager);

    if (d->activeTheme != theme) {
        Dtk::Widget::DThemeManager::instance()->setTheme(theme);
        d->activeTheme = theme;

        // FIXME: check style plugin is load. use what prifix ?
        if (QStyle *style = QStyleFactory::create("d" + theme)) {
            qApp->setStyle(style);
        }

        emit themeChanged(theme);
    }
}

void ThemeManager::setPrefix(const QString prefix)
{
    Q_D(ThemeManager);
    d->prefix = prefix;
}

void ThemeManager::updateQss()
{
    QWidget *w = qobject_cast<QWidget *>(sender());
//    qDebug() << w;
    if (w) {
        w->setStyleSheet(w->styleSheet());
        w->style()->unpolish(w);
        w->style()->polish(w);
        w->update();
    }
}
