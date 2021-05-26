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

#include <QDebug>
#include <QAction>
#include <QProcess>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QPixmap>
#include <QTestEventList>
#include <QPointF>
#include <QWheelEvent>
#include <QTest>

#include <DUtil>
#include <QMimeData>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DTitlebar>
#include <DImageButton>
#include <DFileDialog>
#include <DHiDPIHelper>
#include <QLineEdit>
#include <DSettingsDialog>
#include <ddropdown.h>

#include "widget/ddropdown.h"
#include "widget/label.h"
#include "widget/titlebarwidget.h"
#include "widget/waveformscale.h"
//#include "../music-player/view/footerwidget.h"
#include "playqueuewidget.h"
#include "musiclyricwidget.h"
#include "medialibrary.h"
#include "mediameta.h"
#include "metadetector.h"
#include "util/basetool.h"
#include "util/encodingdetector.h"
#include "util/pinyinsearch.h"
#include "metabufferdetector.h"
#include "player.h"
#include "musiclyric.h"
#include "lyriclabel.h"

#include <vlc/vlc.h>
#include "vlc/Error.h"
#include "vlc/Common.h"
#include "vlc/Enums.h"
#include "vlc/Instance.h"
#include "vlc/Media.h"
#include "vlc/MediaPlayer.h"
#include "vlc/Equalizer.h"

//#include "widget/closeconfirmdialog.h"
//#include "widget/playlistitem.h"
#include "mpris.h"
#include "mprismanager.h"
#include "mpriscontroller.h"
#include "mprisplayer.h"
#include "speech/speechCenter.h"
//#include "threadpool.h"
#include "vlc/vlcdynamicinstance.h"
#include "dbusutils.h"
#include "footerwidget.h"
#include "commonservice.h"
#include "databaseservice.h"
#include "ac-desktop-define.h"
#include "mainframe.h"
#include "musiclistdatawidget.h"
#include "application.h"
#include "musicbaselistview.h"
#include "musicsonglistview.h"
#include "playlistview.h"
#include "musicpixmapbutton.h"
#include "waveform.h"
#include "soundvolume.h"
#include "searchedit.h"
#include "titlebarwidget.h"
#include "searchresulttabwidget.h"
#include "musicsearchlistview.h"
#include "listView/singerList/singerdelegate.h"
#include "albumlistview.h"
//#include "musiclistinfoview.h"
#include "dequalizerdialog.h"
#include "infodialog.h"
#include "closeconfirmdialog.h"
#include "subsonglistwidget.h"
#include "songlistview.h"

bool copyDirFiles(const QString &fromDir, const QString &toDir)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);

    if (!targetDir.exists()) {
        if (!targetDir.mkdir(targetDir.absolutePath())) {
            return false;
        }
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    for (auto fileInfo : fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
            continue;
        }

        if (fileInfo.isDir()) {
            if (!copyDirFiles(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) {
                return false;
            }
        } else {
            if (!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) {
                return false;
            }
        }
    }

    return true;
}

TEST(Application, copyMusicToMusicDir1)
{
    // 拷贝音乐文件夹到系统音乐文件夹下
    TEST_CASE_NAME("other")

    QDir dir;
    dir.cd("../resource");
    // 启动方式不同，路径不同
    if (!dir.path().contains("resource")) {
        dir.setPath("../../../tests/resource");
    }

    QStringList stringList = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    if (stringList.size() > 0) {
        stringList[0].append("/歌曲");

        QDir deleteDir(stringList[0]);
        deleteDir.removeRecursively();

        QTest::qWait(50);
        copyDirFiles(dir.path(), stringList[0]);
    }

    QTest::qWait(50);
}

TEST(Application, init)
{
    // 获取音乐数量
    int count = DataBaseService::getInstance()->allMusicInfosCount();
    qDebug() << __FUNCTION__ << "count = " << count;
    // 新建歌单
    DataBaseService::PlaylistData info;
    info.editmode = true;
    info.readonly = false;
    info.uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    info.displayName = "测试歌单";
    info.sortID = DataBaseService::getInstance()->getPlaylistMaxSortid();
    info.sortType = DataBaseService::SortByAddTimeASC;
    DataBaseService::getInstance()->addPlaylist(info);
    // 查询歌曲是否在歌单中
    DataBaseService::getInstance()->isMediaMetaInSonglist(info.uuid, "aaa");
    // 查询全部歌单
    DataBaseService::getInstance()->allPlaylistMeta();
    // 更新歌单1
    QVector<DataBaseService::PlaylistData> playlistDataList;
    info.displayName = "测试歌单2";
    playlistDataList << info;
    DataBaseService::getInstance()->updatePlaylist(playlistDataList);
    // 更新歌单2
    DataBaseService::getInstance()->updatePlaylistDisplayName("测试歌单3", info.uuid);
    // 移除歌单
    DataBaseService::getInstance()->deletePlaylist(info.uuid);
    QStringList lsit = DataBaseService::getInstance()->getDelMetaHashs();
    // 查询所有音乐
    QTest::qWait(100);
    QList<MediaMeta> metaList = DataBaseService::getInstance()->allMusicInfos(true);
    // 删除所有音乐
    QStringList delList;
    for (MediaMeta meta : metaList) {
        delList << meta.hash;
    }
    emit DataBaseService::getInstance()->sigRemoveSelectedSongs("all", delList, false);
    // 新建歌单
    info.editmode = true;
    info.readonly = false;
    info.uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    info.displayName = "测试歌单";
    info.sortID = DataBaseService::getInstance()->getPlaylistMaxSortid();
    info.sortType = DataBaseService::SortByAddTimeASC;
    DataBaseService::getInstance()->addPlaylist(info);
    // 导入歌曲
    QStringList importList;
    QStringList stringList = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    if (stringList.size() > 0) {
        stringList[0].append("/歌曲/001.mp3");
        importList << stringList[0];
        QTest::qWait(50);
    }
    DataBaseService::getInstance()->importMedias(info.uuid, importList);
    QTest::qWait(500);
}

