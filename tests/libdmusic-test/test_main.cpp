// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QCoreApplication>
#include <QTest>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#ifndef __mips__
#include <sanitizer/asan_interface.h>
#endif


class QTestMain : public QObject
{
    Q_OBJECT

public:
    QTestMain(int &argc, char **argv);

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testGTest();

private:
    int m_argc;
    char **m_argv;
};

QTestMain::QTestMain(int &argc, char **argv)
{
    m_argc = argc;
    m_argv = argv;
}

void QTestMain::initTestCase()
{
    qDebug() << "=====start test=====";
}

void QTestMain::cleanupTestCase()
{
    qDebug() << "=====stop test=====";
    // 不再调用 exit(0)：直接 exit 会绕过进程正常退出流程，导致 gcov 的 .gcda
    // 覆盖率数据无法 flush 到磁盘，覆盖率统计恒为 0。改为正常返回，让 QTest::qExec
    // 返回后进程自然退出，atexit 注册的 __gcov_dump 得以执行。
}

void QTestMain::testGTest()
{
    testing::GTEST_FLAG(output) = "xml:./report/report_deepin-music-test.xml";
    testing::InitGoogleTest(&m_argc,m_argv);
    int ret = RUN_ALL_TESTS();
#ifndef __mips__
    __sanitizer_set_report_path("asan.log");
#endif
    Q_UNUSED(ret)
}


int main(int argc, char *argv[])
{
    // QSqlDatabase（DataManager 初始化时使用）等 Qt 模块要求 QCoreApplication 已创建，
    // 否则 Presenter 构造 → DataManager::initPlaylist → QSqlDatabase::open 会解引用空指针 (SIGSEGV)。
    QCoreApplication app(argc, argv);

    QTestMain testMain(argc, argv);
    QTest::qExec(&testMain, argc, argv);
    return 0;
}


#include "test_main.moc"
