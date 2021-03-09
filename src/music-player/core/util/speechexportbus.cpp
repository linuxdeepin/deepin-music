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

#include "speechexportbus.h"
#include <QHash>
#include <QPair>
#include <QVector>
#include <QVariant>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMetaMethod>
#include <QGenericArgument>
#include <QDebug>
#include <DObject>


SpeechExportBus::SpeechExportBus(QObject *parent) : QObject(parent), m_pExportInterface(new ExporteDBusInterface())
{
    QDBusConnection::sessionBus().registerObject("/org/mpris/speech", m_pExportInterface, QDBusConnection::RegisterOption::ExportAllSlots);
}

SpeechExportBus::~SpeechExportBus()
{
    QDBusConnection::sessionBus().unregisterObject("/org/mpris/speech");
}

void SpeechExportBus::registerAction(const QString &action, const QString &description, const std::function<QVariant(QString)> handler)
{
    m_pExportInterface->actions[action] = {handler, description};
}

void SpeechExportBus::registerQStringListAction(const QString &action, const QString &description, const std::function<QVariant(QStringList)> handler)
{
    m_pExportInterface->strListactions[action] = {handler, description};
}

QStringList SpeechExportBus::list()
{
    return m_pExportInterface->actions.keys();
}

QDBusVariant ExporteDBusInterface::invoke(QString action, QString parameters)
{
    qDebug() << __FUNCTION__ << action;
    QDBusVariant ret;
    if (!actions.contains(action)) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QString("Action \"%1\" is not registered").arg(action));
    } else {
        auto func = actions.value(action).first;
        if (func)
            ret.setVariant(func(parameters));
        else
            ret.setVariant(QVariant());
    }
    return ret;
}

QDBusVariant ExporteDBusInterface::invokeStrlist(QString action, QStringList parameters)
{
    qDebug() << __FUNCTION__ << action;
    QDBusVariant ret;
    if (!strListactions.contains(action)) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QString("strlistAction \"%1\" is not registered").arg(action));
    } else {
        auto func = strListactions.value(action).first;
        if (func)
            ret.setVariant(func(parameters));
        else
            ret.setVariant(QVariant());
    }
    return ret;
}