TEST(Application, importLinkText)
{
    // 扫描歌曲
    TEST_CASE_NAME("importLinkText")

    QTest::qWait(100);
    MainFrame *w = Application::getInstance()->getMainWindow();
    QLabel *ilt = w->findChild<QLabel *>(AC_importLinkText);
    ilt->linkActivated("");
    QCOMPARE(5, DataBaseService::getInstance()->allMusicInfosCount());
    QTest::qWait(500);
}

TEST(Application, deleteAllMusic)
{
    // 删除所有音乐
    TEST_CASE_NAME("deleteAllMusic")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QPoint pos = QPoint(130, 30);
    QTestEventList event;

    // 点击专辑
    pos = QPoint(130, 30);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(100);

    // 点击所有音乐
    QTest::qWait(100);
    pos = QPoint(130, 100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();
    QTest::qWait(100);


    // dialog list 点击
    QTest::qWait(50);
    QTimer::singleShot(1000, w, [ = ]() {
        QTimer::singleShot(1000, w, [ = ]() {
            // 清空ListView
            QTest::qWait(50);
            DDialog *messageBox = w->findChild<DDialog *>("MessageBox");
            if (messageBox) {
                QTestEventList event;
                event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key::Key_Tab, Qt::NoModifier, 50);
                event.addKeyClick(Qt::Key::Key_Enter, Qt::NoModifier, 50);
                event.simulate(messageBox);
                event.clear();

                PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
                if (plv) {
                    QList<MediaMeta> metas = DataBaseService::getInstance()->allMusicInfos();
                    QStringList strlist;
                    for (MediaMeta item : metas) {
                        strlist << item.hash;
                    }
                    DataBaseService::getInstance()->removeSelectedSongs("all", strlist, true);
                    Player::getInstance()->playRmvMeta(strlist);
                }
            }
        });

        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });

    // 全选
    pos = QPoint(20, 20);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);

    event.addMouseMove(pos);
    event.addKeyClick(Qt::Key_A, Qt::ControlModifier, 10);
    event.simulate(plv->viewport());
    event.clear();

    QTest::qWait(100);
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(plv->viewport(), &menuEvent);

    QTest::qWait(1000);
}

TEST(Application, copyMusicToMusicDir2)
{
    // 拷贝音乐文件夹到系统音乐文件夹下
    TEST_CASE_NAME("other")

    QDir dir;
    dir.cd("../resource");
    // 启动方式不同，路径不同
    if (!dir.path().contains("resource")) {
        dir.setPath("../../../tests/resource");
    }

    QStringList stringList = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    if (stringList.size() > 0) {
        stringList[0].append("/歌曲");

        QDir deleteDir(stringList[0]);
        deleteDir.removeRecursively();

        QTest::qWait(50);
        copyDirFiles(dir.path(), stringList[0]);
    }

    QTest::qWait(50);
}

TEST(Application, importLinkText1)
{
    // 扫描歌曲
    TEST_CASE_NAME("importLinkText1")

    QTest::qWait(100);
    MainFrame *w = Application::getInstance()->getMainWindow();
    QLabel *ilt = w->findChild<QLabel *>(AC_importLinkText);
    ilt->linkActivated("");
    QTest::qWait(500);
}

//// Dialg窗口
//TEST(Application, musicListDialg)
//{
//    TEST_CASE_NAME("musicListDialg")

//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
//    QTest::qWait(50);
//    // 点击专辑
//    QPoint pos(130, 20);
//    QTestEventList event;
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(baseListView->viewport());
//    event.clear();

//    // 双击list
//    pos = QPoint(20, 20);
//    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(alv->viewport());
//    event.clear();

//    // 二级页面点击，界面改动，case位置需要改变
//    QTest::qWait(200);
//    MusicListInfoView *mliv = w->findChild<MusicListInfoView *>(AC_musicListInfoView);
//    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.simulate(mliv->viewport());
//    event.clear();
//    QTest::qWait(100);

//    // 播放
//    QTimer::singleShot(500, w, [ = ]() {
//        QTestEventList event;
//        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
//        event.addDelay(100);
//        event.simulate(menuWidget);
//        event.clear();
//        QTest::qWait(100);
//    });
//    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
//    qApp->sendEvent(mliv->viewport(), &menuEvent);
//    QTest::qWait(600);

//    // 暂停
//    QTimer::singleShot(500, w, [ = ]() {
//        QTestEventList event;
//        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
//        event.addDelay(100);
//        event.simulate(menuWidget);
//        event.clear();
//        QTest::qWait(100);
//    });
//    qApp->sendEvent(mliv->viewport(), &menuEvent);
//    QTest::qWait(600);
//    emit backBtn->clicked();

//    QTest::qWait(200);
//}

