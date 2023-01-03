// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class ShortcutPrivate;
class Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit Shortcut(QObject *parent = nullptr);

    Q_INVOKABLE void show(const int &x,const int &y);

private:
    ShortcutPrivate *m_data;
};

#endif // SHORTCUT_H
