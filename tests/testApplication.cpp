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
#include "widget/musicimagebutton.h"
#include "widget/titlebarwidget.h"
#include "widget/waveformscale.h"
//#include "../music-player/view/footerwidget.h"
#include "playlistwidget.h"
#include "musiclyricwidget.h"
#include "medialibrary.h"
#include "mediameta.h"
#include "metadetector.h"
#include "net/geese.h"
#include "util/basetool.h"
#include "util/cueparser.h"
#include "util/encodingdetector.h"
#include "util/pinyinsearch.h"
#include "metabufferdetector.h"
#include "player.h"
#include "fft.h"
#include "inotifyfiles.h"
#include "musiclyric.h"
#include "lyriclabel.h"

#include <vlc/vlc.h>
#include "vlc/Audio.h"
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
#include "speech/exportedinterface.h"
#include "threadpool.h"
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
#include "musiclistinfoview.h"
#include "musiclistdialog.h"
#include "dequalizerdialog.h"

TEST(Application, deleteAllMusic)
{
//    // 删除所有音乐
//    TEST_CASE_NAME("deleteAllMusic")


//    QTest::qWait(500);
//    MainFrame *w = Application::getInstance()->getMainWindow();
//    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);


//    // 点击所有音乐
//    QPoint pos = QPoint(130, 100);
//    QTestEventList event;
//    event.addMouseMove(pos);
//    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
//    event.simulate(baseListView->viewport());
//    event.clear();


//    // 双击list
//    pos = QPoint(20, 20);
//    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
//    event.addMouseMove(pos);
//    event.addKeyClick(Qt::Key_A, Qt::ControlModifier, 100);
//    event.simulate(plv->viewport());
//    event.clear();

//    // 清空ListView
//    plv->slotRmvFromSongList();

//    QTest::qWait(500);
}


// 文件夹拷贝
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

TEST(Application, copyMusicToMusicDir)
{
    // 拷贝音乐文件夹到系统音乐文件夹下
    TEST_CASE_NAME("copyMusicToMusicDir")

    QStringList stringList = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    if (stringList.size() > 0) {
        stringList[0].append("/歌曲");
        copyDirFiles("/home/helin/Desktop/歌曲", stringList[0]);
    }
}

TEST(Application, importLinkText)
{
    // 扫描歌曲
    TEST_CASE_NAME("importLinkText")

    QTest::qWait(1000);
    MainFrame *w = Application::getInstance()->getMainWindow();
    QLabel *ilt = w->findChild<QLabel *>(AC_importLinkText);
    ilt->linkActivated("");
    QTest::qWait(1000);
}

TEST(Application, musicListDialog)
{
    TEST_CASE_NAME("musicListDialog")

    QTest::qWait(1000);
    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);


    QTest::qWait(500);
    // 点击专辑
    QPoint pos = QPoint(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    // dialog list 点击
    QTest::qWait(500);
    QTimer::singleShot(1500, w, [ = ]() {
        MusicListInfoView *mliv = w->findChild<MusicListInfoView *>(AC_musicListInfoView);
        MusicListDialog *mld = w->findChild<MusicListDialog *>(AC_musicListDialogAlbum);
        mld->setThemeType(0);

        MusicImageButton *closeBt = w->findChild<MusicImageButton *>(AC_musicListDialogCloseBt);
        QTestEventList event;
        QPoint pos(20, 20);
        event.addMouseMove(pos);
        event.addMouseMove(QPoint(-10, -10));
        event.simulate(closeBt);
        event.clear();

        pos = QPoint(130, 20);
        event.addMouseMove(pos);
        event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
        event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
        event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
        event.simulate(mliv->viewport());
        event.clear();

        QTest::qWait(500);


        closeBt->setTransparent(0);
        closeBt->setTransparent(1);

        closeBt->setAutoChecked(0);
        closeBt->setAutoChecked(1);


        event.addMouseMove(QPoint(20, 20));
        event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 20), 100);
        event.simulate(closeBt);
        event.clear();
        mld->setThemeType(1);
    });

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(alv->viewport());
    event.clear();

    QTest::qWait(2000);
}

