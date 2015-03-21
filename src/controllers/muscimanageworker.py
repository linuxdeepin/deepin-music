#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty

from .utils import registerContext
from dwidgets.tornadotemplate import template


class MusicManageWorker(QObject):

    artistsChanged = pyqtSignal('QVariant')
    albumsChanged = pyqtSignal('QVariant')
    foldersChanged = pyqtSignal('QVariant')

    __contextName__ = 'MusicManageWorker'

    @registerContext
    def __init__(self, parent=None):
        super(MusicManageWorker, self).__init__(parent)

        self._artists = []
        self._albums = []
        self._folders = []

        self.initData()

    def initData(self):
        for i in range(100):
            item = {
                'name': 'artist%d' % i,
                'count': 100,
                'urls': range(100)
            }
            self._artists.append(item)

        for i in range(20):
            item = {
                'name': 'album%d' % i,
                'count': 20,
                'urls': range(20)
            }
            self._albums.append(item)

        for i in range(1000):
            item = {
                'name': '/home/djf/folder%d' % i,
                'count': 1000,
                'urls': range(1000)
            }
            self._folders.append(item)

    @pyqtProperty('QVariant')
    def categories(self):
        return []

    @pyqtProperty('QVariant', notify=artistsChanged)
    def artists(self):
        return self._artists 

    @artists.setter
    def artists(self, value):
        self._artists = value
        self.artistsChanged.emit(self._artists)

    @pyqtProperty('QVariant', notify=albumsChanged)
    def albums(self):
        return self._albums 

    @albums.setter
    def albums(self, value):
        self._albums = value
        self.albumsChanged.emit(self._albums)

    @pyqtProperty('QVariant', notify=foldersChanged)
    def folders(self):
        return self._folders 

    @folders.setter
    def folders(self, value):
        self._folders = value
        self.foldersChanged.emit(self._folders)
