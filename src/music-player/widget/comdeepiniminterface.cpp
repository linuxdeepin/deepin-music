// Copyright (C) 2017 The Qt Company Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "comdeepiniminterface.h"

/*
 * Implementation of interface class ComDeepinImInterface
 */

ComDeepinImInterface::ComDeepinImInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ComDeepinImInterface::~ComDeepinImInterface()
{
}

