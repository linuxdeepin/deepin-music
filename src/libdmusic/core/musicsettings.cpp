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
#include "util/log.h"

MusicSettings::MusicSettings(QObject *parent) :
    QObject(parent)
{
    qCDebug(dmMusic) << "Initializing MusicSettings";
    init();
}

MusicSettings::~MusicSettings()
{
    qCDebug(dmMusic) << "Destroying MusicSettings";
    if (m_settings != nullptr) {
        qCDebug(dmMusic) << "Syncing and cleaning up settings";
        m_settings->sync();
        qApp->processEvents();
        delete m_settings;
        m_settings = nullptr;
        qCDebug(dmMusic) << "Settings cleanup completed";
    }
}

void MusicSettings::init()
{
    qCDebug(dmMusic) << "Initializing MusicSettings";
    auto coverPath = DmGlobal::cachePath() + "/images/default_cover.png";
    if (!QFile::exists(coverPath)) {
        QDir imageDir(DmGlobal::cachePath() + "/images");
        if (!imageDir.exists()) {
            qCDebug(dmMusic) << "Creating images directory";
            bool isExists = imageDir.cdUp();
            isExists &= imageDir.mkdir("images");
            isExists &= imageDir.cd("images");
            if (!isExists) {
                qCWarning(dmMusic) << "Failed to create images directory";
            }
        }
        qCDebug(dmMusic) << "Creating default cover image";
        QImage defaultImg(":/data/default_cover.png");
        defaultImg = defaultImg.scaled(430, 430, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!defaultImg.save(coverPath)) {
            qCWarning(dmMusic) << "Failed to save default cover image";
        } else {
            qCDebug(dmMusic) << "Successfully saved default cover image to:" << coverPath;
        }
    }
    qCDebug(dmMusic) << "Loading settings from JSON file";
    m_settings = Dtk::Core::DSettings::fromJsonFile(":/data/music-settings.json");
    if (m_settings.isNull()) {
        qCCritical(dmMusic) << "Failed to load settings from JSON file";
        return;
    }
    auto configFilepath = DmGlobal::configPath() + "/config.ini";
    qCDebug(dmMusic) << "Setting config backend to:" << configFilepath;
    auto backend = new Dtk::Core::QSettingBackend(configFilepath, m_settings);
    m_settings->setBackend(backend);
    qCDebug(dmMusic) << "Settings backend initialized successfully";
}

QPointer<Dtk::Core::DSettings> MusicSettings::settings()
{
    qCDebug(dmMusic) << "Getting settings pointer";
    if (m_settings.isNull()) {
        qCWarning(dmMusic) << "Settings pointer is null";
    }
    return m_settings;
}

void MusicSettings::sync()
{
    if (!m_settings.isNull()) {
        qCDebug(dmMusic) << "Syncing settings to disk";
        m_settings->sync();
    } else {
        qCWarning(dmMusic) << "Cannot sync - settings is null";
    }
}

void MusicSettings::reset()
{
    if (!m_settings.isNull()) {
        qCDebug(dmMusic) << "Resetting settings to defaults";
        m_settings->reset();
    } else {
        qCWarning(dmMusic) << "Cannot reset - settings is null";
    }
}

QVariant MusicSettings::value(const QString &key)
{
    if (m_settings.isNull()) {
        qCWarning(dmMusic) << "Cannot get value - settings is null, key:" << key;
        return QVariant();
    }
    qCDebug(dmMusic) << "Getting setting value for key:" << key;
    return m_settings->value(key);
}

void MusicSettings::setValue(const QString &key, const QVariant &value)
{
    if (m_settings.isNull()) {
        qCWarning(dmMusic) << "Cannot set value - settings is null, key:" << key;
        return;
    }
    if (m_settings->value(key) != value) {
        qCDebug(dmMusic) << "Setting value for key:" << key << "value:" << value;
        m_settings->setOption(key, value);
    } else {
        qCDebug(dmMusic) << "Value unchanged for key:" << key << "value:" << value;
    }
}
