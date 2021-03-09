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

#ifndef SPEECHEXPORTBUS_H
#define SPEECHEXPORTBUS_H

#include <QObject>
#include <QDBusContext>
#include <QScopedPointer>
#include <QDBusVariant>

class ExporteDBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.speech")
public Q_SLOTS:
    QDBusVariant invoke(QString action, QString parameters);
    QDBusVariant invokeStrlist(QString action, QStringList parameters);
public:
    QHash<QString, QPair<std::function<QVariant(QString)>, QString>> actions;
    QHash<QString, QPair<std::function<QVariant(QStringList)>, QString>> strListactions;
private:
};

class SpeechExportBus : public QObject
{
    Q_OBJECT
public:
    explicit SpeechExportBus(QObject *parent = nullptr);
    ~SpeechExportBus();
    void registerAction(const QString &action, const QString &description, const std::function<QVariant(QString)> handler = nullptr);
    void registerQStringListAction(const QString &action, const QString &description, const std::function<QVariant(QStringList)> handler = nullptr);
    QStringList list();
private:
    ExporteDBusInterface *m_pExportInterface = nullptr;
};

#endif // SPEECHEXPORTBUS_H