// Dialg窗口
TEST(Application, musicListDialg1)
{
    TEST_CASE_NAME("musicListDialg")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(alv->viewport());
    event.clear();

    // 二级页面点击，界面改动，case位置需要改变
    QTest::qWait(200);
    PlayListView *mliv = w->findChild<PlayListView *>(AC_musicListInfoView);
    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

    if (mliv == nullptr) {
        return;
    }
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(mliv->viewport());
    event.clear();
    QTest::qWait(100);

    // 添加到我的收藏
    QTimer::singleShot(500, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        if (menuWidget) {
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.addDelay(100);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(100);

            menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(100);
        }
    });
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(mliv->viewport(), &menuEvent);
    QTest::qWait(700);

    // 添加到新的歌单
    QTimer::singleShot(500, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        if (menuWidget) {
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.addDelay(50);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(50);

            menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(50);
        }
    });
    qApp->sendEvent(mliv->viewport(), &menuEvent);

    // 添加到自定义歌单
    QTest::qWait(1500);
    QTimer::singleShot(500, w, [ = ]() {
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        if (menuWidget) {
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.addDelay(50);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(50);

            menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(menuWidget);
            event.clear();
            QTest::qWait(500);
            emit backBtn->clicked();
        }
    });
    qApp->sendEvent(mliv->viewport(), &menuEvent);
    QTest::qWait(200);
}

