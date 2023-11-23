// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AI_H
#define AI_H

#include <QObject>
#include <QDBusInterface>

#include "util/singleton.h"

class UosAIInterface : public QObject, public DMusic::DSingleton<UosAIInterface>
{
    Q_OBJECT
public:
    explicit UosAIInterface(QObject *parent = nullptr);

    void parseAIFunction(/*QByteArray*/); //解析UOSAI传过来的function参数

    bool isConnected() {return m_isCopilotConnected;};

private:
    void handleAICall(QString &, QMap<QString, QString> &); //根据不同的参数进行不同的操作

signals:

public slots:


private:
    bool m_isCopilotConnected = false;
    QDBusInterface *m_aiSessionBus = nullptr;

};

#endif // AI_H
