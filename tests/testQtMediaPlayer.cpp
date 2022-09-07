// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "application.h"
#include "ac-desktop-define.h"

#include "mediameta.h"
#include "metadetector.h"
#include "core/util/global.h"
#include "core/qtplayer.h"

#include <QStandardPaths>



TEST(Application, qtPlayerGetCoverData)
{
    TEST_CASE_NAME("qtPlayerGetCoverData")
    MediaMeta meta;
    QList<QUrl> li;
    QString lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    lastImportPath += "/歌曲/004.mp3";
    meta.localPath = lastImportPath;
    meta.hash = "004_hash_001";

    meta.getCoverData(Global::cacheDir(), 0);

    QPixmap img = MetaDetector::getCoverDataPixmap(meta, 0);
}

TEST(Application, qtPlayerStartPlaying)
{
    MediaMeta meta;
    QList<QUrl> li;
    QString lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    lastImportPath += "/歌曲/004.mp3";
    meta.localPath = lastImportPath;
    meta.hash = "004_hash_001";

    QtPlayer m_player;
    m_player.setMute(false);
    m_player.setVolume(100);

    m_player.setMediaMeta(meta);
    m_player.play();
    m_player.getMute();
    m_player.length();

    m_player.pause();
    qint64 time = m_player.time();

    if (m_player.state() == PlayerBase::Paused) {
        m_player.setTime(time);
        m_player.play();
    }
    m_player.setFadeInOutFactor(1.0);
    m_player.stop();
}

