/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QScopedPointer>
#include <QPointer>
#include <QFrame>

#include "dtkwidget_global.h"

DCORE_BEGIN_NAMESPACE
class DSettings;
DCORE_END_NAMESPACE

namespace dm { //DMUSIC_BEGIN_NAMESPACE

class WidgetFactory;
class ContentPrivate;
class LIBDTKWIDGETSHARED_EXPORT Content : public QFrame
{
    Q_OBJECT
public:
    explicit Content(QWidget *parent = 0);
    ~Content();

    WidgetFactory* widgetFactory();
Q_SIGNALS:
    void scrollToGroup(const QString &key);

public Q_SLOTS:
    void onScrollToGroup(const QString &key);
    void updateSettings(QPointer<DTK_CORE_NAMESPACE::DSettings> settings);

private:
    QScopedPointer<ContentPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Content)
};

}//DMUSIC_END_NAMESPACE
