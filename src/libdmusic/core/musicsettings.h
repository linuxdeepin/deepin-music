// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
