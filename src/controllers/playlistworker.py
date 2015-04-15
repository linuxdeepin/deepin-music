#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from collections import OrderedDict
import json
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QFile, QIODevice, QTimer
                          )
from PyQt5.QtMultimedia import QMediaPlaylist, QMediaContent, QMediaPlayer

from .utils import registerContext, contexts, registerObj
from dwidgets.mediatag.song import Song
from dwidgets import dthread
from .coverworker import CoverWorker
from config.constants import PlaylistPath
from config.constants import CoverPath
from log import logger
from dwidgets import DListModel
from .muscimanageworker import QmlSongObject, MusicManageWorker



class PlayerBin(QMediaPlayer):

    def __init__(self):
        super(PlayerBin, self).__init__()
        self.setNotifyInterval(50)


class DLocalMediaContent(QMediaContent):

    def __init__(self, url):
        super(DLocalMediaContent, self).__init__(QUrl.fromLocalFile(url))
        self._url = url

    @pyqtProperty('QString')
    def url(self):
        return self._url

    @pyqtProperty('QString')
    def type(self):
        return 'DLocalMediaContent'


class DOnlineMediaContent(QMediaContent):

    def __init__(self, url, tags=None):
        super(DOnlineMediaContent, self).__init__(QUrl(url))
        self._url = url

    @pyqtProperty('QString')
    def url(self):
        return self._url

    @pyqtProperty('QString')
    def type(self):
        return 'DOnlineMediaContent'



class DMediaPlaylist(QMediaPlaylist):

    nameChanged = pyqtSignal('QString')

    countChanged = pyqtSignal(int)
    mediasChanged = pyqtSignal('QVariant')

    def __init__(self, name=''):
        super(DMediaPlaylist, self).__init__()
        self._name = name
        self._urls = []
        self._medias = DListModel(QmlSongObject)

    @pyqtProperty('QString', notify=nameChanged)
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        self._name = name
        self.nameChanged.emit()

    @pyqtProperty('QVariant')
    def urls(self):
        return self._urls

    @pyqtProperty(int, notify=countChanged)
    def count(self):
        return self.mediaCount()

    @pyqtSlot()
    def updateListModel(self):
        self._medias.clear()
        for url in self._urls:
            if url.startswith('http'):
                pass
            else:
                songObj = MusicManageWorker.getSongObjByUrl(url)
                self._medias.append(songObj)

    @pyqtSlot()
    def clearMedias(self):
        flag = self.clear()
        if flag:
            self._urls = []

    def addMedia(self, url):
        if url not in self._urls:
            if url.startswith('http'):
                self.addOnlineMedia(url)
                self._urls.append(url)
            else:
                if os.path.exists(url):
                    self.addLocalMedia(url)
                    self._urls.append(url)

    def addMedias(self, urls):
        for url in urls:
            self.addMedia(url)

    def addLocalMedia(self, url):
        mediaContent = DLocalMediaContent(url)
        super(DMediaPlaylist, self).addMedia(mediaContent)

    def addOnlineMedia(self, url):
        mediaContent = DOnlineMediaContent(url)
        super(DMediaPlaylist, self).addMedia(mediaContent)


