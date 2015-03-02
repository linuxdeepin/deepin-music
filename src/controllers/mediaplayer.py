#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty, QUrl
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts

from PyQt5.QtMultimedia import QMediaPlayer, QMediaContent


class MediaPlayer(QMediaPlayer):

    __contextName__ = "MediaPlayer"

    @registerContext
    def __init__(self):
        super(MediaPlayer, self).__init__()
        self.setVolume(50)
        self.setNotifyInterval(50)
        self.setPlaybackRate(1)
        self.setMuted(False)

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
            self.play();
        else:
            self.pause()
