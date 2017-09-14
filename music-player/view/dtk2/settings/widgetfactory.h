/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "dtkwidget_global.h"

DCORE_BEGIN_NAMESPACE
class DSettingsOption;
DCORE_END_NAMESPACE

namespace dm { //DMUSIC_BEGIN_NAMESPACE

typedef QWidget *(*WidgetCreateHandle)(QObject *);

class WidgetFactoryPrivate;
class LIBDTKWIDGETSHARED_EXPORT WidgetFactory : public QObject
{
    Q_OBJECT
public:
    explicit WidgetFactory(QObject *parent = 0);
    ~WidgetFactory();

    void registerWidget(const QString &viewType, WidgetCreateHandle handle);
    QWidget *createWidget(QPointer<DTK_CORE_NAMESPACE::DSettingsOption> settings);
Q_SIGNALS:

public Q_SLOTS:
    static QWidget *createTwoColumHandle(DTK_CORE_NAMESPACE::DSettingsOption *option, QWidget *rightWidget);

private:
    QScopedPointer<WidgetFactoryPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), WidgetFactory)
};

}//DMUSIC_END_NAMESPACE