// Dialg窗口
TEST(Application, musicListDialg3)
{
    TEST_CASE_NAME("musicListDialg")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(alv->viewport());
    event.clear();

    // 二级页面点击，界面改动，case位置需要改变
    QTest::qWait(200);
    PlayListView *mliv = w->findChild<PlayListView *>(AC_musicListInfoView);
    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(mliv->viewport());
    event.clear();
    QTest::qWait(100);

    // 从歌单中删除
    QTimer::singleShot(300, w, [ = ]() {
        QTimer::singleShot(300, w, [ = ]() {
            QTestEventList event;
            DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(messageBox);
            event.clear();
            w->slotLeftClicked();
        });
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(mliv->viewport(), &menuEvent);
    QTest::qWait(1000);
}

// Dialg窗口
TEST(Application, musicListDialg4)
{
    TEST_CASE_NAME("musicListDialg")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(alv->viewport());
    event.clear();

    // 二级页面点击，界面改动，case位置需要改变
    QTest::qWait(200);
    PlayListView *mliv = w->findChild<PlayListView *>(AC_musicListInfoView);
    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(mliv->viewport());
    event.clear();
    QTest::qWait(100);

    // 从本地中删除
    QTimer::singleShot(500, w, [ = ]() {
        QTimer::singleShot(800, w, [ = ]() {
            QTestEventList event;
            DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
            event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
            event.simulate(messageBox);
            event.clear();
            QTest::qWait(500);
            emit backBtn->clicked();
        });
        QTestEventList event;
        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.addDelay(100);
        event.simulate(menuWidget);
        event.clear();
        QTest::qWait(100);
    });
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
    qApp->sendEvent(mliv->viewport(), &menuEvent);
    QTest::qWait(200);
}

// Dialg窗口
TEST(Application, musicListDialg5)
{
    TEST_CASE_NAME("musicListDialg5")

//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
//    QTest::qWait(50);
//    // 点击专辑
//    QPoint pos(130, 20);
//    QTestEventList event;
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(baseListView->viewport());
//    event.clear();

//    // 双击list
//    pos = QPoint(20, 20);
//    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(alv->viewport());
//    event.clear();

//    // 二级页面点击，界面改动，case位置需要改变
//    QTest::qWait(200);
//    PlayListView *mliv = w->findChild<PlayListView *>(AC_musicListInfoView);
//    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.simulate(mliv->viewport());
//    event.clear();
//    QTest::qWait(100);

//    // 歌曲信息
//    QTimer::singleShot(500, w, [ = ]() {
//        QTimer::singleShot(800, w, [ = ]() {
//            InfoDialog *infoDialog = w->findChild<InfoDialog *>(AC_infoDialog);
//            if (infoDialog)
//                infoDialog->close();
//            QTest::qWait(500);
//            emit backBtn->clicked();
//        });
//        QTestEventList event;
//        DMenu *menuWidget = static_cast<DMenu *>(qApp->activePopupWidget());
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
//        event.addDelay(100);
//        event.simulate(menuWidget);
//        event.clear();
//        QTest::qWait(100);
//    });
//    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(20, 20));
//    qApp->sendEvent(mliv->viewport(), &menuEvent);
//    QTest::qWait(200);
}

// Dialg窗口
TEST(Application, musicListDialg6)
{
    TEST_CASE_NAME("musicListDialg")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(alv->viewport());
    event.clear();

    // 二级页面点击，界面改动，case位置需要改变
    QTest::qWait(200);
    PlayListView *mliv = w->findChild<PlayListView *>(AC_musicListInfoView);
    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(mliv->viewport());
    event.clear();
    QTest::qWait(100);

    QTimer::singleShot(800, w, [ = ]() {
        QTestEventList event;
        DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
        event.simulate(messageBox);
        event.clear();
        QTest::qWait(500);
        emit backBtn->clicked();
    });

    // Dialog KeyPress
    QKeyEvent keyEvent(QKeyEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    qApp->sendEvent(mliv->viewport(), &keyEvent);

    QKeyEvent keyEvent1(QKeyEvent::KeyPress, Qt::Key_Delete, Qt::ShiftModifier);
    qApp->sendEvent(mliv->viewport(), &keyEvent1);
    QTest::qWait(200);
}

// 界面修改，该case需要修改
// Dialg窗口
//TEST(Application, musicListDialg7)
//{
//    TEST_CASE_NAME("musicListDialg")

//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
//    QTest::qWait(50);
//    // 点击专辑
//    QPoint pos(130, 20);
//    QTestEventList event;
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(baseListView->viewport());
//    event.clear();

//    // 双击list
//    pos = QPoint(20, 20);
//    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
//    event.simulate(alv->viewport());
//    event.clear();

//    // 二级页面点击，界面改动，case位置需要改变
//    QTest::qWait(200);
//    MusicListInfoView *mliv = w->findChild<MusicListInfoView *>(AC_musicListInfoView);
//    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.simulate(mliv->viewport());
//    event.clear();
//    QTest::qWait(100);

//    QTimer::singleShot(800, w, [ = ]() {
//        QTestEventList event;
//        DDialog *messageBox = w->findChild<DDialog *>(AC_MessageBox);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
//        event.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 50);
//        event.simulate(messageBox);
//        event.clear();
//        QTest::qWait(500);
//        emit backBtn->clicked();
//    });

//    QKeyEvent keyEvent1(QKeyEvent::KeyPress, Qt::Key_I, Qt::ControlModifier);
//    qApp->sendEvent(mliv->viewport(), &keyEvent1);
//    QTest::qWait(1000);
//}

// Dialg 播放所有，随机播放
TEST(Application, musicListDialg8)
{
    TEST_CASE_NAME("musicListDialg")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(50);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(alv->viewport());
    event.clear();

    // 二级页面点击，界面改动，case位置需要改变
    QTest::qWait(200);
    DPushButton *playAll = w->findChild<DPushButton *>(AC_dialogPlayAll);
    DPushButton *playRandom = w->findChild<DPushButton *>(AC_dialogPlayRandom);
    DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(playAll);
    event.clear();
    QTest::qWait(500);

    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(playRandom);
    event.clear();

    QTest::qWait(200);
    emit backBtn->clicked();
}

TEST(Application, musicListDialg9)
{
    TEST_CASE_NAME("musicListDialg9")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 关闭专辑弹窗
    QTimer::singleShot(200, w, [ = ]() {
        QTest::qWait(500);
        PlayListView *mliv = w->findChild<PlayListView *>(AC_musicListInfoView);
        DPushButton *backBtn = w->findChild<DPushButton *>(AC_titleBarLeft);

        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
        emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::DarkType);

        if (mliv) {
            QTestEventList event;
            QPoint pos = QPoint(20, 20);
            event.addMouseMove(pos);
            event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
            event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
            event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
            event.simulate(mliv->viewport());
            event.clear();
        }

        QTest::qWait(100);
        if (backBtn) {
            emit backBtn->clicked();
        }

        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
        emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::LightType);
        QTest::qWait(500);
    });

    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(alv->viewport());
    event.clear();
    QTest::qWait(400);
}

TEST(Application, copyMusicToMusicDir4)
{
    // 拷贝音乐文件夹到系统音乐文件夹下
    TEST_CASE_NAME("copyMusicToMusicDir")

    QDir dir;
    dir.cd("../resource");
    // 启动方式不同，路径不同
    if (!dir.path().contains("resource")) {
        dir.setPath("../../../tests/resource");
    }

    QStringList stringList = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    if (stringList.size() > 0) {
        stringList[0].append("/歌曲");

        QDir deleteDir(stringList[0]);
        deleteDir.removeRecursively();

        QTest::qWait(50);
        copyDirFiles(dir.path(), stringList[0]);
    }

    QTest::qWait(50);
}

TEST(Application, importLinkText2)
{
    // 扫描歌曲
    TEST_CASE_NAME("importLinkText2")

    QTest::qWait(100);
    MainFrame *w = Application::getInstance()->getMainWindow();
    QLabel *ilt = w->findChild<QLabel *>(AC_importLinkText);
    ilt->linkActivated("");
    QTest::qWait(500);
}

TEST(Application, viewChanged)
{
    TEST_CASE_NAME("viewChanged")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);

    // 点击专辑
    QTest::qWait(50);
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(50);
    DDropdown *albumDropdown = w->findChild<DDropdown *>(AC_albumDropdown);
    QList<QAction *> actionList = albumDropdown->actions();
    for (auto item = actionList.begin(); item != actionList.end(); ++item) {
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
    }

    // icon模式
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = actionList.begin(); item != actionList.end(); ++item) {
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
    }

    QTest::qWait(50);
    // 点击切换回list模式
    DToolButton *iconListBtn = w->findChild<DToolButton *>(AC_btlistMode);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconListBtn);
    event.clear();

    // 点击歌手
    QTest::qWait(50);
    pos.setY(60);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(50);
    DDropdown *artistDropdown = w->findChild<DDropdown *>(AC_artistDropdown);
    QList<QAction *> artistList = artistDropdown->actions();
    for (auto item = artistList.begin(); item != artistList.end(); ++item) {
        emit artistDropdown->triggered(*item);
        QTest::qWait(50);
        emit artistDropdown->triggered(*item);
        QTest::qWait(50);
    }

    // icon模式
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = artistList.begin(); item != artistList.end(); ++item) {
        emit artistDropdown->triggered(*item);
        QTest::qWait(50);
        emit artistDropdown->triggered(*item);
        QTest::qWait(50);
    }

    QTest::qWait(50);
    // 点击切换回list模式
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconListBtn);
    event.clear();


    // 点击所有音乐
    QTest::qWait(50);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(50);
    DDropdown *allMusicDropdown = w->findChild<DDropdown *>(AC_musicDropdown);
    QList<QAction *> allMusicList = allMusicDropdown->actions();
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
    }

    // icon模式
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
    }

    QTest::qWait(50);
    // 点击切换回list模式
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconListBtn);
    event.clear();


    // 点击我的收藏
    QTest::qWait(50);
    pos.setY(130);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(50);
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
    }

    // icon模式
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
        emit allMusicDropdown->triggered(*item);
        QTest::qWait(50);
    }

    QTest::qWait(50);
    // 点击切换回list模式
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconListBtn);
    event.clear();
}

