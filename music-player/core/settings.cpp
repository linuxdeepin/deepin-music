/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#include "settings.h"

#include <DSettingsOption>

#include <QFile>
#include <QFileInfo>
#include <qsettingbackend.h>
#include "util/global.h"
#include "util/threadpool.h"

#include <QDebug>

AppSettings::AppSettings(QObject *parent) :
    QObject(parent)
{

}

void AppSettings::init()
{
    m_settings = Dtk::Core::DSettings::fromJsonFile(":/data/deepin-music-settings.json");
    auto configFilepath = Global::configPath() + "/config.ini";
    auto backend = new Dtk::Core::QSettingBackend(configFilepath);
    m_settings->setBackend(backend);
//    ThreadPool::instance()->manager(backend->thread());
}

QPointer<Dtk::Core::DSettings> AppSettings::settings() const
{
    return m_settings;
}

void AppSettings::sync()
{
    m_settings->sync();
}

QVariant AppSettings::value(const QString &key) const
{
    return m_settings->value(key);
}

void AppSettings::setOption(const QString &key, const QVariant &value)
{
    qDebug() << "setOption" << key << value;
    m_settings->setOption(key, value);
}
