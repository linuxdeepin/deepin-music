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

#include "application.h"
//#include "config.h"

#include "mainframe.h"

#include <QDebug>
#include <QTranslator>
#include <QIcon>
#include <QImageReader>
#include <sys/time.h>
#include <DApplicationSettings>
#include <QFile>
#include <DLog>
//#include <dgiofile.h>
//#include <dgiofileinfo.h>
#include <DAboutDialog>
namespace {
//const QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + "deepin" + QDir::separator() + "deepin-album"/* + QDir::separator()*/;
}
// namespace

#define IMAGE_HEIGHT_DEFAULT    100
#define IMAGE_LOAD_DEFAULT    100

DApplication *Application::dAppNew = nullptr;
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

DApplication *Application::getDAppNew()
{
    return dAppNew;
}

Application *Application::getApp()
{
    if (dApp1 == nullptr)
        dApp1 = new Application;
    return dApp1;
}

void Application::setApp(DApplication *app)
{
    dAppNew = app;
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
