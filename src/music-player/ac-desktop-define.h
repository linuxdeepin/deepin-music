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

#ifndef DESKTOP_ACCESSIBLE_UI_DEFINE_H
#define DESKTOP_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>

//#define SELECT_CASE "musicListDialg5"

#ifdef SELECT_CASE
#define TEST_CASE_NAME(testName) if(testName != SELECT_CASE) return;
#else
#define TEST_CASE_NAME(testName)
#endif


#ifdef ENABLE_ACCESSIBILITY
#define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename) classObj->setAccessibleName(accessiblename);
#else
#define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename)
#endif

#define AC_SET_OBJECT_NAME(classObj,objectname) classObj->setObjectName(objectname);

//#define AC_forwardWidget    QObject::tr("forwardWidget")
//#define AC_downWidget       QObject::tr("downWidget")
#define AC_btCover          "btCover"
#define AC_Play             "Play"
#define AC_Prev             "Prev"
#define AC_Next             "Next"
#define AC_Waveform         "Waveform"
#define AC_Favorite         "Favorite"
#define AC_Lyric            "Lyric"
#define AC_PlayMode         "PlayMode"
#define AC_Sound            "Sound"
#define AC_VolSlider        "volSlider"
#define AC_DSlider          "DSlider"
#define AC_PlayQueue        "PlayQueue"
//#define AC_ctlWidget        QObject::tr("ctlWidget")
//#define AC_actWidget        QObject::tr("actWidget")
#define AC_ImportWidget     "ImportWidget"
#define AC_importButton     "importButton"
#define AC_importLinkText   "importLinkText"
#define AC_addMusicButton   "addMusicButton"
//#define AC_centralWidget    QObject::tr("centralWidget")
//#define AC_MainFrame                  QObject::tr("MainFrame")
#define AC_MusicListDataWidget        "MusicListDataWidget"
//#define AC_actionBar                  QObject::tr("actionBar")
#define AC_albumDropdown              "albumDropdown"
#define AC_artistDropdown             "artistDropdown"
#define AC_musicDropdown              "musicDropdown"
//#define AC_btPlayAll                  QObject::tr("btPlayAll")
#define AC_btIconMode                 "btIconMode"
#define AC_btlistMode                 "btlistMode"
#define AC_btSort                     "btSort"
#define AC_MusicListScrollArea        "MusicListScrollArea"
//#define AC_libraryWidget              QObject::tr("libraryWidget")
//#define AC_dataBaseLabel              QObject::tr("dataBaseLabel")
//#define AC_customizeLabel             QObject::tr("customizeLabel")
//#define AC_addListBtn                 QObject::tr("addListBtn")
#define AC_dataBaseListview           "dataBaseListview"
#define AC_customizeListview          "customizeListview"
//#define AC_MusicListWidget            QObject::tr("MusicListWidget")
//#define AC_btClearAll                 QObject::tr("btClearAll")
#define AC_CloseConfirmDialog         QObject::tr("CloseConfirmDialog")
#define AC_switchBtn                  "switchBtn"
#define AC_effectCombox               "effectCombox"
#define AC_saveBtn                    "saveBtn"
#define AC_baud_pre                   "baud_pre"
#define AC_slider_60                  "baud_60"
#define AC_slider_170                 "baud_170"
#define AC_slider_310                 "baud_310"
#define AC_slider_600                 "baud_600"
#define AC_slider_1K                  "baud_1K"
#define AC_slider_3K                  "baud_3K"
#define AC_slider_6K                  "baud_6K"
#define AC_slider_12K                 "baud_12K"
#define AC_slider_14K                 "baud_14K"
#define AC_slider_16K                 "baud_16K"
#define AC_Restore                    "Restore"
#define AC_slWidget                   "slWidget"
#define AC_mequalizer                 "mequalizer"
#define AC_mtabwidget                 "mtabwidget"
#define AC_mTitlebar                  "mTitlebar"
#define AC_Dequalizer                 "Dequalizer"
#define AC_PlayListView               "playListView"
#define AC_albumListView              "albumListView"
#define AC_singerListView             "singerListView"
#define AC_musicListInfoView          "musicListInfoView"
#define AC_Search                     "search"
#define AC_musicLyricWidget           "musicLyricWidget"
#define AC_lyricview                  "lyricview"
#define AC_musicView                  "musicView"
#define AC_singerView                 "singerView"
#define AC_albumView                  "albumtView"
#define AC_musicListDialogSinger      "musicListDialogSinger"
#define AC_musicListDialogAlbum       "musicListDialogAlbum"
#define AC_titleMenu                  "titleMenu"
#define AC_equalizerAction            "equalizerAction"
#define AC_settingsAction             "settingsAction"
#define AC_configDialog               "configDialog"
#define AC_infoDialog                 "infoDialog"
#define AC_musicListDialogCloseBt     "musicListDialogCloseBt"
#define AC_MessageBox                 "messageBox"
//#define AC_FileDialog                 QObject::tr("fileDialog")
#define AC_searchResultTabWidget      "searchResultTabWidget"
#define AC_dialogPlayAll              "dialogPlayAll"
#define AC_dialogPlayRandom           "dialogPlayRandom"
#define AC_subSonglistWidget          "subSonglistWidget"
#define AC_titleBarLeft               "titleBarLeft"
#define AC_subSonglistWidget          "subSonglistWidget"
#define AC_Shortcut_Escape            "shortcut_Escape"
#define AC_tablet_songListViewDlg     "tabletSongListViewDialog"
#define AC_tablet_songListView        "tabletSongListView"
#define AC_tablet_title_select        "tablet_title_select"
#define AC_musicListDataPlayAll       "MusicListDataPlayAll"

#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
