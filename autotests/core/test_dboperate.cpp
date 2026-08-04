// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/core/dboperate.h (class DBOperate).
#include <gtest/gtest.h>
#include <QObject>
#include <QStringList>
#include <QSet>
#include <QString>
#include "global.h"
#include "core/dboperate.h"

TEST(DBOperateTest, Construct_WithSuffixes_NoThrow) {
    QStringList suffixes = {"mp3", "flac", "wav"};
    DBOperate op(suffixes);
    SUCCEED();
}
TEST(DBOperateTest, Construct_EmptySuffixes_NoThrow) {
    QStringList empty;
    DBOperate op(empty);
    SUCCEED();
}
TEST(DBOperateTest, SlotClearImportingHash_EmptyString_NoThrow) {
    QStringList s{"mp3"};
    DBOperate op(s);
    EXPECT_NO_THROW(op.slotClearImportingHash("nonexistent-hash"));
}
TEST(DBOperateTest, SlotImportMetas_EmptyUrls_NoThrow) {
    QStringList s{"mp3"};
    DBOperate op(s);
    EXPECT_NO_THROW(op.slotImportMetas(QStringList(), QSet<QString>(), false,
                                       QSet<QString>(), QSet<QString>(), "", false));
}
TEST(DBOperateTest, SlotImportMetas_WithUrlsButNoFiles_NoThrow) {
    QStringList s{"mp3"};
    DBOperate op(s);
    QStringList urls = {"/tmp/nonexistent-a.mp3", "/tmp/nonexistent-b.flac"};
    QSet<QString> hashes = {"h1", "h2"};
    EXPECT_NO_THROW(op.slotImportMetas(urls, hashes, false,
                                       QSet<QString>(), hashes, "playhash", false));
}
