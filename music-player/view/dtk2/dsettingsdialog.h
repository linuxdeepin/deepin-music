/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QPointer>
#include <QScopedPointer>

#include <DSettings>

#include "dabstractdialog.h"

namespace dm { //DMUSIC_BEGIN_NAMESPACE

class DSettingsDialogPrivate;
class DSettingsDialog : public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT
public:
    DSettingsDialog(QWidget *parent = 0);
    ~DSettingsDialog();

public Q_SLOTS:
    void updateSettings(DTK_CORE_NAMESPACE::DSettings *settings);

private:
    QScopedPointer<DSettingsDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettingsDialog)
};

}//DMUSIC_END_NAMESPACE
