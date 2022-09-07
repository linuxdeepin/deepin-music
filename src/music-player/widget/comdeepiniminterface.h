// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMINTERFACE_H
#define IMINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.im
 */
class ComDeepinImInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.im"; }

public:
    ComDeepinImInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~ComDeepinImInterface();

    Q_PROPERTY(QRect geometry READ geometry)
    inline QRect geometry() const
    { return qvariant_cast< QRect >(property("geometry")); }

    Q_PROPERTY(bool imActive READ imActive WRITE setImActive)
    inline bool imActive() const
    { return qvariant_cast< bool >(property("imActive")); }
    inline void setImActive(bool value)
    { setProperty("imActive", QVariant::fromValue(value)); }

    Q_PROPERTY(bool imSignalLock READ imSignalLock WRITE setImSignalLock)
    inline bool imSignalLock() const
    { return qvariant_cast< bool >(property("imSignalLock")); }
    inline void setImSignalLock(bool value)
    { setProperty("imSignalLock", QVariant::fromValue(value)); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> setKeyboardHeight(int h)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(h);
        return asyncCallWithArgumentList(QStringLiteral("setKeyboardHeight"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void geometryChanged(const QRect &rect);
    void imActiveChanged(bool is);
    void imSignalLockChanged(bool is);
};

namespace com {
  namespace deepin {
    typedef ::ComDeepinImInterface im;
  }
}
#endif