class PlaylistWorker(QObject):

    __contextName__ = 'PlaylistWorker'

    nameExisted = pyqtSignal('QString')
    playlistNamesChanged = pyqtSignal('QVariant')
    currentPlaylistChanged = pyqtSignal('QString')

    registerObj  = pyqtSignal('QString', 'QVariant')

    @registerContext
    def __init__(self, parent=None):
        super(PlaylistWorker, self).__init__(parent)

        self._playlists = OrderedDict()
        self._playlistNames = []
        self._currentPlaylist = None

        self.registerObj.connect(registerObj)
        self.createPlaylistByName('temporary')
        self.createPlaylistByName('favorite')

        self.initPlaylist()

        

    def initPlaylist(self):
        urls = [
            u'/usr/share/deepin-sample-music/邓入比_我们的情歌.mp3',
            u'/usr/share/deepin-sample-music/郭一凡_说走就走的旅行.mp3',
            u'/usr/share/deepin-sample-music/胡彦斌_依然是你.mp3'
        ]
        for url in urls:
            self.addMediaToFavorite(url)
        self.loadPlaylists()

    def savePlaylists(self):
        # result = OrderedDict()
        # for name, playlist in self._playlists.items():
        #     _playlist = OrderedDict()
        #     for url, mediaContent in playlist.mediaContents.items():
        #         if 'created_date' in mediaContent.tags:
        #             mediaContent.tags.pop('created_date')
        #         _playlist[url] = mediaContent.tags
        #     result[name] = _playlist

        # playlistPath = os.path.join(PlaylistPath, 'DeepinMusic3.playlist')
        # with open(playlistPath, 'wb') as f:
        #     json.dump(result, f, indent=4)
        pass

    def loadPlaylists(self):
        pass
        # playlistPath = os.path.join(PlaylistPath, 'DeepinMusic3.playlist')
        # if os.path.exists(playlistPath):
        #     with open(playlistPath, 'r') as f:
        #         results = json.load(f, object_pairs_hook=OrderedDict)
        #     for name, _playlist in results.items():
        #         playlist = self.createPlaylistByName(name)
        #         medias = []
        #         for url, tags in _playlist.items():
        #             if url.startswith("http"):
        #                 medias.append({
        #                     'url' : url,
        #                     'tags': tags,
        #                     'updated': True
        #                 })
        #             else:
        #                 medias.append(tags)
        #         playlist.addMedias(medias)
            # from objbrowser import browse
            # browse(results)

    def savePlaylistByName(self, name):
        f = QFile(os.sep.join([PlaylistPath, '%s.m3u' % name]))
        flag = f.open(QIODevice.ReadWrite)
        if flag:
            self._playlists[name].save(f, 'm3u')
            f.close()

    @pyqtProperty('QVariant')
    def temporaryPlaylist(self):
        return self._playlists['temporary']

    @pyqtProperty('QVariant')
    def favoritePlaylist(self):
        return self._playlists['favorite']

    @pyqtSlot('QString', result='QVariant')
    def getPlaylistByName(self, name):
        if name in self._playlists:
            return self._playlists[name]
        else:
            return None

    @pyqtProperty(dict)
    def playlists(self):
        return self._playlists

    @pyqtSlot('QString')
    def addMediaToTemporary(self, url):
        self._playlists['temporary'].addMedia(url)

    @pyqtSlot('QString')
    def addMediaToFavorite(self, url):
        self._playlists['favorite'].addMedia(url)

    @pyqtProperty('QVariant', notify=playlistNamesChanged)
    def playlistNames(self):
        return self._playlistNames

    @playlistNames.setter
    def playlistNames(self, names):
        self._playlistNames = names
        self.playlistNamesChanged.emit(self._playlistNames)

    @pyqtSlot('QString')
    def createPlaylistByName(self, name):
        names = self._playlistNames + ['favorite', 'temporary']
        if name in self._playlists:
            self.nameExisted.emit(name)
        else:
            self._playlists[name] = DMediaPlaylist(name)
            self.registerObj.emit(name, self._playlists[name]._medias)
            if name not in ['favorite', 'temporary']:
                self._playlistNames.insert(0, {'name': name})
                self.playlistNames = self._playlistNames

        return self._playlists[name]

    @pyqtSlot('QString', result='QVariant')
    def getMediasByName(self, name):
        # if name in self._playlists:
        #     return self._playlists[name].medias
        pass

    def addLocalMediaToTemporary(self, url):
        playlist = self.temporaryPlaylist
        self.currentPlaylistChanged.emit('temporary')
        playlist.addMedia(url)

    def addLocalMediasToTemporary(self, urls):
        playlist = self.temporaryPlaylist
        self.currentPlaylistChanged.emit('temporary')
        playlist.addMedias(urls)

    def addOnlineMediaToTemporary(self, media):
        playlist = self.temporaryPlaylist
        self.currentPlaylistChanged.emit('temporary')
        playlist.addMedia(media['url'], media['tags'], media['updated'])

    def addOnlineMediasToTemporary(self, medias):
        playlist = self.temporaryPlaylist
        self.currentPlaylistChanged.emit('temporary')
        playlist.addMedias(medias)

    def addOnlineMediaToFavorite(self, media):
        playlist = self.favoritePlaylist
        playlist.addMedia(media['url'], media['tags'], media['updated'])

    def removeFavoriteMediaContent(self, url):
        playlist = self.favoritePlaylist
        playlist.removeMediaByUrl(url)

    def addOnlineMediasToFavorite(self, medias):
        playlist = self.favoritePlaylist
        playlist.addMedias(medias)
