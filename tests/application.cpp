// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

#define IMAGE_HEIGHT_DEFAULT    100
#define IMAGE_LOAD_DEFAULT    100

Application *Application::dApp1 = nullptr;
Application::Application()
{
}

Application::~Application()
{
}

Application *Application::getInstance()
{
    if (nullptr == dApp1) {
        dApp1 = new Application();
    }
    return dApp1;
}

void Application::setMainWindow(MainFrame *window)
{
    if (nullptr != window) {
        m_mainwindow = window;
    }
}

MainFrame *Application::getMainWindow()
{
    return m_mainwindow;
}
