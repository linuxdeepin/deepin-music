// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusaiinterface.h"

#include <QDebug>

DBusAIInterface::DBusAIInterface(QString &aiServicePath, QObject *parent) : QObject(parent)
{
    mp_interface = new QDBusInterface("com.deepin.copilot", aiServicePath, "com.deepin.copilot.app", QDBusConnection::sessionBus());

    if (mp_interface->isValid()) {
        connect(mp_interface, SIGNAL(chatTextReceived(QString, QString)), this, SLOT(slotProcessChange(QString, QString)));
        connect(mp_interface, SIGNAL(error(QString, qint32, QString)), this, SLOT(slotProcessEnd(QString, qint32, QString)));
    }

}

void DBusAIInterface::slotChatTextReceived(QString s1, QString s2)
{
    qDebug() << __func__ << "s1: " << s1 << "    s2: " << s2;

}

void DBusAIInterface::slotError(QString s1, qint32 int1, QString s2)
{
    qDebug() << __func__ << "s1: " << s1 << "     int1: " << int1 << "    s2: " << s2;

}

