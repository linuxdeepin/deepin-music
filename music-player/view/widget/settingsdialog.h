/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class SettingsDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = 0);

signals:

public slots:
};

#endif // SETTINGSDIALOG_H
