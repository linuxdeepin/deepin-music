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

#ifndef QTESTDEFINE_H
#define QTESTDEFINE_H
#include <QtTest>
#include <QCoreApplication>
//#include "application.h"
#include "mainframe.h"
#include <QTest>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

static int count_testDefine = 0;
//static bool switch_on_test = true;
static QString testPath_test = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QDir::separator() + "test";
static QString appPath_test = QApplication::applicationDirPath() + QDir::separator() + "test";

#endif // QTESTDEFINE_H
