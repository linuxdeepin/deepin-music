/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * to mark the desktop UI
 **/

#ifndef DESKTOP_ACCESSIBLE_UI_DEFINE_H
#define DESKTOP_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>


//#define SELECT_CASE "other"

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

#define AC_Footer           QObject::tr("Footer")
#define AC_forwardWidget    QObject::tr("forwardWidget")
#define AC_downWidget       QObject::tr("downWidget")
#define AC_btCover          QObject::tr("btCover")
#define AC_Play             QObject::tr("Play")
#define AC_Prev             QObject::tr("Prev")
#define AC_Next             QObject::tr("Next")
#define AC_Waveform         QObject::tr("Waveform")
#define AC_Favorite         QObject::tr("Favorite")
#define AC_Lyric            QObject::tr("Lyric")
#define AC_PlayMode         QObject::tr("PlayMode")
#define AC_Sound            QObject::tr("Sound")
#define AC_VolSlider        QObject::tr("volSlider")
#define AC_DSlider          QObject::tr("DSlider")
#define AC_PlayList         QObject::tr("PlayList")
#define AC_ctlWidget        QObject::tr("ctlWidget")
#define AC_actWidget        QObject::tr("actWidget")
#define AC_ImportWidget     QObject::tr("ImportWidget")
#define AC_importButton     QObject::tr("importButton")
#define AC_importLinkText   QObject::tr("importLinkText")
#define AC_addMusicButton   QObject::tr("addMusicButton")
#define AC_centralWidget    QObject::tr("centralWidget")
#define AC_MainFrame                  QObject::tr("MainFrame")
#define AC_MusicListDataWidget        QObject::tr("MusicListDataWidget")
#define AC_actionBar                  QObject::tr("actionBar")
#define AC_albumDropdown              QObject::tr("albumDropdown")
#define AC_artistDropdown             QObject::tr("artistDropdown")
#define AC_musicDropdown              QObject::tr("musicDropdown")
#define AC_btPlayAll                  QObject::tr("btPlayAll")
#define AC_btIconMode                 QObject::tr("btIconMode")
#define AC_btlistMode                 QObject::tr("btlistMode")
#define AC_MusicListScrollArea        QObject::tr("MusicListScrollArea")
#define AC_libraryWidget              QObject::tr("libraryWidget")
#define AC_dataBaseLabel              QObject::tr("dataBaseLabel")
#define AC_customizeLabel             QObject::tr("customizeLabel")
#define AC_addListBtn                 QObject::tr("addListBtn")
#define AC_dataBaseListview           QObject::tr("dataBaseListview")
#define AC_customizeListview          QObject::tr("customizeListview")
#define AC_MusicListWidget            QObject::tr("MusicListWidget")
#define AC_btClearAll                 QObject::tr("btClearAll")
#define AC_CloseConfirmDialog         QObject::tr("CloseConfirmDialog")
#define AC_switchBtn                  QObject::tr("switchBtn")
#define AC_effectCombox               QObject::tr("effectCombox")
#define AC_saveBtn                    QObject::tr("saveBtn")
#define AC_baud_pre                   QObject::tr("baud_pre")
#define AC_slider_60                  QObject::tr("baud_60")
#define AC_slider_170                 QObject::tr("baud_170")
#define AC_slider_310                 QObject::tr("baud_310")
#define AC_slider_600                 QObject::tr("baud_600")
#define AC_slider_1K                  QObject::tr("baud_1K")
#define AC_slider_3K                  QObject::tr("baud_3K")
#define AC_slider_6K                  QObject::tr("baud_6K")
#define AC_slider_12K                 QObject::tr("baud_12K")
#define AC_slider_14K                 QObject::tr("baud_14K")
#define AC_slider_16K                 QObject::tr("baud_16K")
#define AC_Restore                    QObject::tr("Restore")
#define AC_slWidget                   QObject::tr("slWidget")
#define AC_mequalizer                 QObject::tr("mequalizer")
#define AC_mtabwidget                 QObject::tr("mtabwidget")
#define AC_mTitlebar                  QObject::tr("mTitlebar")
#define AC_Dequalizer                 QObject::tr("Dequalizer")
#define AC_PlayListView               QObject::tr("playListView")
#define AC_albumListView              QObject::tr("albumListView")
#define AC_singerListView             QObject::tr("singerListView")
#define AC_musicListInfoView          QObject::tr("musicListInfoView")
#define AC_Search                     QObject::tr("search")
#define AC_musicLyricWidget           QObject::tr("musicLyricWidget")
#define AC_lyricview                  QObject::tr("lyricview")
#define AC_musicView                  QObject::tr("musicView")
#define AC_singerView                 QObject::tr("singerView")
#define AC_albumView                  QObject::tr("albumtView")
#define AC_musicListDialogSinger      QObject::tr("musicListDialogSinger")
#define AC_musicListDialogAlbum       QObject::tr("musicListDialogAlbum")
#define AC_titleMenu                  QObject::tr("titleMenu")
#define AC_equalizerAction            QObject::tr("equalizerAction")
#define AC_settingsAction             QObject::tr("settingsAction")
#define AC_configDialog               QObject::tr("configDialog")
#define AC_infoDialog                 QObject::tr("infoDialog")
#define AC_musicListDialogCloseBt     QObject::tr("musicListDialogCloseBt")
#define AC_MessageBox                 QObject::tr("messageBox")
#define AC_FileDialog                 QObject::tr("fileDialog")


#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
