/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "label.h"

Label::Label(QWidget *parent)
    : Label("", parent)
{
}

Label::Label(const QString &text, QWidget *parent):
    QLabel (text, parent)
{
}

Label::~Label()
{
}

void Label::mousePressEvent(QMouseEvent * /*event*/)
{
    emit clicked(false);
}
