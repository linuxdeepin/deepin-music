// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DStackedWidget>

DWIDGET_USE_NAMESPACE

class MusicStatckedWidget : public DStackedWidget
{
    Q_OBJECT
public:
    //
    static constexpr int AnimationDelay = 400;
    static constexpr int InputAnimationDelay = 250;
public:
    explicit MusicStatckedWidget(QWidget *parent = Q_NULLPTR);
    // 向上移动动画
    void animationToUp();
    // 向下移动动画
    void animationToDown();

    // 导入向下移动动画
    void animationImportToDown(const QSize &size);
    // 导入向左移动动画
    void animationImportToLeft(const QSize &size);
    // 虚拟键盘引起的向上移动动画
    void animationToUpByInput();
    // 虚拟键盘引起的向下移动动画
    void animationToDownByInput();
};

