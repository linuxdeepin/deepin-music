// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util/singleton.h>
#include <DSettings>

class MusicSettings : public QObject, public DMusic::DSingleton<MusicSettings>
{
    Q_OBJECT
public:
    explicit MusicSettings(QObject *parent = nullptr);
    ~MusicSettings();

    static void init();
    static void release();
    static QPointer<Dtk::Core::DSettings> settings();

    static void sync();
    static QVariant value(const QString &key);
    static void setOption(const QString &key, const QVariant &value);

private:
    static QPointer<Dtk::Core::DSettings> m_settings;
    friend class DMusic::DSingleton<MusicSettings>;
};
