// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/util/singleton.h (template DSingleton<T>).
#include <gtest/gtest.h>
#include "util/singleton.h"

namespace {
struct SgTestType {
    int v = 42;
};
}  // namespace

TEST(DSingletonTest, GetInstance_ReturnsNonNull) {
    auto *p = DMusic::DSingleton<SgTestType>::getInstance();
    ASSERT_NE(p, nullptr);
}
TEST(DSingletonTest, GetInstance_ReturnsSameInstance) {
    auto *a = DMusic::DSingleton<SgTestType>::getInstance();
    auto *b = DMusic::DSingleton<SgTestType>::getInstance();
    EXPECT_EQ(a, b);
}
TEST(DSingletonTest, Instance_StateAccessible) {
    auto *p = DMusic::DSingleton<SgTestType>::getInstance();
    EXPECT_EQ(p->v, 42);
}
