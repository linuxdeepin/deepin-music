#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import random
from collections import OrderedDict
import json
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
      pyqtProperty, QUrl, QFile, QIODevice, QTimer
      )
from PyQt5.QtGui import QGuiApplication


class DPlaylist(QObject):
    nameChanged = pyqtSignal('QString')
    countChanged = pyqtSignal(int)
    mediasChanged = pyqtSignal('QVariant')

    currentIndexChanged = pyqtSignal(int)
    playbackModeChanged = pyqtSignal(int)

    def __init__(self):
        super(DPlaylist, self).__init__()

        self._name = name
        self._urls = []
        self._medias = []
        self._mediaContents = OrderedDict()
        self._count = 0

        self._currentIndex = 0
        self._playbackMode = 1 # 1: CurrentItemInLoop, 3: Loop, 4: Random

    @pyqtProperty('QString', notify=nameChanged)
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        self._name = name
        self.nameChanged.emit()

    @pyqtProperty('QVariant', notify=mediasChanged)
    def medias(self):
        return self._medias

    @medias.setter
    def medias(self, medias):
        self._medias = medias
        self.mediasChanged.emit(self._medias)
        self.count = len(self.medias)

    @pyqtProperty(int, notify=countChanged)
    def count(self):
        return self._count

    @count.setter
    def count(self, value):
        self._count = value
        self.countChanged.emit(value)

    @pyqtProperty(int, notify=currentIndexChanged)
    def currentIndex(self):
        return self._currentIndex

    @currentIndex.setter
    def currentIndex(self, index):
        self._currentIndex = index
        self.currentIndexChanged.emit(index)

    def setCurrentIndex(self, index):
        self.currentIndex = index

    @pyqtProperty(int)
    def playbackMode(self):
        return self._playbackMode

    @playbackMode.setter
    def playbackMode(self, value):
        self._playbackMode = value
        self.playbackModeChanged.emit(value)

    def addMedia(self, url):
        if url not in self._urls:
            self._urls.append(url)

    def addMedias(self, urls):
        for url in urls:
            self.addMedia(url)

    def removeMediaByUrl(self, url):
        if url in self._urls:
            self._urls.remove(url)

    def removeMediaByIndex(self, index):
        if index < len(self._urls):
            self._urls.pop(index)

    def clear(self):
        self._urls = []
        self._medias = []
        self._mediaContents = OrderedDict()

    def next(self):
        if self._playbackMode == 1:
            if self.currentIndex < self._count - 1:
                self.currentIndex = self.currentIndex + 1
            elif self.currentIndex == self._count - 1:
                self.currentIndex = 0
        elif self._playbackMode == 3:
            if self.currentIndex < self._count - 1:
                self.currentIndex = self.currentIndex + 1
            elif self.currentIndex == self._count - 1:
                self.currentIndex = 0
        elif self._playbackMode == 4:
            if self._count > 0:
                index = random.random(0, self._count - 1)
                self.currentIndex = index
        else:
            if self.currentIndex < self._count - 1:
                self.currentIndex = self.currentIndex + 1
            elif self.currentIndex == self._count - 1:
                self.currentIndex = 0

    def previous(self):
        if self._playbackMode == 1:
            if self.currentIndex > 0 :
                self.currentIndex = self.currentIndex - 1
            elif self.currentIndex == 0:
                self.currentIndex = self.currentIndex - 1
        elif self._playbackMode == 3:
            if self.currentIndex > 0:
                self.currentIndex = self.currentIndex - 1
            elif self.currentIndex == 0:
                self.currentIndex = self.currentIndex - 1
        elif self._playbackMode == 4:
            if self._count > 0:
                index = random.random(0, self._count - 1)
                self.currentIndex = index
        else:
            if self.currentIndex > 0:
                self.currentIndex = self.currentIndex - 1
            elif self.currentIndex == 0:
                self.currentIndex = self.currentIndex - 1

if __name__ == '__main__':
    main()


    app = QGuiApplication()



    exitCode = app.exec_()
    sys.exit(exitCode)