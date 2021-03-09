/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
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

#include <QTest>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <infodialog.h>
#include <DApplication>

#include "ac-desktop-define.h"

#include "mainframe.h"
#include "musicbaselistview.h"
#include "playlistview.h"
#include "musicsonglistview.h"
#include "databaseservice.h"

TEST(Application, dataBaseService)
{
    TEST_CASE_NAME("dataBaseService")

    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QTest::qWait(50);

    DataBaseService::getInstance()->getMusicInfoByHash("all");
    DataBaseService::getInstance()->getCustomSongList();
//    DataBaseService::getInstance()->getDatabase();
//    DataBaseService::getInstance()->getImportStatus();

    QList<MediaMeta> metaList;
    metaList.append(MediaMeta());
    DataBaseService::getInstance()->addMetaToPlaylist("all", metaList);
    DataBaseService::getInstance()->updateMetaCodec(MediaMeta());
    DataBaseService::getInstance()->setFirstSong("");
    DataBaseService::getInstance()->updatePlaylistDisplayName("新建歌单", "");
    DataBaseService::getInstance()->signalCoverUpdate(Player::getInstance()->getActiveMeta());

    QTest::qWait(50);
}