TEST(Application, musicListDialg_1)
{
    TEST_CASE_NAME("musicListDialg_1")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(500);
    // 点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();



    // dialog list 点击
    QTest::qWait(500);
    QTimer::singleShot(1500, w, [ = ]() {
        MusicListDialog *mld = w->findChild<MusicListDialog *>(AC_musicListDialogAlbum);
        MusicListInfoView *mliv = w->findChild<MusicListInfoView *>(AC_musicListInfoView);

        QPoint pos(130, 20);
        QTestEventList event;
        event.addMouseMove(pos);
        event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
        event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
        event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
        event.simulate(mliv->viewport());
        event.clear();


        QTest::qWait(500);
        event.addKeyPress(Qt::Key::Key_0, Qt::ShiftModifier, 100);
        event.simulate(mliv->viewport());
        event.clear();

        QTest::qWait(500);
        event.addKeyPress(Qt::Key::Key_0, Qt::ControlModifier, 100);
        event.simulate(mliv->viewport());
        event.clear();

        QTest::qWait(500);

        mld->close();
    });

    // 双击list
    pos = QPoint(20, 20);
    AlbumListView *alv = w->findChild<AlbumListView *>(AC_albumListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(alv->viewport());
    event.clear();
    QTest::qWait(2000);
}

TEST(Application, viewChanged)
{
    TEST_CASE_NAME("viewChanged")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);

    // 点击专辑
    QTest::qWait(500);
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(500);
    DDropdown *albumDropdown = w->findChild<DDropdown *>(AC_albumDropdown);
    QList<QAction *> actionList = albumDropdown->actions();
    for (auto item = actionList.begin(); item != actionList.end(); ++item) {
        emit albumDropdown->triggered(*item);
        albumDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }

    // icon模式
    QTest::qWait(500);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = actionList.begin(); item != actionList.end(); ++item) {
        emit albumDropdown->triggered(*item);
        albumDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }

    // 点击歌手
    QTest::qWait(500);
    pos.setY(60);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(500);
    DDropdown *artistDropdown = w->findChild<DDropdown *>(AC_artistDropdown);
    QList<QAction *> artistList = artistDropdown->actions();
    for (auto item = artistList.begin(); item != artistList.end(); ++item) {
        emit artistDropdown->triggered(*item);
        artistDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }

    // icon模式
    QTest::qWait(500);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = artistList.begin(); item != artistList.end(); ++item) {
        emit artistDropdown->triggered(*item);
        artistDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }

    //点击所有音乐
    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(500);
    DDropdown *allMusicDropdown = w->findChild<DDropdown *>(AC_musicDropdown);
    QList<QAction *> allMusicList = allMusicDropdown->actions();
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        allMusicDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }

    // icon模式
    QTest::qWait(500);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        allMusicDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }

    //点击我的收藏
    QTest::qWait(500);
    pos.setY(130);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    // list排序
    QTest::qWait(500);
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        allMusicDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }

    // icon模式
    QTest::qWait(500);
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    // icon排序
    for (auto item = allMusicList.begin(); item != allMusicList.end(); ++item) {
        emit allMusicDropdown->triggered(*item);
        allMusicDropdown->setCurrentAction(*item);
        QTest::qWait(500);
    }
}

TEST(Application, customViewChanged)
{
    TEST_CASE_NAME("customViewChanged")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicSongListView *songListView = w->findChild<MusicSongListView *>(AC_customizeListview);
    QTest::qWait(500);
    //点击自定义列表
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(songListView->viewport());
    event.clear();

    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(500);
    //点击所有歌曲
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();
}

