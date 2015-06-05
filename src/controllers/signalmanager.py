#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl)
from .utils import registerContext, contexts


class SignalManager(QObject):

    __contextName__ = 'SignalManager'

    registerQmlObj = pyqtSignal('QString', 'QVariant')

    #init load
    downloadSuggestPlaylist = pyqtSignal()

    #play song by local url
    playMusicByLocalUrl = pyqtSignal('QString')

    #play song by songId like 86245
    playMusicBySongIdSignal = pyqtSignal(int)

    #play songs by ids like '1503754_189217_280991'
    playMusicByIdsSignal = pyqtSignal('QString')

    #play songs by album Id like 1285572
    playMusicByAlbumIdSignal = pyqtSignal(int)

    # add online song to favorite playlist by songId
    addToFavoriteByIdSignal = pyqtSignal(int)

    # remove online song from playlist by songId 
    removeFromFavoriteByIdSignal = pyqtSignal(int)

    #添加我的收藏
    addtoFavorite = pyqtSignal('QString')

    #移出我的收藏
    removeFromFavorite = pyqtSignal('QString')

    # 在线音乐添加到下载列表
    addtoDownloadlist = pyqtSignal(int)
    addAlltoDownloadlist = pyqtSignal('QString')

    # 在线音乐对象切换本地音乐对象
    switchOnlinetoLocal = pyqtSignal('QString', 'QString')

    #download song  
    addLocalSongToDataBase = pyqtSignal('QString')
    addLocalSongsToDataBase = pyqtSignal(list)

    # mediaplayer
    previousSong = pyqtSignal()
    playToggle = pyqtSignal(bool)
    nextSong = pyqtSignal()
    volumnIncrease = pyqtSignal()
    volumnDecrease = pyqtSignal()
    playbackModeChanged = pyqtSignal(int)

    playingChanged = pyqtSignal(bool)
    lrcPositionChanged = pyqtSignal('qint64', int)

    #Dialog
    dialogClosed = pyqtSignal()

    #add playlist dialog
    newPlaylistDialogShowed = pyqtSignal()
    addNewPlaylist = pyqtSignal('QString')

    #add multi playlist dialog
    newMultiPlaylistDialogShowed = pyqtSignal('QString', 'QString')
    addMutiPlaylistFlags = pyqtSignal('QVariant')
    addSongsToMultiPlaylist = pyqtSignal('QString', 'QString', list)

    #global search
    globalSearched = pyqtSignal('QString')
    jumpToLocalDetailArtist = pyqtSignal('QString')
    jumpToLocalDetailAlbum = pyqtSignal('QString')

    #online search
    onlineResult = pyqtSignal(dict)
    suggestPlaylist = pyqtSignal(dict)

    playAll = pyqtSignal('QString')
    informationShow = pyqtSignal('QString')

    #lrc
    downloadLrc = pyqtSignal('QString', 'QString')
    updateLrc = pyqtSignal()
    noLrcFound = pyqtSignal()
    lrcToggleShow = pyqtSignal()
    lrcToggleLock = pyqtSignal()

    lineModeChanged = pyqtSignal(int)
    singleTextInfoChanged = pyqtSignal('QString', float, int)
    douleTextInfoChanged = pyqtSignal(list)

    fontIncreaseChanged = pyqtSignal()
    fontDecreaseChanged = pyqtSignal()
    lrcBackHalfSecond = pyqtSignal()
    lrcForwardHarfSecond = pyqtSignal()
    lrcThemeChanged = pyqtSignal()
    showLrcSingleLine = pyqtSignal()
    showLrcDoubleLine = pyqtSignal()
    kalaokChanged = pyqtSignal()
    locked = pyqtSignal()
    unlocked = pyqtSignal()
    lrcSetting = pyqtSignal()
    lrcSearch = pyqtSignal()
    lrcClosed = pyqtSignal()

    # window manager
    simpleFullToggle = pyqtSignal()
    miniFullToggle = pyqtSignal()
    fullMode = pyqtSignal()
    simpleMode = pyqtSignal()
    miniMode = pyqtSignal()

    hideShowWindowToggle = pyqtSignal()
    hideShowDesktopLrcToggle = pyqtSignal()

    # SystemTray
    systemTrayContext = pyqtSignal()

    exited = pyqtSignal()

    @registerContext
    def __init__(self, parent=None):
        super(SignalManager, self).__init__(parent)


signalManager = SignalManager()
