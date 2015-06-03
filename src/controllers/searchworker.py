#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QThreadPool, 
                          QRunnable)
from PyQt5.QtGui import QCursor, QDesktopServices
import requests
from models import *
from dwidgets import DListModel, ModelMetaclass
from .muscimanageworker import QmlSongObject, QmlArtistObject, QmlAlbumObject, MusicManageWorker
from .signalmanager import signalManager
from .web360apiworker import Web360ApiWorker
from .utils import registerContext
from config.constants import OnlineAlbumCoverPath
from .coverworker import CoverWorker


class QmlOnlineSongObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('url', 'QString'),
        ('title', 'QString'),
        ('artist', 'QString'),
        ('album', 'QString'),
        ('songId', int),
        ('singerId', int),
        ('albumId', int),
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)


class QmlOnlineAlbumObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('singerId', int),
        ('albumId', int),
        ('artist', 'QString'),
        ('album', 'QString'),
        ('cover', 'QString', CoverWorker.defaultAlbumCover)
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)


class QmlSuggestPlaylistObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('listId', int),
        ('listName', 'QString'),
        ('playAll', 'QString'),
        ('cover', 'QString', CoverWorker.defaultAlbumCover)
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


class SuggestPlaylistListModel(DListModel):

    __contextName__ = 'SuggestPlaylistListModel'

    @registerContext
    def __init__(self, dataTye):
        super(SuggestPlaylistListModel, self).__init__(dataTye)



class AlbumCover360Runnable(QRunnable):

    def __init__(self, index, worker, artist, album, url):
        super(AlbumCover360Runnable, self).__init__()
        self.index = index
        self.worker = worker
        self.artist = artist
        self.album = album
        self.url = url

    def run(self):
        localUrl = self.worker.getAlbumCoverPath(self.artist, self.album)
        try:
            r = requests.get(self.url)
            with open(localUrl, "wb") as f:
                f.write(r.content)
            self.worker.albumCoverDownloaded.emit(self.index, localUrl)
        except Exception, e:
            pass


class PlaylistCover360Runnable(QRunnable):

    def __init__(self, index, worker, listName, url):
        super(PlaylistCover360Runnable, self).__init__()
        self.index = index
        self.worker = worker
        self.listName = listName
        self.url = url

    def run(self):
        localUrl = self.worker.getPlaylistCoverPath(self.listName)
        try:
            r = requests.get(self.url)
            with open(localUrl, "wb") as f:
                f.write(r.content)
            self.worker.playlistCoverDownloaded.emit(self.index, localUrl)
        except Exception, e:
            pass