TEST(Application, iconModeChanged)
{
    TEST_CASE_NAME("iconModeChanged")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    QTest::qWait(500);
    //点击切换到icon模式
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    QTest::qWait(500);
    //点击切换到list模式
    DToolButton *iconListBtn = w->findChild<DToolButton *>(AC_btlistMode);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconListBtn);
    event.clear();


    QTest::qWait(500);
    // 双击list
    pos = QPoint(20, 120);
    PlayListView *plv = w->findChild<PlayListView *>(AC_PlayListView);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMousePress(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.addMouseDClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(plv->viewport());
    event.clear();
}

TEST(Application, preMusic)
{
    TEST_CASE_NAME("preMusic")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DToolButton *preBtn = w->findChild<DToolButton *>(AC_Prev);
    QTest::qWait(500);
    // 点击上一首
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(preBtn);
    event.clear();

    DToolButton *playBtn = w->findChild<DToolButton *>(AC_Play);
    QTest::qWait(500);
    // 点击播放
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event1.simulate(playBtn);
    event1.clear();

    DToolButton *nextBtn = w->findChild<DToolButton *>(AC_Next);
    QTest::qWait(1500);
    // 点击下一首
    QTestEventList event2;
    event2.addMouseMove(pos);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event2.simulate(nextBtn);
    event2.clear();
}

TEST(Application, btCover)
{
    TEST_CASE_NAME("btCover")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicPixmapButton *mpb = w->findChild<MusicPixmapButton *>(AC_btCover);
    QTest::qWait(500);
    // 点击头像按钮
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(mpb);
    event.clear();
}

TEST(Application, waveform)
{
    TEST_CASE_NAME("waveform")

    MainFrame *w = Application::getInstance()->getMainWindow();
    Waveform *wf = w->findChild<Waveform *>(AC_Waveform);
    QTest::qWait(500);
    // 点击进度条
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseMove(pos);

    pos = QPoint(200, 20);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseMove(pos);

    pos = QPoint(300, 20);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseMove(pos);
    event.simulate(wf);
    event.clear();
}

TEST(Application, btFavorite)
{
    TEST_CASE_NAME("btFavorite")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_Favorite);
    QTest::qWait(500);
    // 点击进收藏按钮
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(bt);
    event.clear();
}

TEST(Application, btLyric)
{
    TEST_CASE_NAME("btLyric")

    MainFrame *w = Application::getInstance()->getMainWindow();

    DIconButton *bt = w->findChild<DIconButton *>(AC_Lyric);
    QTest::qWait(500);
    // 点击歌词按钮
    QPoint pos = QPoint(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(bt);
    event.clear();

    QTest::qWait(500);

    // 滚动歌词
    AbstractWheelWidget *lv = w->findChild<AbstractWheelWidget *>(AC_lyricview);

    QTestEventList event1;
    pos = QPoint(280, 440);
    QWheelEvent wheel(pos, -120, Qt::MouseButton::MidButton, Qt::NoModifier);
    lv->event(&wheel);

    QWheelEvent wheel1(pos, -240, Qt::MouseButton::MidButton, Qt::NoModifier);
    lv->event(&wheel1);

    QEvent seroll(QEvent::Scroll);
    lv->event(&seroll);


    MusicLyricWidget *mlw = w->findChild<MusicLyricWidget *>(AC_musicLyricWidget);
    mlw->updateUI();
    mlw->slotTheme(0);
    mlw->slotTheme(1);


    // 点击歌词按钮
    QTest::qWait(1000);
    QTestEventList event2;
    event2.addMouseMove(QPoint(20, 20));
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, QPoint(20, 20), 500);
    event2.simulate(bt);
    event2.clear();
    QTest::qWait(500);
}

TEST(Application, playMode)
{
    TEST_CASE_NAME("playMode")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_PlayMode);
    QTest::qWait(500);
    // 点击播放模式
    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(bt);
    event.clear();
}