//TEST(Application, customViewChanged)
//{
//    TEST_CASE_NAME("customViewChanged")

//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);
//    QTest::qWait(500);
//    //点击自定义列表
//    QPoint pos(130, 20);
//    QTestEventList event;
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.simulate(songListView->viewport());
//    event.clear();

//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
//    QTest::qWait(500);
//    //点击所有歌曲
//    pos.setY(100);
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.simulate(baseListView->viewport());
//    event.clear();
//}

TEST(Application, iconModeChanged)
{
    TEST_CASE_NAME("iconModeChanged")

    MainFrame *w = Application::getInstance()->getMainWindow();

    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);

    QTest::qWait(500);
    QPoint pos = QPoint(20, 120);
    QTestEventList event;

    // 点击专辑
    QTest::qWait(500);
    pos = QPoint(20, 15);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // icon模式
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

    // 双击icon播放
    QTest::qWait(500);
    pos = QPoint(100, 80);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    QMouseEvent mouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(plv, &mouseEvent);

    Player::getInstance()->resume();

    QTest::qWait(500);
    pos.setY(-10);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();

    Player::getInstance()->pause();

    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();
    QTest::qWait(500);



    // 点击歌手
    pos = QPoint(20, 50);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // icon模式
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

    // 双击icon播放

    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();

    Player::getInstance()->pause();

    QTest::qWait(500);
    pos.setY(-10);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();

    Player::getInstance()->resume();

    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();



    // 点击所有音乐
    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // icon模式
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

    // 双击icon播放

    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();

    Player::getInstance()->pause();

    QTest::qWait(500);
    pos.setY(-10);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();

    Player::getInstance()->resume();

    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.simulate(plv->viewport());
    event.clear();


    QTest::qWait(500);
}

TEST(Application, preMusic)
{
    TEST_CASE_NAME("preMusic")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DButtonBoxButton *preBtn = w->findChild<DButtonBoxButton *>(AC_Prev);
    QTest::qWait(50);
    // 点击上一首
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(preBtn);
    event.clear();

    DButtonBoxButton *playBtn = w->findChild<DButtonBoxButton *>(AC_Play);
    QTest::qWait(50);
    // 点击播放
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addDelay(3000);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event1.simulate(playBtn);
    event1.clear();

    DButtonBoxButton *nextBtn = w->findChild<DButtonBoxButton *>(AC_Next);
    QTest::qWait(50);
    // 点击下一首
    QTestEventList event2;
    event2.addMouseMove(pos);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event2.simulate(nextBtn);
    event2.clear();
}

TEST(Application, btCover)
{
    TEST_CASE_NAME("btCover")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicPixmapButton *mpb = w->findChild<MusicPixmapButton *>(AC_btCover);
    QTest::qWait(50);
    // 点击头像按钮
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(mpb);
    event.clear();
    QTest::qWait(1000);
}

TEST(Application, waveform)
{
    TEST_CASE_NAME("waveform")

    MainFrame *w = Application::getInstance()->getMainWindow();
    Waveform *wf = w->findChild<Waveform *>(AC_Waveform);
    QTest::qWait(50);
    // 点击进度条
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseMove(pos);

    pos = QPoint(200, 20);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseMove(pos);

    pos = QPoint(300, 20);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseMove(pos);
    event.simulate(wf);
    event.clear();

    QMouseEvent mouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(wf, &mouseEvent);

    QMouseEvent mouseMoveEvent(QEvent::MouseMove, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(wf, &mouseMoveEvent);
}

TEST(Application, btFavorite)
{
    TEST_CASE_NAME("btFavorite")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_Favorite);
    QTest::qWait(50);
    // 点击进收藏按钮
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(bt);
    event.clear();
}

TEST(Application, playMode)
{
    TEST_CASE_NAME("playMode")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_PlayMode);
    QTest::qWait(50);
    // 点击播放模式
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(bt);
    event.clear();
}

TEST(Application, btSound)
{
    TEST_CASE_NAME("btSound")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_Sound);
    QTest::qWait(50);

    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(bt);
    event.clear();

    SoundVolume *sv = w->findChild<SoundVolume *>(AC_VolSlider);
    QTest::qWait(200);
    sv->setVolume(100);
    QTest::qWait(200);
    sv->setVolume(50);
    QTest::qWait(200);
    sv->setVolume(0);
    QTest::qWait(200);
    sv->setVolume(65);
    QTest::qWait(200);


    QWheelEvent wheelEvent(pos, 10, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(sv, &wheelEvent);
    QWheelEvent wheelEvent1(pos, -10, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(sv, &wheelEvent1);

    sv->delayHide();
}

TEST(Application, btPlayList)
{
    TEST_CASE_NAME("btPlayList")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_PlayQueue);
    QTest::qWait(50);

    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(bt);
    event.clear();
}

