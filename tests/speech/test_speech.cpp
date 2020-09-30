#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <DUtil>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DDialog>
#include <DApplication>
#include <DTitlebar>
#include <DImageButton>
#include <DFileDialog>
#include <DHiDPIHelper>

#include "speechCenter.h"
#include "exportedinterface.h"

DWIDGET_USE_NAMESPACE

static QString musicfile1 = "/usr/share/music/bensound-sunny.mp3";

TEST(speech, test)
{
    QString artist = "unknown";
    SpeechCenter *speech = SpeechCenter::getInstance();
    speech->playMusic(musicfile1);
    speech->playArtist(artist);
    speech->playArtistMusic(artist, musicfile1);
    speech->playFaverite();
    speech->playCustom("all");
    speech->playRadom();
    speech->pause();
    speech->resume();
    speech->previous();
    speech->next();
    speech->favorite();
//    speech->unFaverite();
    speech->setMode(1);
    for (int i = 1 ; i < 8; i++) {
        speech->onSpeedResult(i, false);
    }
    ASSERT_TRUE(speech->stop());
}

TEST(test_speech, manager)
{
    ExportedInterface *ex = new ExportedInterface;
    ex->registerAction("1", "playmusic");
    ex->registerAction("2", "play artist");
    ex->registerAction("3", "play artist song");
    ex->registerAction("4", "play faverite");
    ex->registerAction("5", "play custom ");
    ex->registerAction("6", "play radom");
    ex->registerAction("11", "pause");
    ex->registerAction("12", "stop");
    ex->registerAction("13", "resume");
    ex->registerAction("14", "previous");
    ex->registerAction("15", "next");
    ex->registerAction("21", "faverite");
    ex->registerAction("22", "unfaverite");
    ex->registerAction("23", "set play mode");
    QString json = "{"
                   "\"groups\": ["
                   "{"
                   "\"key\": \"base\","
                   "\"name\": \"Basic\","
                   "\"groups\": ["
                   "{"
                   "\"key\": \"close\","
                   "\"name\": \"Close Main Window\","
                   "\"hide\": false,"
                   "\"options\": ["
                   "{"
                   "\"key\": \"close_action\","
                   "\"name\": \" \","
                   "\"type\": \"radiogroup\","
                   "\"items\": ["
                   "\"Minimize to system tray\","
                   "\"Exit Music\""
                   "],"
                   "\"default\": 0"
                   "}"
                   "]"
                   "}"
                   "]"
                   "}"
                   "]"
                   "}";

    for (int i = 1; i < 24; i++) {
        if (i == 22)
            continue;
        QString action = QString::number(i);
        ex->invoke(action, json);
    }
}
