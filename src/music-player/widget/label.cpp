// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DGuiApplicationHelper>
#include <QDebug>

#include "label.h"

Label::Label(QWidget *parent)
    : DLabel("", parent)
{
}

Label::Label(const QString &text, QWidget *parent):
    DLabel(text, parent)
{
}

Label::~Label()
{
}

void Label::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    // 控件大小改变
    emit signalSizeChange();
}

void Label::mousePressEvent(QMouseEvent * /*event*/)
{
    m_selected = true;
    Q_EMIT clicked(false);
}
