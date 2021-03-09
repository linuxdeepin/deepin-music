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

#include <QObject>
#include <QTimer>
#include <QScopedPointer>

//class HoverFilter : public QObject
//{
//    Q_OBJECT
//public:
//    explicit HoverFilter(QObject *parent = nullptr);
//    bool eventFilter(QObject *obj, QEvent *event);
//};

//class HoverShadowFilter : public QObject
//{
//    Q_OBJECT
//public:
//    explicit HoverShadowFilter(QObject *parent = nullptr);
//    bool eventFilter(QObject *obj, QEvent *event);
//};


class HintFilter: public QObject
{
    Q_OBJECT
public:
    explicit HintFilter(QObject *parent = nullptr);
    ~HintFilter();

    //void hideAll();
    bool eventFilter(QObject *obj, QEvent *event);
    void showHitsFor(QWidget *w, QWidget *hint);

private:
    void showHint(QWidget *hint);

private:
    QTimer  *m_delayShowTimer = nullptr;

    QWidget *m_parentWidget = nullptr;
    QWidget *m_hintWidget = nullptr;
};
