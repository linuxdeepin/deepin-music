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