TEST(Application, searchSort)
{
    TEST_CASE_NAME("searchSort")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 歌曲搜索
    QLineEdit *se = w->findChild<QLineEdit *>(AC_Search);

    // 点击所有音乐
    QTest::qWait(50);
    QPoint pos1(130, 100);
    QTestEventList event;
    event.addMouseMove(pos1);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 10);
    event.simulate(baseListView->viewport());
    event.clear();


    // 搜索a
    QTest::qWait(50);
    QPoint pos(100, 20);
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    event1.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 100);
    event1.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 100);
    event1.simulate(se);
    event1.clear();

    // list排序
    QTest::qWait(50);
    DDropdown *albumDropdown = w->findChild<DDropdown *>(AC_albumDropdown);
    QList<QAction *> actionList = albumDropdown->actions();
    for (auto item = actionList.begin(); item != actionList.end(); ++item) {
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
    }


    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(85, 15), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    SearchResultTabWidget *srtw = w->findChild<SearchResultTabWidget *>(AC_searchResultTabWidget);
    if (srtw) {
        qApp->sendEvent(srtw, &event2);
    }

    // list排序
    QTest::qWait(50);
    for (auto item = actionList.begin(); item != actionList.end(); ++item) {
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
    }


    QMouseEvent event3(QEvent::MouseButtonPress, QPointF(160, 15), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    if (srtw) {
        qApp->sendEvent(srtw, &event3);
    }

    // list排序
    QTest::qWait(50);
    for (auto item = actionList.begin(); item != actionList.end(); ++item) {
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
        emit albumDropdown->triggered(*item);
        QTest::qWait(50);
    }

    QTest::qWait(100);
}

TEST(Application, searchKey)
{
    TEST_CASE_NAME("searchKey")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 歌曲搜索
    QLineEdit *se = w->findChild<QLineEdit *>(AC_Search);
    se->clear();

    // 点击所有音乐
    QTest::qWait(50);
    QPoint pos1(130, 100);
    QTestEventList event;
    event.addMouseMove(pos1);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 10);
    event.simulate(baseListView->viewport());
    event.clear();


    QTest::qWait(50);
    QPoint pos(100, 20);
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    event1.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 100);
    event1.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event1.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 100);
    event1.simulate(se);
    event1.clear();


    // 点击所有音乐
    QTest::qWait(100);
    QTestEventList event2;
    event2.addMouseMove(pos1);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event2.simulate(baseListView->viewport());
    event2.clear();

    // 歌手搜索
    QTest::qWait(50);
    QTestEventList event3;
    event3.addMouseMove(pos);
    event3.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    se->clear();
    event3.addKeyPress(Qt::Key::Key_D, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Up, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event3.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 50);
    event3.simulate(se);
    event3.clear();


    // 点击所有音乐
    QTest::qWait(100);
    QTestEventList event4;
    event4.addMouseMove(pos1);
    event4.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event4.simulate(baseListView->viewport());
    event4.clear();

    // 专辑搜索
    QTest::qWait(50);
    QTestEventList event5;
    event5.addMouseMove(pos);
    event5.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    se->clear();
    event5.addKeyPress(Qt::Key::Key_D, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 50);
    event5.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 50);
    event5.simulate(se);
    event5.clear();



    QTest::qWait(100);
}

TEST(Application, searchMouse)
{
    TEST_CASE_NAME("searchMouse")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 歌曲搜索
    QLineEdit *se = w->findChild<QLineEdit *>(AC_Search);
    se->clear();

    // 点击所有音乐
    QTest::qWait(50);
    QPoint pos1(130, 100);
    QTestEventList event;
    event.addMouseMove(pos1);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    QTest::qWait(50);

    QPoint pos(100, 20);
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event1.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 500);
    event1.simulate(se);
    event1.clear();


    QTest::qWait(50);
    MusicSearchListview *mslv = w->findChild<MusicSearchListview *>(AC_musicView);
    QTestEventList event2;
    QPoint pos2 = QPoint(100, 20);
    event2.addMouseMove(pos2);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos2, 500);
    event2.simulate(mslv->viewport());
    event2.clear();


    // 点击所有音乐
    QTest::qWait(100);
    QTestEventList event3;
    event3.addMouseMove(pos1);
    event3.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event3.simulate(baseListView->viewport());
    event3.clear();

    // 歌手搜索
    QTest::qWait(50);
    QTestEventList event4;
    event4.addMouseMove(pos);
    event4.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    se->clear();
    event4.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 100);
    event4.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 100);
    event4.simulate(se);
    event4.clear();


    QTest::qWait(100);
    MusicSearchListview *mslv1 = w->findChild<MusicSearchListview *>(AC_singerView);
    QTestEventList event5;
    pos = QPoint(100, 30);
    event5.addMouseMove(pos);
    event5.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event5.simulate(mslv1->viewport());
    event5.clear();

    // 点击所有音乐
    QTest::qWait(100);
    QTestEventList event6;
    event6.addMouseMove(pos1);
    event6.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 10);
    event6.simulate(baseListView->viewport());
    event6.clear();


    // 专辑搜索
    QTest::qWait(50);
    QTestEventList event7;
    event7.addMouseMove(pos);
    event7.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 50);
    se->clear();
    event7.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 100);
    event7.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 100);
    event7.simulate(se);
    event7.clear();


    QTest::qWait(50);
    MusicSearchListview *mslv2 = w->findChild<MusicSearchListview *>(AC_albumView);
    QTestEventList event9;
    pos = QPoint(100, 27);
    event9.addMouseMove(pos);
    event9.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event9.simulate(mslv2->viewport());
    event9.clear();
    QTest::qWait(500);


    // 点击所有音乐
    QTest::qWait(100);
    QTestEventList event10;
    event10.addMouseMove(pos1);
    event10.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 10);
    event10.simulate(baseListView->viewport());
    event10.clear();
}

