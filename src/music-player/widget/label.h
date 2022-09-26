// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DLabel>

DWIDGET_USE_NAMESPACE

class Label : public DLabel
{
    Q_OBJECT
public:
    explicit Label(QWidget *parent = Q_NULLPTR);
    explicit Label(const QString &text, QWidget *parent = Q_NULLPTR);
    ~Label() override;

signals:
    void clicked(bool);
    // 控件大小改变
    void signalSizeChange();

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    bool m_selected = false;
};
