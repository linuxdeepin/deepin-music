// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