TEST(Application, viewChangedDark)
{
    TEST_CASE_NAME("viewChangedDark")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::DarkType);
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(500);

    // 关闭歌曲信息
    QTimer::singleShot(200, w, [ = ]() {
        QTest::qWait(50);
        InfoDialog *infoDialog = w->findChild<InfoDialog *>("infoDialog");
        if (infoDialog) {
            infoDialog->close();
        }
    });

    // 歌曲信息
    QTestEventList event;
    QTest::qWait(50);
    event.addKeyClick(Qt::Key_I, Qt::ControlModifier, 10);
    event.simulate(w);
    event.clear();

    // 点击专辑
    QPoint pos(130, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    QTest::qWait(500);
    // 专辑 点击切换到icon模式
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    // 点击歌手
    QTest::qWait(500);
    pos.setY(60);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    QTest::qWait(500);
    // 歌手 点击切换到icon模式
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    // 点击所有音乐
    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();
    // 点击我的收藏
    QTest::qWait(500);
    pos.setY(130);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    DIconButton *btPlaylist = w->findChild<DIconButton *>(AC_PlayQueue);
    QTest::qWait(500);

    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(btPlaylist);
    event.clear();

    QTest::qWait(500);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::LightType);
    QTest::qWait(500);
}

TEST(Application, dequalizerDialog)
{
    TEST_CASE_NAME("dequalizerDialog")

    MainFrame *w = Application::getInstance()->getMainWindow();
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::DarkType);
    QTest::qWait(50);

    QAction *act = w->findChild<QAction *>(AC_equalizerAction);
    DMenu *menu = w->findChild<DMenu *>(AC_titleMenu);

    QTimer::singleShot(1500, w, [ = ]() {
        DSettingsDialog *dd = w->findChild<DSettingsDialog *>(AC_Dequalizer);

        // 开关
        DSwitchButton *swbt = w->findChild<DSwitchButton *>(AC_switchBtn);
        QTest::qWait(50);
        emit swbt->checkedChanged(false);
        QTest::qWait(50);
        emit swbt->checkedChanged(true);


        // pre
        QTest::qWait(50);
        DSlider *slider_pre = w->findChild<DSlider *>(AC_baud_pre);
        slider_pre->setValue(-10);
        QTest::qWait(50);
        slider_pre->setValue(10);

        // 60
        QTest::qWait(50);
        DSlider *slider_60 = w->findChild<DSlider *>(AC_slider_60);
        slider_60->setValue(-10);
        QTest::qWait(50);
        slider_60->setValue(10);

        // 170
        QTest::qWait(50);
        DSlider *slider_170 = w->findChild<DSlider *>(AC_slider_170);
        slider_170->setValue(-10);
        QTest::qWait(50);
        slider_170->setValue(10);

        // 310
        QTest::qWait(50);
        DSlider *slider_310 = w->findChild<DSlider *>(AC_slider_310);
        slider_310->setValue(-10);
        QTest::qWait(50);
        slider_310->setValue(10);

        // 600
        QTest::qWait(50);
        DSlider *slider_600 = w->findChild<DSlider *>(AC_slider_600);
        slider_600->setValue(-10);
        QTest::qWait(50);
        slider_600->setValue(10);

        // 1K
        QTest::qWait(50);
        DSlider *slider_1K = w->findChild<DSlider *>(AC_slider_1K);
        slider_1K->setValue(-10);
        QTest::qWait(50);
        slider_1K->setValue(10);

        // 3K
        QTest::qWait(50);
        DSlider *slider_3K = w->findChild<DSlider *>(AC_slider_3K);
        slider_3K->setValue(-10);
        QTest::qWait(50);
        slider_3K->setValue(10);

        // 6K
        QTest::qWait(50);
        DSlider *slider_6K = w->findChild<DSlider *>(AC_slider_6K);
        slider_6K->setValue(-10);
        QTest::qWait(50);
        slider_6K->setValue(10);

        // 12K
        QTest::qWait(50);
        DSlider *slider_12K = w->findChild<DSlider *>(AC_slider_12K);
        slider_12K->setValue(-10);
        QTest::qWait(50);
        slider_12K->setValue(10);

        // 14K
        QTest::qWait(50);
        DSlider *slider_14K = w->findChild<DSlider *>(AC_slider_14K);
        slider_14K->setValue(-10);
        QTest::qWait(50);
        slider_14K->setValue(10);

        // 16K
        QTest::qWait(50);
        DSlider *slider_16K = w->findChild<DSlider *>(AC_slider_16K);
        slider_16K->setValue(-10);
        QTest::qWait(50);
        slider_16K->setValue(10);

        // 保存均衡器配置
        QTest::qWait(50);
        DPushButton *push_save = w->findChild<DPushButton *>(AC_saveBtn);
        QTestEventList event;
        event.addMouseMove(QPoint(20, 20), 50);
        event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 20), 50);
        event.simulate(push_save);
        event.clear();
        QTest::qWait(300);

        // 选择均衡器模式
        QTest::qWait(50);
        DComboBox *cbb = w->findChild<DComboBox *>(AC_effectCombox);
        cbb->setCurrentIndex(0);
        QTest::qWait(150);
        cbb->setCurrentIndex(4);
        QTest::qWait(150);

        // 默认均衡器配置
        QTest::qWait(200);
        DPushButton *push_default = w->findChild<DPushButton *>(AC_Restore);
        event.addMouseMove(QPoint(20, 20), 50);
        event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 20), 50);
        event.simulate(push_default);
        event.clear();

        QTest::qWait(300);
        dd->close();
    });

    emit menu->triggered(act);
    QTest::qWait(500);
}

