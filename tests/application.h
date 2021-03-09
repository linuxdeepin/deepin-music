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

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <DApplication>

class MainFrame;
class Application;
class ConfigSetter;
class DatabaseManager;
class DBManager;
class Exporter;
class Importer;
class ScanPathsDialog;
class SignalManager;
class WallpaperSetter;
class ViewerThemeManager;
#if defined(dApp)
#undef dApp
#endif
//#define dApp (static_cast<Application *>(QCoreApplication::instance()))
#define dApp (Application::getApp())

DWIDGET_USE_NAMESPACE

class Application : public QObject
{
    Q_OBJECT

public:
    Application(/*int &argc, char **argv*/);
    ~Application();
    static Application *getInstance();

    DApplication *getDAppNew();
    static Application *getApp();
    static void setApp(DApplication *);
public :
    //test
    void setMainWindow(MainFrame *window);
    MainFrame *getMainWindow();
public:
    MainFrame *m_mainwindow = nullptr;
    static DApplication *dAppNew;
    static Application *dApp1;
};

#endif  // APPLICATION_H_
