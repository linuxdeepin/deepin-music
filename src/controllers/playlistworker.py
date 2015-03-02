#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl
    )
from PyQt5.QtMultimedia import QMediaPlaylist, QMediaContent
from .utils import registerContext


class PlaylistWorker(QObject):

    __contextName__ = 'PlaylistWorker'

    nameExisted = pyqtSignal(str)

    @registerContext
    def __init__(self, parent=None):
        super(PlaylistWorker, self).__init__(parent)

        self._playlists = {}
        self._playlists['temporary'] = QMediaPlaylist()

        self.initPlaylist()



    def initPlaylist(self):
        self.addMediaToTemporary(os.sep.join([os.path.dirname(os.getcwd()), 'music', '1.mp3']))
        self.addMediaToTemporary(os.sep.join([os.path.dirname(os.getcwd()), 'music', '2.mp3']))
        self.addMediaToTemporary(os.sep.join([os.path.dirname(os.getcwd()), 'music', '3.mp3']))
        self.addMediaToTemporary(os.sep.join([os.path.dirname(os.getcwd()), 'music', '4.mp3']))
        self.addMediaToTemporary(os.sep.join([os.path.dirname(os.getcwd()), 'music', '5.mp3']))
        print QUrl(os.sep.join([os.getcwd(), 'playlist.m3u']))
        self._playlists['temporary'].save(QUrl(os.sep.join([os.getcwd(), 'playlist.m3u'])))

    @pyqtProperty(dict)
    def playlists(self):
        return self._playlistss

    @pyqtSlot('QString', 'QString')
    def addMediaToTemporary(self, url):
        self._playlists['temporary'].addMedia(QMediaContent(QUrl(url)))

    @pyqtSlot('QString', 'QString')
    def addMediaByName(self, name, url):
        if name in self._playlists:
            self._playlists[name].addMedia(QMediaContent(QUrl(url)))
        else:
            print("the playlist named %s isn't existed" % name)

    @pyqtSlot('QString')
    def createPlaylistByName(self, name):
        if name in self._playlistss:
            self.nameExisted.emit(name)
        else:
            self._playlistss[name] = QMediaPlaylist()