TEST(Application, btSound)
{
    TEST_CASE_NAME("btSound")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_Sound);
    QTest::qWait(500);

    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(bt);
    event.clear();

    SoundVolume *sv = w->findChild<SoundVolume *>(AC_VolSlider);
    QTestEventList event1;
    pos = QPoint(30, 60);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 1000);
    event1.addMouseMove(pos);

    pos = QPoint(30, 120);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 1000);
    event1.addMouseMove(pos);
    event1.simulate(sv);
    event1.clear();


    DSlider *ds = w->findChild<DSlider *>(AC_DSlider);
    QTestEventList event2;
    pos = QPoint(10, 10);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 1000);
    event2.addMouseMove(pos);

    pos = QPoint(10, 40);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 1000);
    event2.addMouseMove(pos);
    event2.simulate(ds);
    event2.clear();


    pos = QPoint(20, 20);
    QTestEventList event3;
    event3.addMouseMove(pos);
    event3.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event3.simulate(bt);
    event3.clear();
}

TEST(Application, btPlayList)
{
    TEST_CASE_NAME("btPlayList")

    MainFrame *w = Application::getInstance()->getMainWindow();
    DIconButton *bt = w->findChild<DIconButton *>(AC_PlayList);
    QTest::qWait(500);

    QPoint pos(20, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(bt);
    event.clear();
}

TEST(Application, searchKey)
{
    TEST_CASE_NAME("searchKey")

    MainFrame *w = Application::getInstance()->getMainWindow();
    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);

    // 歌曲搜索
    QLineEdit *se = w->findChild<QLineEdit *>(AC_Search);

    // 点击所有音乐
    QTest::qWait(500);
    QPoint pos1(130, 100);
    QTestEventList event;
    event.addMouseMove(pos1);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event.simulate(baseListView->viewport());
    event.clear();


    QTest::qWait(500);
    QPoint pos(100, 20);
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event1.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 500);
    event1.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 500);
    event1.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 500);
    event1.simulate(se);
    event1.clear();


    // 点击所有音乐
    QTest::qWait(1000);
    QTestEventList event2;
    event2.addMouseMove(pos1);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event2.simulate(baseListView->viewport());
    event2.clear();

    // 歌手搜索
    QTest::qWait(500);
    QTestEventList event3;
    event3.addMouseMove(pos);
    event3.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    se->clear();
    event3.addKeyPress(Qt::Key::Key_D, Qt::NoModifier, 500);
    event3.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 500);
    event3.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 500);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event3.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event3.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 500);
    event3.simulate(se);
    event3.clear();


    // 点击所有音乐
    QTest::qWait(1000);
    QTestEventList event4;
    event4.addMouseMove(pos1);
    event4.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event4.simulate(baseListView->viewport());
    event4.clear();

    // 专辑搜索
    QTest::qWait(500);
    QTestEventList event5;
    event5.addMouseMove(pos);
    event5.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    se->clear();
    event5.addKeyPress(Qt::Key::Key_D, Qt::NoModifier, 500);
    event5.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 500);
    event5.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 500);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Down, Qt::NoModifier, 100);
    event5.addKeyPress(Qt::Key::Key_Enter, Qt::NoModifier, 500);
    event5.simulate(se);
    event5.clear();
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
    QTest::qWait(500);
    QPoint pos1(130, 100);
    QTestEventList event;
    event.addMouseMove(pos1);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    QTest::qWait(500);

    QPoint pos(100, 20);
    QTestEventList event1;
    event1.addMouseMove(pos);
    event1.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event1.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 500);
    event1.simulate(se);
    event1.clear();


    QTest::qWait(500);
    MusicSearchListview *mslv = w->findChild<MusicSearchListview *>(AC_musicView);
    QTestEventList event6;
    QPoint pos2 = QPoint(100, 20);
    event6.addMouseMove(pos2);
    event6.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos2, 500);
    event6.simulate(mslv->viewport());
    event6.clear();




    // 点击所有音乐
    QTest::qWait(1000);
    QTestEventList event2;
    event2.addMouseMove(pos1);
    event2.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event2.simulate(baseListView->viewport());
    event2.clear();

    // 歌手搜索
    QTest::qWait(500);
    QTestEventList event3;
    event3.addMouseMove(pos);
    event3.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    se->clear();
    event3.addKeyPress(Qt::Key::Key_D, Qt::NoModifier, 500);
    event3.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 500);
    event3.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 500);
    event3.simulate(se);
    event3.clear();


    QTest::qWait(500);
    MusicSearchListview *mslv1 = w->findChild<MusicSearchListview *>(AC_singerView);
    QTestEventList event7;
    pos = QPoint(100, 100);
    event7.addMouseMove(pos);
    event7.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event7.simulate(mslv1->viewport());
    event7.clear();

    QTest::qWait(500);



    // 点击所有音乐
    QTest::qWait(1000);
    QTestEventList event8;
    event8.addMouseMove(pos1);
    event8.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos1, 100);
    event8.simulate(baseListView->viewport());
    event8.clear();

    // 歌手搜索
    QTest::qWait(500);
    QTestEventList event9;
    event9.addMouseMove(pos);
    event9.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    se->clear();
    event9.addKeyPress(Qt::Key::Key_D, Qt::NoModifier, 500);
    event9.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 500);
    event9.addKeyPress(Qt::Key::Key_A, Qt::NoModifier, 500);
    event9.simulate(se);
    event9.clear();


    QTest::qWait(500);
    MusicSearchListview *mslv2 = w->findChild<MusicSearchListview *>(AC_albumView);
    QTestEventList event10;
    pos = QPoint(100, 20);
    event10.addMouseMove(pos);
    event10.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event10.simulate(mslv2->viewport());
    event10.clear();

    QTest::qWait(500);
    QTestEventList event11;
    event11.addMouseMove(pos);
    event11.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event11.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 100);
    event11.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 100);
    event11.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 100);
    event11.addKeyPress(Qt::Key::Key_Backspace, Qt::NoModifier, 100);
    event11.simulate(se);
    event11.clear();
    QTest::qWait(500);
}

