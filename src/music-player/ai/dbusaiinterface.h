// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSAIINTERFACE_H
#define DBUSAIINTERFACE_H

#include <QObject>
#include <QDBusInterface>

class DBusAIInterface : public QObject
{
    Q_OBJECT
public:
    explicit DBusAIInterface(QString &aiServicePath, QObject *parent = nullptr);

signals:

public slots:
    void slotChatTextReceived(QString, QString);
    void slotError(QString, qint32, QString);

private:
    QDBusInterface *mp_interface = nullptr;
};

#endif // DBUSAIINTERFACE_H
