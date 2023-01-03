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

#pragma once

#include <util/singleton.h>
#include <DSettings>

class MusicSettings : public QObject
{
    Q_OBJECT
public:
    explicit MusicSettings(QObject *parent = nullptr);
    ~MusicSettings();

    void init();
    QPointer<Dtk::Core::DSettings> settings();

    void sync();
    void reset();
    QVariant value(const QString &key);
    void setValue(const QString &key, const QVariant &value);

private:
    QPointer<Dtk::Core::DSettings> m_settings = nullptr;
};
