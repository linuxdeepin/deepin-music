/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QtGlobal>
#include <QObject>
#include <QScopedPointer>
#include <QPointer>

#include "../../core/util/singleton.h"

class QWidget;
class ThemeManagerPrivate;
class ThemeManager : public QObject, public Singleton<ThemeManager>
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = 0);
    ~ThemeManager();

    void regisetrWidget(QPointer<QWidget> widget, QStringList propertys = QStringList());

public:
    QString theme() const;

public slots:
    void setTheme(const QString theme);
    void setPrefix(const QString prefix);
    // TODO: move to ThemeManagerPrivate
    void updateQss();

signals:
    void themeChanged(QString theme);

private:

    friend class Singleton<ThemeManager>;
    QScopedPointer<ThemeManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ThemeManager)
};

