/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#ifndef DBUSUTILS_H
#define DBUSUTILS_H
#include <QVariant>
#include <QDBusConnection>

class DBusUtils
{
public:
    DBusUtils();
    static QVariant readDBusProperty(const QString &service, const QString &path, const QString &interface = QString(), const char *propert = "", QDBusConnection connection = QDBusConnection::sessionBus());
//    static QVariant readDBusMethod(const QString &service, const QString &path, const QString &interface, const char *method);
};

#endif // DBUSUTILS_H
