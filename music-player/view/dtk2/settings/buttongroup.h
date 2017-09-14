/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

class ButtonGroupPrivate;
class ButtonGroup : public QFrame
{
    Q_OBJECT
public:
    explicit ButtonGroup(QWidget *parent = 0);
    ~ButtonGroup();

Q_SIGNALS:
    void buttonChecked(int id);

public Q_SLOTS:
    void setCheckedButton(int id);
    void setButtons(const QStringList &texts);

private:
    QScopedPointer<ButtonGroupPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ButtonGroup)
};

