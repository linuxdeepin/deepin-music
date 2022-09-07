// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DFrame>

DWIDGET_USE_NAMESPACE

class WaveformScale : public DFrame
{
    Q_OBJECT

    static const int WAVE_WIDTH;
    static const int WAVE_TEXTHEIGHT;
    static const int WAVE_TEXTDEFAULTWIDTH;
    static const int WAVE_OFFSET;

public:
    explicit WaveformScale(QWidget *parent = nullptr);

    void setValue(qint64 value);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QString      timeStr;
    qint64       curValue = 0;
};

