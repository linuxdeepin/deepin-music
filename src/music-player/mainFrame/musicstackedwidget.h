/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <DStackedWidget>

DWIDGET_USE_NAMESPACE

class MusicStatckedWidget : public DStackedWidget
{
    Q_OBJECT
public:
    //
    static constexpr int AnimationDelay = 400;
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
};

