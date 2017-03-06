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
#include <QLineEdit>

class TitelEditPrivate;
class TitelEdit: public QLineEdit
{
public:
    TitelEdit(QWidget *parent=nullptr);
    ~TitelEdit();

private:
    QScopedPointer<TitelEditPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), TitelEdit)
};

