// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DGuiApplicationHelper>
#include <QDebug>

#include "controliconbutton.h"

ControlIconButton::ControlIconButton(QWidget *parent)
    : DIconButton(parent)
{
}

ControlIconButton::~ControlIconButton()
{
}

void ControlIconButton::enterEvent(QEvent *event)
{
    emit mouseIn(true);
    return DIconButton::enterEvent(event);
}

void ControlIconButton::leaveEvent(QEvent *event)
{
    emit mouseIn(false);
    return DIconButton::leaveEvent(event);
}