TEST(Application, viewChangedDark)
{
    TEST_CASE_NAME("viewChangedDark")

    MainFrame *w = Application::getInstance()->getMainWindow();
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::DarkType);
    QTest::qWait(500);

    MusicBaseListView *baseListView = w->findChild<MusicBaseListView *>(AC_dataBaseListview);
    QTest::qWait(500);
    //点击专辑
    QPoint pos(130, 20);
    QTestEventList event;
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    DToolButton *iconModeBtn = w->findChild<DToolButton *>(AC_btIconMode);
    QTest::qWait(500);
    //专辑 点击切换到icon模式
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    //点击歌手
    QTest::qWait(500);
    pos.setY(60);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();



    QTest::qWait(500);
    //歌手 点击切换到icon模式
    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(iconModeBtn);
    event.clear();

    //点击所有音乐
    QTest::qWait(500);
    pos.setY(100);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();
    //点击我的收藏
    QTest::qWait(500);
    pos.setY(130);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 100);
    event.simulate(baseListView->viewport());
    event.clear();

    DIconButton *btPlaylist = w->findChild<DIconButton *>(AC_PlayList);
    QTest::qWait(500);

    pos = QPoint(20, 20);
    event.addMouseMove(pos);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.addMouseClick(Qt::MouseButton::LeftButton, Qt::NoModifier, pos, 500);
    event.simulate(btPlaylist);
    event.clear();


    QTest::qWait(500);
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::LightType);
    QTest::qWait(500);
}

