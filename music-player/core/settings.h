/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <util/singleton.h>
#include <settings.h>

class AppSettings : public QObject, public DMusic::DSingleton<AppSettings>
{
    Q_OBJECT
public:
    explicit AppSettings(QObject *parent = 0);

    void init();
    QPointer<Dtk::Settings> settings() const;

    void sync();
    QVariant value(const QString &key) const;
    void setOption(const QString &key, const QVariant &value);

private:
    QPointer<Dtk::Settings> m_settings;
    friend class DMusic::DSingleton<AppSettings>;
};
