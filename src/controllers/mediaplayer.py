#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty, QUrl
from PyQt5.QtGui import QCursor
from .utils import registerContext

from PyQt5.QtMultimedia import QMediaPlayer, QMediaContent


class MediaPlayer(QMediaPlayer):

    __contextName__ = "MediaPlayer"

    @registerContext
    def __init__(self):
        super(MediaPlayer, self).__init__()
        self.setMedia(QMediaContent(QUrl('http://sv67.yunpan.cn/Download.outputAudio/1900187504/9704d5814e04cb07a918a95f76965d1564988118/67_67.7b3e40d0d6c36462a661bcd9b1ccaccc/1.0/openapi/14250369993075/10003/c5c83c9661de45ff73310e6986e5e93e/愿得一人心_4197.mp3')))
        self.setVolume(50)
        # self.play()

    @pyqtProperty('QMediaContent')
    def mediaObject(self):
        return self.media()

    @pyqtProperty(int)
    def d_notifyInterval(self):
        return  self.notifyInterval()

    @d_notifyInterval.setter
    def d_notifyInterval(self, interval):
        self.setNotifyInterval(interval)

    @pyqtProperty(int)
    def d_duration(self):
        return self.duration()

    @pyqtSlot(int)
    def setPostion(self, pos):
        super(MediaPlayer, self).setPostion(pos)

    @pyqtProperty(int)
    def duration(self):
        return super(MediaPlayer, self).duration()

    @pyqtProperty(bool)
    def seekable(self):
        return self.isSeekable()
