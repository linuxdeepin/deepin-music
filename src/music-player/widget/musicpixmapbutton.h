// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DPushButton>

DWIDGET_USE_NAMESPACE

class MusicPixmapButton : public DPushButton
{
    Q_OBJECT
public:
    explicit MusicPixmapButton(QWidget *parent = Q_NULLPTR);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

};
