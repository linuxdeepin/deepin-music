#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl)
from PyQt5.QtGui import QCursor, QDesktopServices
from models import *
from dwidgets import DListModel, ModelMetaclass
from .muscimanageworker import QmlSongObject, QmlArtistObject, QmlAlbumObject, MusicManageWorker
from .signalmanager import signalManager
from .utils import registerContext


class QmlOnlineSongObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('songId', int),
        ('songName', 'QString'),
        ('singerId', int),
        ('singerName', 'QString'),
        ('albumId', int),
        ('albumName', 'QString')
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)


class QmlOnlineAlbumObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('singerId', int),
        ('singerName', 'QString'),
        ('albumId', int),
        ('albumName', 'QString'),
        ('albumImage', 'QString')
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)


class SearchLocalSongListModel(DListModel):

    __contextName__ = 'SearchLocalSongListModel'

    @registerContext
    def __init__(self, dataTye):
        super(SearchLocalSongListModel, self).__init__(dataTye)

class SearchLocalArtistListModel(DListModel):

    __contextName__ = 'SearchLocalArtistListModel'

    @registerContext
    def __init__(self, dataTye):
        super(SearchLocalArtistListModel, self).__init__(dataTye)

class SearchLocalAlbumListModel(DListModel):

    __contextName__ = 'SearchLocalAlbumListModel'

    @registerContext
    def __init__(self, dataTye):
        super(SearchLocalAlbumListModel, self).__init__(dataTye)


class SearchOnlineSongListModel(DListModel):

    __contextName__ = 'SearchOnlineSongListModel'

    @registerContext
    def __init__(self, dataTye):
        super(SearchOnlineSongListModel, self).__init__(dataTye)

class SearchOnlineAlbumListModel(DListModel):

    __contextName__ = 'SearchOnlineAlbumListModel'

    @registerContext
    def __init__(self, dataTye):
        super(SearchOnlineAlbumListModel, self).__init__(dataTye)


class SearchWorker(QObject):

    __contextName__ = 'SearchWorker'

    _searchLocalSongObjsListModel = SearchLocalSongListModel(QmlSongObject)
    _searchLocalArtistObjsListModel = SearchLocalArtistListModel(QmlArtistObject)
    _searchLocalAlbumObjsListModel = SearchLocalAlbumListModel(QmlAlbumObject)

    _searchOnlineSongObjsListModel = SearchOnlineSongListModel(QmlOnlineSongObject)
    _searchOnlineAlbumObjsListModel = SearchOnlineAlbumListModel(QmlOnlineAlbumObject)

    keywordChanged = pyqtSignal('QString')

    @registerContext
    def __init__(self, parent=None):
        super(SearchWorker, self).__init__(parent)
        self._keyword = ''
        self.initConnect()

    def initConnect(self):
        signalManager.globalSearched.connect(self.searchLocalSongs)
        signalManager.onlineResult.connect(self.handleOnlineSongs)

    @pyqtProperty('QString', notify=keywordChanged)
    def keyword(self):
        return self._keyword

    @keyword.setter
    def keyword(self, value):
        self._keyword = value
        self.keywordChanged.emit(value)

    def searchLocalSongs(self, keyword):
        self.keyword = keyword
        self._searchLocalSongObjsListModel.clear()
        for song in Song.select().where(Song.title.contains(keyword) | Song.artist.contains(
            keyword) | Song.album.contains(keyword)):
            if song.url in MusicManageWorker._songObjs:
                self._searchLocalSongObjsListModel.append(MusicManageWorker._songObjs[song.url])

        self._searchLocalArtistObjsListModel.clear()
        for artist in Artist.select().where(Artist.name.contains(keyword)):
            if artist.name in MusicManageWorker._artistObjs:
                self._searchLocalArtistObjsListModel.append(MusicManageWorker._artistObjs[artist.name])
                if self._searchLocalArtistObjsListModel.count >= 5:
                    break

        self._searchLocalAlbumObjsListModel.clear()
        for album in Album.select().where(Album.name.contains(keyword)):
            if album.name in MusicManageWorker._albumObjs:
                self._searchLocalAlbumObjsListModel.append(MusicManageWorker._albumObjs[album.name])
                if self._searchLocalAlbumObjsListModel.count >= 5:
                    break

    def handleOnlineSongs(self, result):
        if 'songList' in result:
            songList = result['songList']
            self._searchOnlineSongObjsListModel.clear()
            for song in songList:
                obj = QmlOnlineSongObject(**song)
                self._searchOnlineSongObjsListModel.append(obj)

        if 'albumList' in result and 'data' in result['albumList']:
            self._searchOnlineAlbumObjsListModel.clear()
            albumList = result['albumList']['data']
            for album in albumList:
                obj = QmlOnlineAlbumObject(**album)
                self._searchOnlineAlbumObjsListModel.append(obj)

    def searchLocal(self, keyword):
        pass
