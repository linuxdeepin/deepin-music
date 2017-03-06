/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "titeledit.h"

#include <QDebug>

class TitelEditPrivate
{
public:
    TitelEditPrivate(TitelEdit *parent) : q_ptr(parent) {}

    TitelEdit *q_ptr;
    Q_DECLARE_PUBLIC(TitelEdit)
};

TitelEdit::TitelEdit(QWidget* parent) :
    QLineEdit(parent), d_ptr(new TitelEditPrivate(this))
{
}

TitelEdit::~TitelEdit()
{

}
