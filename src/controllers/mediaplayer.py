#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl, QDate)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from PyQt5.QtMultimedia import QMediaPlayer, QMediaContent


class MediaPlayer(QMediaPlayer):

    __contextName__ = "MediaPlayer"

    musicInfoChanged = pyqtSignal()

    @registerContext
    def __init__(self):
        super(MediaPlayer, self).__init__()
        self.setVolume(50)
        self.setNotifyInterval(50)
        self.setPlaybackRate(1)
        self.setMuted(False)

        self.initConnect()

    def initConnect(self):
        self.currentMediaChanged.connect(self.updateMusicInfo)

    @pyqtSlot('QMediaContent')
    def updateMusicInfo(self, media):
        self.musicInfoChanged.emit()

    @pyqtProperty('QMediaContent')
    def mediaObject(self):
        return super(MediaPlayer, self).media()

    @pyqtSlot(str)
    def setMediaUrl(self, url):
        self.setMedia(QMediaContent(QUrl(url)))

    @pyqtProperty(int)
    def volume(self):
        return super(MediaPlayer, self).volume()

    @pyqtSlot(int)
    def setVolume(self, value):
        return super(MediaPlayer, self).setVolume(value)

    @pyqtProperty(int)
    def notifyInterval(self):
        return  super(MediaPlayer, self).notifyInterval()

    @pyqtSlot(int)
    def setNotifyInterval(self, interval):
        super(MediaPlayer, self).setNotifyInterval(interval)

    @pyqtSlot(int)
    def setPostion(self, pos):
        super(MediaPlayer, self).setPostion(pos)

    @pyqtProperty(int)
    def duration(self):
        return super(MediaPlayer, self).duration()

    @pyqtProperty(bool)
    def seekable(self):
        return self.isSeekable()

    @pyqtProperty(str)
    def errorString(self):
        return super(MediaPlayer, self).errorString()

    @pyqtSlot(bool)
    def playToggle(self, playing):
        if playing:
            self.play()

            print self.isMetaDataAvailable()
            self.showMetaData()

            # media = self.playlist().currentMedia()
            # print(media.canonicalUrl())
            # print(media.canonicalResource().dataSize())

        else:
            self.pause()

    @pyqtProperty('QString')
    def positionString(self):
        position = super(MediaPlayer, self).position()
        return duration_to_string(position)

    @pyqtProperty('QString')
    def durationString(self):
        duration = super(MediaPlayer, self).duration()
        return duration_to_string(duration)

    @pyqtSlot('QString', result='QString')
    def metaData(self, key):
        return super(MediaPlayer, self).metaData(key)

    def showMetaData(self):
        import json
        metaData = {}
        for key in self.availableMetaData():
            v = self.metaData(key)
            if isinstance(v, QDate):
                v = v.toString('yyyy.MM.dd')
            metaData.update({key: v})
        path = os.sep.join([os.path.dirname(os.getcwd()), 'music', '%s.json' %self.metaData('Title')])
        f = open(path, 'w')
        f.write(json.dumps(metaData, indent=4))
        f.close()
