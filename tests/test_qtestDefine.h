// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
