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
#include <QPointer>
#include <QScopedPointer>
#include <util/singleton.h>

class QWidget;
class ThemeManagerPrivate;
class ThemeManager : public QObject, public DMusic::DSingleton<ThemeManager>
{
    Q_OBJECT
public:
    ~ThemeManager();

    void regisetrWidget(QPointer<QWidget> widget, QStringList propertys = QStringList());
    QString theme() const;

public slots:
    void setTheme(const QString theme);
    void setPrefix(const QString prefix);
    // TODO: move to ThemeManagerPrivate
    void updateQss();

signals:
    void themeChanged(QString theme);

private:
    explicit ThemeManager(QObject *parent = 0);
    friend class DMusic::DSingleton<ThemeManager>;

    QScopedPointer<ThemeManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ThemeManager)
};

