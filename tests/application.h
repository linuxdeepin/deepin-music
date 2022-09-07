// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <DApplication>

class MainFrame;

DWIDGET_USE_NAMESPACE

class Application : public QObject
{
    Q_OBJECT

public:
    Application(/*int &argc, char **argv*/);
    ~Application();
    static Application *getInstance();

public :
    //test
    void setMainWindow(MainFrame *window);
    MainFrame *getMainWindow();

public:
    MainFrame *m_mainwindow = nullptr;
    static Application *dApp1;
};

#endif  // APPLICATION_H_
