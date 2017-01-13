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

class Settings :public Dtk::Settings, public DMusic::DSingleton<Settings>
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    static QPointer<Dtk::Settings> instance() {
        static auto setting = appSettings();
        return setting;
    }
signals:

public slots:

private:
    static QPointer<Dtk::Settings> appSettings();

    friend class DMusic::DSingleton<Settings>;
};
