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

#include "dbusutils.h"
#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QMutex>

static QMutex mutex;

DBusUtils::DBusUtils()
{

}

QVariant DBusUtils::readDBusProperty(const QString &service, const QString &path, const QString &interface, const char *propert, QDBusConnection connection)
{
    // 创建QDBusInterface接口
    mutex.lock();
    QDBusInterface ainterface(service,
                              path,
                              interface,
                              connection);
    if (!ainterface.isValid()) {
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        //cause dead lock if no unlock here,
        mutex.unlock();
        QVariant v(0) ;
        return  v;
    }
    //调用远程的value方法
    QVariant v = ainterface.property(propert);
    mutex.unlock();
    return  v;
}
//QVariant DBusUtils::readDBusMethod(const QString &service, const QString &path, const QString &interface, const char *method)
//{
//    // 创建QDBusInterface接口
//    QDBusInterface ainterface(service, path,
//                              interface,
//                              QDBusConnection::sessionBus());
//    if (!ainterface.isValid()) {
//        qDebug() <<  "error:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
//        QVariant v(0) ;
//        return  v;
//    }
//    //调用远程的value方法
//    QDBusReply<QDBusVariant> reply = ainterface.call(method);
//    if (reply.isValid()) {
////        return reply.value();
//        QVariant v(0) ;
//        return  v;
//    } else {
//        qDebug() << "error1:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
//        QVariant v(0) ;
//        return  v;
//    }
//}
