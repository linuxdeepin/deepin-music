#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

TEST(UnitTest, Empty)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}
