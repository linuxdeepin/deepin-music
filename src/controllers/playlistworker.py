#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from collections import OrderedDict
import json 
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl, QFile, QIODevice
    )
from PyQt5.QtMultimedia import QMediaPlaylist, QMediaContent
from .utils import registerContext, contexts
from dwidgets.mediatag.song import Song
from dwidgets import dthread
from config.constants import PlaylistPath
from log import logger


class BaseMediaContent(QObject):

    titleChanged = pyqtSignal('QString')
    artistChanged = pyqtSignal('QString')
    coverChanged = pyqtSignal('QString')

    def __init__(self, url):
        super(BaseMediaContent, self).__init__()
        self._url = url
        self._title = ''
        self._artist = ''
        self._cover = ''

    @pyqtProperty('QString')
    def url(self):
        return self._url

    @pyqtProperty('QString', notify=titleChanged)
    def title(self):
        return self._title

    @title.setter
    def title(self, title):
        self._title = title
        self.titleChanged.emit(title)

    @pyqtProperty('QString', notify=artistChanged)
    def artist(self):
        return self._artist

    @artist.setter
    def artist(self, artist):
        self._artist = artist
        self.artistChanged.emit(artist)

    @pyqtProperty('QString', notify=coverChanged)
    def cover(self):
        return self._cover

    @cover.setter
    def cover(self, cover):
        self._cover = cover
        self.coverChanged.emit(cover)

class DRealLocalMediaContent(BaseMediaContent):

    def __init__(self, url, tags=None):
        super(DRealLocalMediaContent, self).__init__(url)
        if tags:
            self._tags = tags
        else:
            self._tags = Song(url)
        self.title = self._tags['title']
        self.artist = self._tags['artist']

    @pyqtProperty(dict)
    def tags(self):
        return self._tags

    @tags.setter
    def tags(self, value):
        self._tags = value


class DRealOnlineMediaContent(BaseMediaContent):

    tagsUpdated = pyqtSignal(dict)
    playlinkChanged = pyqtSignal('QString')

    tagsUpdateSuccessed = pyqtSignal('QString')

    __keys__ = {
        'title': 'songName',
        'artist': 'singerName',
        'album': 'albumName',
        'albumImage_100x100': 'albumImage_100x100',
        'albumImage_500x500': 'albumImage_500x500',

        'songId': 'songId',
        'singerId': 'singerId',
        'albumId': 'albumId',

        'serviceEngName': 'serviceEngName',
        'serviceName': 'serviceName',
        'serviceUrl': 'serviceUrl',

        'playlinkUrl': 'playlinkUrl'
    }

    def __init__(self, url, onlineTags=None):
        super(DRealOnlineMediaContent, self).__init__(url)
        self._palyLinkUrl = ''
        self._onlineTags = onlineTags
        self._tags = {}
        self.tagsUpdated.connect(self.updateTags)
        if onlineTags:
            self.updateTags(onlineTags)
        else:
            self.updateTagsByUrl(url)

    def updateTags(self, tags):
        for key in self.__keys__:
            k = self.__keys__[key]
            if k in tags:
                self._tags[key] = tags[k]
        self.title = self._tags['title']
        self.artist = self._tags['artist']
        self.playlinkUrl = self._tags['playlinkUrl']

    @dthread
    @pyqtSlot(int)
    def updateTagsByUrl(self, url):
        import requests
        ret = requests.get(url)
        tags = ret.json()
        self.tagsUpdated.emit(tags)

    @pyqtProperty(dict)
    def tags(self):
        return self._onlineTags

    @tags.setter
    def tags(self, value):
        self._onlineTags = value

    @pyqtProperty('QString')
    def playlinkUrl(self):
        return self._palyLinkUrl

    @playlinkUrl.setter
    def playlinkUrl(self, link):
        self._palyLinkUrl = link
        self.playlinkChanged.emit(link)
    

class DLocalMediaContent(QMediaContent):

    def __init__(self, url, ret=None):
        super(DLocalMediaContent, self).__init__(QUrl.fromLocalFile(url))
        self._url = url
        self.content = DRealLocalMediaContent(url, ret)

    @pyqtProperty('QString')
    def url(self):
        return self._url


class DOnlineMediaContent(QMediaContent):

    def __init__(self, url, tags=None):
        super(DOnlineMediaContent, self).__init__(QUrl(url))
        self_url = url
        self.content = DRealOnlineMediaContent(url, tags)

    @pyqtProperty('QString')
    def url(self):
        return self._url

