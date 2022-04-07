/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangWenChao <zhangwenchao@uniontech.com>
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