class SearchWorker(QObject):

    __contextName__ = 'SearchWorker'

    _searchLocalSongObjsListModel = SearchLocalSongListModel(QmlSongObject)
    _searchLocalArtistObjsListModel = SearchLocalArtistListModel(QmlArtistObject)
    _searchLocalAlbumObjsListModel = SearchLocalAlbumListModel(QmlAlbumObject)

    _searchOnlineSongObjsListModel = SearchOnlineSongListModel(QmlOnlineSongObject)
    _searchOnlineAlbumObjsListModel = SearchOnlineAlbumListModel(QmlOnlineAlbumObject)

    _suggestPlayListModel = SuggestPlaylistListModel(QmlSuggestPlaylistObject)

    keywordChanged = pyqtSignal('QString')

    albumCoverDownloaded = pyqtSignal(int, 'QString')
    playlistCoverDownloaded = pyqtSignal(int, 'QString')

    @registerContext
    def __init__(self, parent=None):
        super(SearchWorker, self).__init__(parent)
        self._keyword = ''
        self._lists = []
        self.initConnect()

    def initConnect(self):
        self.albumCoverDownloaded.connect(self.updateOnlineAlbumListModel)
        self.playlistCoverDownloaded.connect(self.updatePlaylistListModel)

        signalManager.globalSearched.connect(self.searchLocalSongs)
        signalManager.onlineResult.connect(self.handleOnlineSongs)
        signalManager.suggestPlaylist.connect(self.handleSuggestPlaylist)


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

        self._searchLocalAlbumObjsListModel.clear()
        for album in Album.select().where(Album.name.contains(keyword)):
            if album.name in MusicManageWorker._albumObjs:
                self._searchLocalAlbumObjsListModel.append(MusicManageWorker._albumObjs[album.name])

    def handleOnlineSongs(self, result):
        self._searchOnlineSongObjsListModel.clear()
        self._searchOnlineAlbumObjsListModel.clear()
        if 'songList' in result:
            songList = result['songList']

            for song in songList:
                song['url'] = Web360ApiWorker.getUrlByID(int(song['songId']))
                song['title'] = song['songName']
                song['artist'] = song['singerName']
                song['album'] = song['albumName']
                song['songId'] = int(song['songId'])
                song['singerId'] = int(song['singerId'])
                song['albumId'] = int(song['albumId'])
                obj = QmlOnlineSongObject(**song)
                self._searchOnlineSongObjsListModel.append(obj)

        if 'albumList' in result and 'data' in result['albumList']:
            albumList = result['albumList']['data']
            for album in albumList:
                index = albumList.index(album)
                album['artist'] = album['singerName']
                album['album'] = album['albumName']
                if self.isAlbumCoverExisted(album['artist'], album['album']):
                    album['cover'] = self.getAlbumCoverPath(album['artist'], album['album'])
                else:
                    d = AlbumCover360Runnable(index, self, album['artist'], album['album'], album['albumImage'])
                    QThreadPool.globalInstance().start(d)

                obj = QmlOnlineAlbumObject(**album)
                self._searchOnlineAlbumObjsListModel.append(obj)

    def updateOnlineAlbumListModel(self, index, coverPath):
        self._searchOnlineAlbumObjsListModel.setProperty(index, 'cover', coverPath)


    def handleSuggestPlaylist(self, result):
        if 'lists' in result:
            if self._lists != result['lists']:
                self._lists = result['lists']
                self._suggestPlayListModel.clear()
            else:
                return

            lists = self._lists
            for playlist in lists:
                index = lists.index(playlist)
                _list = {}
                _list['listId'] = playlist['listId']
                _list['listName'] = playlist['listName']
                _list['playAll'] = playlist['playAll']

                if self.isPlaylistCoverExisted(playlist['listName']):
                    _list['cover'] = self.getPlaylistCoverPath(playlist['listName'])
                else:
                    d = PlaylistCover360Runnable(index, self, playlist['listName'], playlist['image'])
                    QThreadPool.globalInstance().start(d)

                obj = QmlSuggestPlaylistObject(**_list)
                self._suggestPlayListModel.append(obj)

    def updatePlaylistListModel(self, index, coverPath):
        self._suggestPlayListModel.setProperty(index, 'cover', coverPath)

    @classmethod
    def isAlbumCoverExisted(cls, artist, album):
        path = cls.getAlbumCoverPath(artist, album)
        if os.path.exists(path):
            return True
        else:
            return False

    @classmethod
    def getAlbumCoverPath(cls, artist, album):
        return os.path.join(OnlineAlbumCoverPath, cls.md5('%s-%s'%(artist, album)))


    @classmethod
    def isPlaylistCoverExisted(cls, listName):
        path = cls.getPlaylistCoverPath(listName)
        if os.path.exists(path):
            return True
        else:
            return False

    @classmethod
    def getPlaylistCoverPath(cls, listName):
        return os.path.join(OnlineAlbumCoverPath, cls.md5(listName))

    @classmethod
    def md5(cls, s):
        import hashlib
        md5Value = hashlib.md5(s.encode('utf-8'))
        return md5Value.hexdigest()

searchWorker = SearchWorker()