TEST(Application, settings)
{
    TEST_CASE_NAME("settings")
    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(50);

    QAction *act = w->findChild<QAction *>(AC_settingsAction);
    DMenu *menu = w->findChild<DMenu *>(AC_titleMenu);

    // 设置窗体
    QTimer::singleShot(1000, w, [ = ]() {
        DSettingsDialog *setting = w->findChild<DSettingsDialog *>(AC_configDialog);

        QTestEventList event;
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 50);
        event.simulate(setting);
        event.clear();

        MusicSettings::setOption("base.close.close_action", 2);

        QTest::qWait(100);
        setting->close();
        QTest::qWait(100);

        QTimer::singleShot(200, w, [ = ]() {
            CloseConfirmDialog *ccd = w->findChild<CloseConfirmDialog *>(AC_CloseConfirmDialog);
            if (ccd) {
                ccd->close();
            }
        });

        w->close();
    });

    emit menu->triggered(act);
    QTest::qWait(500);
}

TEST(Application, dDropdown)
{
    TEST_CASE_NAME("dDropdown")
    MainFrame *w = Application::getInstance()->getMainWindow();


    DDropdown *act = w->findChild<DDropdown *>(AC_albumDropdown);
    QTestEventList event;
    QPoint pos(20, 20);
    event.addMouseMove(pos);
    event.addMouseMove(QPoint(-10, -10));
    event.simulate(act);
    event.clear();
}

TEST(Application, musicListDataWidgetDropEvent)
{
    TEST_CASE_NAME("musicListDataWidgetDropEvent")
    MainFrame *w = Application::getInstance()->getMainWindow();

    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    MusicListDataWidget *mldw = w->findChild<MusicListDataWidget *>(AC_MusicListDataWidget);

    // 双击list
    QPoint pos = QPoint(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(plv->viewport());
    event.clear();

    // musicListDataWidget DropEvent
    QMimeData mimedata;
    QList<QUrl> li;
    QString lastImportPath =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    lastImportPath += "/歌曲/004.mp3";
    li.append(QUrl(lastImportPath));
    mimedata.setUrls(li);

    pos = QPoint(130, 130);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(mldw, &e);

    QTest::qWait(100);
}

// 平板相关
TEST(Application, tabletallMusic)
{
    TEST_CASE_NAME("tabletallMusic")

    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    DToolButton *iconListBtn = w->findChild<DToolButton *>(AC_btlistMode);

    QPoint pos(130, 20);
    QTestEventList event;
    // 点击所有音乐
    QTest::qWait(50);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // icon模式
    QTest::qWait(50);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconModeBtn);
    event.clear();

    QTest::qWait(50);
    // 点击切换回list模式
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(iconListBtn);
    event.clear();

    // 点击所有音乐
    QTest::qWait(50);
    pos = QPoint(130, 100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    // 切换选择模式
    emit CommonService::getInstance()->signalSelectMode(CommonService::MultSelect);
    // 全选
    emit CommonService::getInstance()->signalSelectAll();
    // 横屏
    emit CommonService::getInstance()->signalHScreen(false);
    emit CommonService::getInstance()->signalHScreen(true);

    CommonService::getInstance()->setIsTabletEnvironment(false);
    QTest::qWait(50);
}

// 添加到播放队列等
TEST(Application, tabletAddToSonglist)
{
    TEST_CASE_NAME("tabletAddToSonglist")

    CommonService::getInstance()->setIsTabletEnvironment(true);

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    QPoint pos(130, 20);
    QTestEventList event;
    // 点击所有音乐
    QTest::qWait(50);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 10);
    event.simulate(baseListView->viewport());
    event.clear();

    QTest::qWait(200);
    PlayListView *mliv = w->findChild<PlayListView *>(AC_PlayListView);

    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(mliv->viewport());
    event.clear();
    QTest::qWait(100);

    mliv->slotUpdatePlayingIcon();
    QModelIndexList list = mliv->tabletSelectedIndexes();

    // 添加到歌单
    QTimer::singleShot(300, w, [ = ]() {
        SongListView *songlist = w->findChild<SongListView *>(AC_tablet_songListView);
        if (songlist) {
            songlist->setThemeType(0);
            songlist->setThemeType(1);
            QPoint pos(50, 20);
            QTestEventList event;
            event.addMouseMove(pos);
            event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
            event.simulate(songlist->viewport());
            event.clear();
        }
        QTest::qWait(100);
    });

    mliv->slotShowSongList();

    CommonService::getInstance()->setIsTabletEnvironment(false);
    QTest::qWait(50);
}

TEST(Application, end)
{
    TEST_CASE_NAME("end")
    QTest::qWait(500);
}







