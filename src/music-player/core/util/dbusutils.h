// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
