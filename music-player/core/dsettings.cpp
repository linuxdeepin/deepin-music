/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dsettings.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <QSettings>

#include "../../musicapp.h"

class DSettingsPrivate
{
public:
    DSettingsPrivate(DSettings *parent) : q_ptr(parent)
    {
        settings = new QSettings(MusicApp::configPath() + "/config.ini", QSettings::IniFormat);
    }

    QSettings           *settings       = nullptr;

    DSettings *q_ptr;
    Q_DECLARE_PUBLIC(DSettings)
};

DSettings::DSettings(QObject *parent) : QObject(parent), d_ptr(new DSettingsPrivate(this))
{
    loadDefault(":/data/deepin-music-settings.json");
}

DSettings::~DSettings()
{

}

void DSettings::loadDefault(const QString &jsonFileName)
{
    Q_D(DSettings);

    QFile jsonFile(jsonFileName);
    jsonFile.open(QIODevice::ReadOnly);
    auto jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
    auto mainGroups = jsonDoc.object().value("main_groups");

    for (auto groupJson : mainGroups.toArray()) {
        auto group = groupJson.toObject();
        auto groupTitle = group.value("key").toString();
        for (auto subGroupJson : group.value("groups").toArray()) {
            auto subGroup = subGroupJson.toObject();
            auto subGroupTitle = subGroup.value("key").toString();
            for (auto optionJson :  subGroup.value("options").toArray()) {
                auto option = optionJson.toObject();
                auto optionKey = option.value("key").toString();
                d->settings->beginGroup(QString("%1.%2.%3").arg(groupTitle).arg(subGroupTitle).arg(optionKey));
                d->settings->setValue("default", option.value("default").toVariant());
                d->settings->endGroup();
                qDebug() << option.value("default") << QString("%1.%2.%3").arg(groupTitle).arg(subGroupTitle).arg(optionKey);
            }
        }
    }
    d->settings->sync();
}

void DSettings::sync()
{
    Q_D(DSettings);
    d->settings->sync();
}

void DSettings::reset()
{
    Q_D(DSettings);
    for (auto optKey : d->settings->childGroups()) {
        d->settings->beginGroup(optKey);
        auto optDefault = d->settings->value("default");
        d->settings->setValue("value", optDefault);
        emit optionChange(optKey, optDefault);
        d->settings->endGroup();
    }
}

QVariant DSettings::option(const QString &key)
{
    Q_D(DSettings);
    d->settings->beginGroup(key);

    QVariant value;
    if (d->settings->contains("value")) {
        value = d->settings->value("value");
    } else {
        value = d->settings->value("default");
    }
    d->settings->endGroup();
    return value;
}

void DSettings::setOption(const QString &key, const QVariant &value)
{
    Q_D(DSettings);
    d->settings->beginGroup(key);
//    qDebug() << key;
    d->settings->setValue("value", value);
    d->settings->endGroup();

    emit optionChange(key, value);
}
