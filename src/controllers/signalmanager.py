#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl)
from .utils import registerContext, contexts


class SignalManager(QObject):

    __contextName__ = 'SignalManager'

    #添加我的收藏
    addtoFavorite = pyqtSignal('QString')

    #移出我的收藏
    removeFromFavorite = pyqtSignal('QString')

    # 在线音乐添加到下载列表
    addtoDownloadlist = pyqtSignal(int)
    addAlltoDownloadlist = pyqtSignal('QString')

    # 在线音乐对象切换本地音乐对象
    switchOnlinetoLocal = pyqtSignal('QString', 'QString')

    #
    addLocalSongToDataBase = pyqtSignal('QString')
    addLocalSongsToDataBase = pyqtSignal(list)

    # mediaplayer

    playingChanged = pyqtSignal(bool)

    #lrc
    sizeChanged = pyqtSignal(int, int)

    noLrcFound = pyqtSignal()

    lineModeChanged = pyqtSignal(int)
    singleTextInfoChanged = pyqtSignal('QString', float, int)
    douleTextInfoChanged = pyqtSignal(list)

    previousSong = pyqtSignal()
    playToggle = pyqtSignal(bool)
    nextSong = pyqtSignal()
    fontIncreaseChanged = pyqtSignal()
    fontDecreaseChanged = pyqtSignal()

    showLrcSingleLine = pyqtSignal()
    showLrcDoubleLine = pyqtSignal()

    locked = pyqtSignal()
    unlocked = pyqtSignal()
    toggleShow = pyqtSignal()
    lrcClosed = pyqtSignal()

    @registerContext
    def __init__(self, parent=None):
        super(SignalManager, self).__init__(parent)



signalManager = SignalManager()
