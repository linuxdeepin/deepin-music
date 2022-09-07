// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DIconButton>

DWIDGET_USE_NAMESPACE

class ControlIconButton : public DIconButton
{
    Q_OBJECT
public:
    explicit ControlIconButton(QWidget *parent = Q_NULLPTR);
    ~ControlIconButton() override;

signals:
    // 鼠标是否在按钮上 true为在按钮上
    void mouseIn(bool);

protected:
    virtual void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    virtual void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
private:
};
