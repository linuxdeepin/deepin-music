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

#include <QScopedPointer>
#include <QFrame>

#include "dtkwidget_global.h"

DCORE_BEGIN_NAMESPACE
class DSettings;
DCORE_END_NAMESPACE

namespace dm { //DMUSIC_BEGIN_NAMESPACE

class NavigationPrivate;
class LIBDTKWIDGETSHARED_EXPORT Navigation : public QFrame
{
    Q_OBJECT
public:
    explicit Navigation(QWidget *parent = 0);
    ~Navigation();

Q_SIGNALS:
    void selectedGroup(const QString &key);

public Q_SLOTS:
    void onSelectGroup(const QString &key);
    void updateSettings(QPointer<DTK_CORE_NAMESPACE::DSettings> settings);

private:
    QScopedPointer<NavigationPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Navigation)
};

}//DMUSIC_END_NAMESPACE
