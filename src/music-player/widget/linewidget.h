// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DWidget>
#include <QStackedWidget>
#include <DHorizontalLine>

DWIDGET_USE_NAMESPACE
class Label;
class LineWidget : public DWidget
{
    Q_OBJECT
public:
    explicit LineWidget(QWidget *parent = nullptr);
    ~LineWidget();

public slots:

signals:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

public:
};
