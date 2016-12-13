/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "pushbutton.h"

#include <QDebug>

PushButton::PushButton(QWidget *parent) : QPushButton(parent)
{

}

void PushButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);
    emit entered();
}