TEST(Application, dequalizerDialog)
{
    TEST_CASE_NAME("dequalizerDialog")

    MainFrame *w = Application::getInstance()->getMainWindow();
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::DarkType);
    QTest::qWait(500);

    QAction *act = w->findChild<QAction *>(AC_equalizerAction);
    DMenu *menu = w->findChild<DMenu *>(AC_titleMenu);


    QTimer::singleShot(1500, w, [ = ]() {
        DequalizerDialog *dd = w->findChild<DequalizerDialog *>(AC_Dequalizer);

        // 开关
        DSwitchButton *swbt = w->findChild<DSwitchButton *>(AC_switchBtn);
        QTest::qWait(500);
        emit swbt->checkedChanged(false);
        QTest::qWait(500);
        emit swbt->checkedChanged(true);


        // pre
        QTest::qWait(500);
        DSlider *slider_pre = w->findChild<DSlider *>(AC_baud_pre);
        slider_pre->setValue(-10);
        QTest::qWait(500);
        slider_pre->setValue(10);

        // 60
        QTest::qWait(500);
        DSlider *slider_60 = w->findChild<DSlider *>(AC_slider_60);
        slider_60->setValue(-10);
        QTest::qWait(500);
        slider_60->setValue(10);

        // 170
        QTest::qWait(500);
        DSlider *slider_170 = w->findChild<DSlider *>(AC_slider_170);
        slider_170->setValue(-10);
        QTest::qWait(500);
        slider_170->setValue(10);

        // 310
        QTest::qWait(500);
        DSlider *slider_310 = w->findChild<DSlider *>(AC_slider_310);
        slider_310->setValue(-10);
        QTest::qWait(500);
        slider_310->setValue(10);

        // 600
        QTest::qWait(500);
        DSlider *slider_600 = w->findChild<DSlider *>(AC_slider_600);
        slider_600->setValue(-10);
        QTest::qWait(500);
        slider_600->setValue(10);

        // 1K
        QTest::qWait(500);
        DSlider *slider_1K = w->findChild<DSlider *>(AC_slider_1K);
        slider_1K->setValue(-10);
        QTest::qWait(500);
        slider_1K->setValue(10);

        // 3K
        QTest::qWait(500);
        DSlider *slider_3K = w->findChild<DSlider *>(AC_slider_3K);
        slider_3K->setValue(-10);
        QTest::qWait(500);
        slider_3K->setValue(10);

        // 6K
        QTest::qWait(500);
        DSlider *slider_6K = w->findChild<DSlider *>(AC_slider_6K);
        slider_6K->setValue(-10);
        QTest::qWait(500);
        slider_6K->setValue(10);

        // 12K
        QTest::qWait(500);
        DSlider *slider_12K = w->findChild<DSlider *>(AC_slider_12K);
        slider_12K->setValue(-10);
        QTest::qWait(500);
        slider_12K->setValue(10);

        // 14K
        QTest::qWait(500);
        DSlider *slider_14K = w->findChild<DSlider *>(AC_slider_14K);
        slider_14K->setValue(-10);
        QTest::qWait(500);
        slider_14K->setValue(10);

        // 16K
        QTest::qWait(500);
        DSlider *slider_16K = w->findChild<DSlider *>(AC_slider_16K);
        slider_16K->setValue(-10);
        QTest::qWait(500);
        slider_16K->setValue(10);

        // combox
        QTest::qWait(500);
        DComboBox *cbb = w->findChild<DComboBox *>(AC_effectCombox);
        cbb->setCurrentIndex(0);
        QTest::qWait(500);
        cbb->setCurrentIndex(4);
        QTest::qWait(500);

        QTest::qWait(500);
        dd->close();
    });

    emit menu->triggered(act);
}

TEST(Application, settings)
{
    TEST_CASE_NAME("settings")
    MainFrame *w = Application::getInstance()->getMainWindow();

    QTest::qWait(500);

    QAction *act = w->findChild<QAction *>(AC_settingsAction);
    DMenu *menu = w->findChild<DMenu *>(AC_titleMenu);

    QTimer::singleShot(1000, w, [ = ]() {
        DSettingsDialog *setting = w->findChild<DSettingsDialog *>(AC_configDialog);

        QTest::qWait(500);
        setting->close();
    });

    emit menu->triggered(act);
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

TEST(Application, end)
{
    TEST_CASE_NAME("end")
    QTest::qWait(500);
}







