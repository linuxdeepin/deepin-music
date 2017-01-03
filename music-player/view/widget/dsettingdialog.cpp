/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dsettingdialog.h"

#include <QScrollArea>

class DSettingDialogPrivate
{
public:
    DSettingDialogPrivate(DSettingDialog *parent) : q_ptr(parent) {}


    DSettingDialog *q_ptr;
    Q_DECLARE_PUBLIC(DSettingDialog)
};

DSettingDialog::DSettingDialog(QWidget *parent) : ThinWindow(parent), d_ptr(new DSettingDialogPrivate(this))
{

}

DSettingDialog::~DSettingDialog()
{

}
