// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_H
#define SERVICE_H
#include <QString>
#include "reply.h"

/* 服务优先级：
 * SeriesIgnore表示该service可忽略
 * ServicePriorityPatch表示服务修复代码，直接覆盖旧的服务
 */
enum ServicePriority {
    ServicePriorityIgnore   = -10,
    ServicePriorityLow      = 0,
    SerivcePriorityNormal   = 10,
    SerivcePriorityHigh     = 50,
    SerivcePriorityPatch    = 127
};




class IService : public QObject //  IReplyHandler
{
    Q_OBJECT
public:
    virtual ~IService() {}
    virtual QString serviceName() = 0;
    virtual int servicePriority() = 0;
    virtual bool canHandle(const QString &s) = 0;
    virtual IRet service(const QString &semantic) = 0;
    virtual Reply &getServiceReply() = 0;
signals:
    void signalSendShowMessage(Reply reply);
};


#endif // SERVICE_H
