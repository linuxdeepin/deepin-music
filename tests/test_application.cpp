#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "../music-player/view/widget/soundvolume.h"

int sum(int a, int b)
{
    return a + b;
}

TEST(qqq, www)
{
    ASSERT_EQ(2, sum(1, 1));
}

TEST(Application, isRunning3)
{
    SoundVolume *a = new  SoundVolume() ;
    ASSERT_EQ(20, a->radius());
}
