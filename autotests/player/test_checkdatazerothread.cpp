// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/player/vlc/checkdatazerothread.h.
#include <gtest/gtest.h>
#include <QStringList>
#include "vlc/Instance.h"
#include "vlc/sdlplayer.h"
#include "vlc/checkdatazerothread.h"

TEST(CheckDataZeroThreadTest, Construct_WithPlayer_NoThrow) {
    QStringList args{"--ignore-config", "--no-video", "--no-audio"};
    VlcInstance *inst = new VlcInstance(args);
    SdlPlayer *player = new SdlPlayer(inst);
    EXPECT_NO_THROW({ CheckDataZeroThread t(nullptr, player); });
    delete player;
    delete inst;
}
TEST(CheckDataZeroThreadTest, QuitThread_NoThrow) {
    QStringList args{"--ignore-config", "--no-video", "--no-audio"};
    VlcInstance *inst = new VlcInstance(args);
    SdlPlayer *player = new SdlPlayer(inst);
    CheckDataZeroThread t(nullptr, player);
    EXPECT_NO_THROW(t.quitThread());
    delete player;
    delete inst;
}
TEST(CheckDataZeroThreadTest, InitTimeParams_NoThrow) {
    QStringList args{"--ignore-config", "--no-video", "--no-audio"};
    VlcInstance *inst = new VlcInstance(args);
    SdlPlayer *player = new SdlPlayer(inst);
    CheckDataZeroThread t(nullptr, player);
    EXPECT_NO_THROW(t.initTimeParams());
    delete player;
    delete inst;
}
