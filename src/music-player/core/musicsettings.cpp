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

#include "musicsettings.h"

#include <DSettingsOption>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <qsettingbackend.h>
#include "util/global.h"

#include <QDebug>

QPointer<Dtk::Core::DSettings> MusicSettings::m_settings = nullptr;

MusicSettings::MusicSettings(QObject *parent) :
    QObject(parent)
{

}

MusicSettings::~MusicSettings()
{
    release();
}

void MusicSettings::init()
{
    m_settings = Dtk::Core::DSettings::fromJsonFile(":/data/deepin-music-settings.json");
    auto configFilepath = Global::configPath() + "/config.ini";
    auto backend = new Dtk::Core::QSettingBackend(configFilepath, m_settings);
    m_settings->setBackend(backend);
}

void MusicSettings::release()
{
    if (!m_settings.isNull()) {
        m_settings->sync();
        delete m_settings;
        m_settings = nullptr;
    }
}

QPointer<Dtk::Core::DSettings> MusicSettings::settings()
{
    return m_settings;
}

void MusicSettings::sync()
{
    if (!m_settings.isNull())
        m_settings->sync();
}

QVariant MusicSettings::value(const QString &key)
{
    return m_settings.isNull() ? QVariant() : m_settings->value(key);
}

void MusicSettings::setOption(const QString &key, const QVariant &value)
{
    if (!m_settings.isNull())
        m_settings->setOption(key, value);
}
