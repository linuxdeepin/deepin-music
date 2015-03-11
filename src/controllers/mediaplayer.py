#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QDate)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from PyQt5.QtMultimedia import QMediaPlayer, QMediaContent, QMediaPlaylist

from log import logger



class PlayerBin(QMediaPlayer):

    def __init__(self):
        super(PlayerBin, self).__init__()
        self.setNotifyInterval(50)


class MediaPlayer(QObject):

    __contextName__ = "MediaPlayer"

    musicInfoChanged = pyqtSignal()

    positionChanged = pyqtSignal(int)
    stateChanged = pyqtSignal(int)
    mediaStatusChanged = pyqtSignal('QMediaPlayer::MediaStatus')
    volumeChanged = pyqtSignal(int)

    @registerContext
    def __init__(self):
        super(MediaPlayer, self).__init__()

        self.player = PlayerBin()
        self._playlist = None

        self._state = 0
        self._isPlaying = False

        self.initConnect()

    def initConnect(self):
        self.player.currentMediaChanged.connect(self.updateMusicInfo)
        self.player.positionChanged.connect(self.positionChange)
        # self.player.stateChanged.connect(self.stateChange)
        self.player.mediaStatusChanged.connect(self.mediaStatusChanged)
        self.player.volumeChanged.connect(self.volumeChanged)

    @pyqtSlot('QMediaContent')
    def updateMusicInfo(self, media):
        self.musicInfoChanged.emit()

    @pyqtSlot(int)
    def positionChange(self, pos):
        self.positionChanged.emit(pos)

    @pyqtSlot(int)
    def stateChange(self, state):
        self.stateChanged.emit(state)

    @pyqtSlot(int)
    def mediaStatusChange(self, status):
        self.mediaStatusChanged.emit(status)

    @pyqtSlot(int)
    def volumeChange(self, volume):
        self.volumeChanged.emit(volume)

    @pyqtProperty('QMediaContent')
    def mediaObject(self):
        return self.player.media()

    @pyqtSlot('QString')
    def setMediaUrl(self, url):
        self.player.setMedia(QMediaContent(QUrl(url)))

        self.playToggle(self._isPlaying)

    @pyqtProperty('QVariant')
    def playlist(self):
        return self._playlist

    @pyqtSlot('QMediaPlaylist')
    def setPlaylist(self, playlist):
        self._playlist = playlist

    @pyqtSlot(int)
    def setPlaybackMode(self, playbackMode):
        self.playlist.setPlaybackMode(playbackMode)

    @pyqtProperty(int)
    def volume(self):
        return self.player.volume()

    @pyqtSlot(int)
    def setVolume(self, value):
        return self.player.setVolume(value)

    @pyqtProperty(int)
    def notifyInterval(self):
        return self.player.notifyInterval()

    @pyqtSlot(int)
    def setNotifyInterval(self, interval):
        self.player.setNotifyInterval(interval)

    @pyqtSlot(int)
    def setPosition(self, pos):
        self.player.setPosition(pos)

    @pyqtProperty(int)
    def duration(self):
        return self.player.duration()

    @pyqtProperty(bool)
    def seekable(self):
        return self.player.isSeekable()

    @pyqtProperty(str)
    def errorString(self):
        return self.player.errorString()

    @pyqtSlot(bool)
    def playToggle(self, playing):
        if playing:
            self.play()
        else:
            self.pause()

        self._isPlaying = playing

    @pyqtSlot()
    def stop(self):
        print('+++++++++++++')
        self.player.stop()
        self._state = 0
        self.stateChanged.emit(self._state)

    @pyqtSlot()
    def play(self):
        self.player.play()
        self._state = 1
        self.stateChanged.emit(self._state)

    @pyqtSlot()
    def pause(self):
        self.player.pause()
        self._state = 2
        self.stateChanged.emit(self._state)

    @pyqtProperty(int)
    def state(self):
        return self._state

    @pyqtProperty('QString')
    def positionString(self):
        position = self.player.position()
        return duration_to_string(position)

    @pyqtProperty('QString')
    def durationString(self):
        duration = self.player.duration()
        return duration_to_string(duration)

    @pyqtSlot('QString', result='QString')
    def metaData(self, key):
        return self.player.metaData(key)

    def showMetaData(self):
        import json
        metaData = {}
        for key in self.availableMetaData():
            v = self.metaData(key)
            if isinstance(v, QDate):
                v = v.toString('yyyy.MM.dd')
            metaData.update({key: v})
        logger.info(metaData)
        path = os.sep.join(
            [os.path.dirname(os.getcwd()), 'music',
             '%s.json' % self.metaData('Title')])
        f = open(path, 'w')
        f.write(json.dumps(metaData, indent=4))
        f.close()
