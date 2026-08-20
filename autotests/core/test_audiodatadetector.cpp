// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/core/audiodatadetector.h (class AudioDataDetector).
#include <gtest/gtest.h>
#include <QObject>
#include <QString>
#include <QVector>
#include <QThread>
#include "core/audiodatadetector.h"

TEST(AudioDataDetectorTest, Construct_NoThrow) {
    AudioDataDetector d;
    SUCCEED();
}
TEST(AudioDataDetectorTest, Destruct_NoThrow) {
    {
        AudioDataDetector d;
    }
    SUCCEED();
}
TEST(AudioDataDetectorTest, OnClearBufferDetector_NoThrow) {
    AudioDataDetector d;
    EXPECT_NO_THROW(d.onClearBufferDetector());
}
TEST(AudioDataDetectorTest, OnBufferDetector_NonexistentPath_NoThrow) {
    AudioDataDetector d;
    EXPECT_NO_THROW(d.onBufferDetector("/tmp/no-such-audio.mp3", "hash-nonexistent"));
    d.onClearBufferDetector();
}
TEST(AudioDataDetectorTest, OnBufferDetector_EmptyPath_NoThrow) {
    AudioDataDetector d;
    EXPECT_NO_THROW(d.onBufferDetector("", ""));
    d.onClearBufferDetector();
}
