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

#include "thememanager.h"

#include <QDebug>
#include <QFile>
#include <QDynamicPropertyChangeEvent>
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
    QMap<QPointer<QWidget>, QStringList >   watchedWidgetPropertys;

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

    qDebug() << widget;
//    qDebug() << qssFilename;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->setStyleSheet(widget->styleSheet() + d->getQssForWidget(themename, qssFilename));

//    qDebug() << widget;
    connect(this, &ThemeManager::themeChanged, this, [ = ](QString theme) {
        if (widget.isNull()) {
            return ;
        }

        widget->setStyleSheet(d->getQssForWidget(theme, qssFilename));
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    });

    if (!propertys.isEmpty()) {
        widget->installEventFilter(this);
        d->watchedWidgetPropertys.insert(widget, propertys);
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

bool ThemeManager::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(ThemeManager);
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto propEvent = reinterpret_cast<QDynamicPropertyChangeEvent *>(event);
        auto widget = qobject_cast<QWidget *>(watched);
        if (propEvent && widget) {
            auto propName = QString::fromLatin1(propEvent->propertyName().data());
            if (d->watchedWidgetPropertys.contains(widget)) {
                auto props = d->watchedWidgetPropertys.value(widget);
                if (props.contains(propName)) {
                    if (widget) {
                        widget->setStyleSheet(widget->styleSheet());
                        widget->style()->unpolish(widget);
                        widget->style()->polish(widget);
                        widget->update();
                    }
                }
            }
        }
    }
    return QObject::eventFilter(watched, event);
}
