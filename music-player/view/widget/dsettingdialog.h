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
#include "thinwindow.h"

class DSettingDialogPrivate;
class DSettingDialog : public ThinWindow
{
    Q_OBJECT
public:
    explicit DSettingDialog(QWidget *parent = 0);
    ~DSettingDialog();

signals:

public slots:

private:
    QScopedPointer<DSettingDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettingDialog)
};

