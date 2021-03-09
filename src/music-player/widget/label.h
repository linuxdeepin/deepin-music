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
