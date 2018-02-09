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

#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <util/singleton.h>

class QWidget;
class ThemeManagerPrivate;
class ThemeManager : public QObject, public DMusic::DSingleton<ThemeManager>
{
    Q_OBJECT
public:
    ~ThemeManager();

    void regisetrWidget(QPointer<QWidget> widget, QStringList propertys = QStringList());
    QString theme() const;

public Q_SLOTS:
    void setTheme(const QString theme);
    void setPrefix(const QString prefix);

Q_SIGNALS:
    void themeChanged(QString theme);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    explicit ThemeManager(QObject *parent = 0);
    friend class DMusic::DSingleton<ThemeManager>;

    QScopedPointer<ThemeManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ThemeManager)
};