class DMediaPlaylist(QMediaPlaylist):

    nameChanged = pyqtSignal('QString')

    mediasChanged = pyqtSignal('QVariant')

    def __init__(self, name=''):
        super(DMediaPlaylist, self).__init__()
        self._name = name
        self._urls = []
        self._mediaContents = OrderedDict()

    @pyqtProperty('QString')
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        self._name = name
        self.nameChanged.emit()

    @pyqtProperty('QVariant')
    def urls(self):
        return self._urls

    @pyqtProperty('QVariant')
    def mediaContents(self):
        return self._mediaContents

    @pyqtProperty('QVariant', notify=mediasChanged)
    def medias(self):
        medias = []
        for key in self._urls:
            mediaContent = self._mediaContents[key]
            medias.append(mediaContent)
        return medias

    def removeMedia(self, index):
        flag = super(DMediaPlaylist, self).removeMedia(index)
        if flag:
            url = self._urls[index]
            self._urls.pop(index)
            self._mediaContents.pop(url)

    def addMedia(self, url, ret=None):
        url = str(url)
        if url not in self._urls:
            if url.startswith('http://') or url.startswith('https://'):
                self.addOnlineMedia(url, ret)
                self._urls.append(url)
                medias = self.medias
                self.mediasChanged.emit(medias)
            else:
                if os.path.exists(url):
                    self.addLocalMedia(url, ret)
                    self._urls.append(url)
                    medias = self.medias
                    self.mediasChanged.emit(medias)
        if url in self._urls:
            index = self._urls.index(url)
            self.setCurrentIndex(index)

    def addLocalMedia(self, url, ret):
        mediaContent = DLocalMediaContent(url, ret)
        content = mediaContent.content
        self._mediaContents.update({url: content})
        super(DMediaPlaylist, self).addMedia(mediaContent)

    def addOnlineMedia(self, url, ret):
        mediaContent = DOnlineMediaContent(url, ret)
        content = mediaContent.content
        self._mediaContents.update({url: content})
        super(DMediaPlaylist, self).addMedia(mediaContent)


class PlaylistWorker(QObject):

    __contextName__ = 'PlaylistWorker'

    nameExisted = pyqtSignal('QString')

    @registerContext
    def __init__(self, parent=None):
        super(PlaylistWorker, self).__init__(parent)

        self._playlists = OrderedDict()
        self._currentPlaylist = None

        self.createPlaylistByName('temporary')
        self.createPlaylistByName('favorite')

        self.initPlaylist()

    def initPlaylist(self):
        self.loadPlaylists()

    def savePlaylists(self):
        result = OrderedDict()
        for name, playlist in self._playlists.items():
            _playlist = OrderedDict()
            for  url, mediaContent in playlist.mediaContents.items():
                _playlist[url] = mediaContent.tags
            result[name] = _playlist

        playlistPath = os.path.join(PlaylistPath, 'DeepinMusic3.playlist')
        with open(playlistPath, 'wb') as f:
            json.dump(result, f, indent=4)


    def loadPlaylists(self):
        playlistPath = os.path.join(PlaylistPath, 'DeepinMusic3.playlist')
        if os.path.exists(playlistPath):
            with open(playlistPath, 'r') as f:
                results = json.load(f, object_pairs_hook=OrderedDict)
            for name, _playlist in results.items():
                playlist = self.createPlaylistByName(name)
                for url, tags in _playlist.items():
                    url = url.encode('utf-8')
                    playlist.addMedia(url, tags)

    def savePlaylistByName(self, name):
        f = QFile(os.sep.join([PlaylistPath, '%s.m3u' % name]))
        flag = f.open(QIODevice.ReadWrite)
        if flag:
            self._playlists[name].save(f, 'm3u')
            f.close()

    @pyqtProperty('QMediaPlaylist')
    def termporaryPlaylist(self):
        return self._playlists['temporary']

    @pyqtSlot('QString', result='QMediaPlaylist')
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

    @pyqtSlot('QString', 'QString')
    def addMediaByName(self, name, url):
        if name in self._playlists:
            self._playlists[name].addMedia(url)
        else:
            logger.info("the playlist named %s isn't existed" % name)

    @pyqtSlot('QString')
    def createPlaylistByName(self, name):
        if name in self._playlists:
            self.nameExisted.emit(name)
        else:
            self._playlists[name] = DMediaPlaylist(name)

        return self._playlists[name]
