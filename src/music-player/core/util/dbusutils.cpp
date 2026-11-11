// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusutils.h"
#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>

// 注意：原实现持有一把进程级全局静态 QMutex，把所有 readDBusProperty 调用串行化。
// 这会让“一条调用阻塞”传染成“所有线程同时进入 futex_wait”——SDL 实时音频回调里
// 一旦被 DBus 阻塞，主线程 / CdaThread 想做任何 DBus 调用都会卡在同一把锁上。
// Qt DBus 库自身已经线程安全（QDBusInterface 可被多线程并发使用），不需要外层串行锁。
// 因此移除该全局 mutex。详见 sdlplayer.cpp 的回调分析。

DBusUtils::DBusUtils()
{

}

QVariant DBusUtils::readDBusProperty(const QString &service, const QString &path, const QString &interface, const char *propert, QDBusConnection connection)
{
    // 创建QDBusInterface接口。
    // 不再使用全局 mutex 串行化：QDBusInterface / property() 本身线程安全,
    // 串行锁只会把单线程的 DBus 阻塞放大成“进程所有线程同时被锁住”。
    QDBusInterface ainterface(service,
                              path,
                              interface,
                              connection);
    if (!ainterface.isValid()) {
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        QVariant v(0) ;
        return  v;
    }
    //调用远程的value方法
    return ainterface.property(propert);
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
