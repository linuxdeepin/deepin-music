// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicsettings.h"

#include <DSettingsOption>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <qsettingbackend.h>
#include <QImage>
#include <QCoreApplication>
#include <QDebug>

#include "global.h"

MusicSettings::MusicSettings(QObject *parent) :
    QObject(parent)
{
    init();
}

MusicSettings::~MusicSettings()
{
    if (m_settings != nullptr) {
        m_settings->sync();
        qApp->processEvents();
        delete m_settings;
        m_settings = nullptr;
    }
}

void MusicSettings::init()
{
    if (!QFile::exists(DmGlobal::cachePath() + "/images/default_cover.png")) {
        QDir imageDir(DmGlobal::cachePath() + "/images");
        if (!imageDir.exists()) {
            bool isExists = imageDir.cdUp();
            isExists &= imageDir.mkdir("images");
            isExists &= imageDir.cd("images");
        }
        QImage defaultImg(":/data/default_cover.png");
        defaultImg = defaultImg.scaled(430, 430, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        defaultImg.save(DmGlobal::cachePath() + "/images/default_cover.png");
    }
    m_settings = Dtk::Core::DSettings::fromJsonFile(":/data/music-settings.json");
    auto configFilepath = DmGlobal::configPath() + "/config.ini";
    auto backend = new Dtk::Core::QSettingBackend(configFilepath, m_settings);
    m_settings->setBackend(backend);
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

void MusicSettings::reset()
{
    if (!m_settings.isNull())
        m_settings->reset();
}

QVariant MusicSettings::value(const QString &key)
{
    return m_settings.isNull() ? QVariant() : m_settings->value(key);
}

void MusicSettings::setValue(const QString &key, const QVariant &value)
{
    if (!m_settings.isNull() && m_settings->value(key) != value)
        m_settings->setOption(key, value);
}
