/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QScopedPointer>

class DSettingsPrivate;
class DSettings : public QObject
{
    Q_OBJECT
public:
    explicit DSettings(QObject *parent = 0);
    ~DSettings();

    void loadDefault(const QString &jsonFileName);
    void sync();
    void reset();

    QVariant option(const QString &key);
signals:
    void optionChange(const QString &key, const QVariant &value);

public slots:
    void setOption(const QString &key, const QVariant &value);

private:
    QScopedPointer<DSettingsPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettings)
};

