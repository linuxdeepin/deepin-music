// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "label.h"

class CoverPrivate;
class Cover : public Label
{
    Q_OBJECT
public:
    explicit Cover(QWidget *parent = Q_NULLPTR);
    ~Cover() override;

public slots:
    void setCoverPixmap(const QPixmap &pixmap);

protected:
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:

    int m_radius = 8;
    QColor  m_borderColor;
    QColor  m_shadowColor;
    QPixmap m_Background;

    QMarginsF outterMargins;
};

