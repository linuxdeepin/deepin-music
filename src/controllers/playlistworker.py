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
        self.song = Song(url)
        self.title = self.song['title']
        self.artist = self.song['artist']


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

    def __init__(self, url, tags=None):
        super(DRealOnlineMediaContent, self).__init__(url)
        self._palyLinkUrl = ''
        self.song = {}
        self.tagsUpdated.connect(self.updateTags)
        if tags:
            self.updateTags(tags)
        else:
            self.updateTagsByUrl(url)

    def updateTags(self, tags):
        for key in self.__keys__:
            k = self.__keys__[key]
            if k in tags:
                self.song[key] = tags[k]

        self.title = self.song['title']
        self.artist = self.song['artist']
        self.playlinkUrl = self.song['playlinkUrl']

    @dthread
    @pyqtSlot(int)
    def updateTagsByUrl(self, url):
        import requests
        ret = requests.get(url)
        tags = ret.json()
        self.tagsUpdated.emit(tags)

    @pyqtProperty('QString')
    def playlinkUrl(self):
        return self._palyLinkUrl

    @playlinkUrl.setter
    def playlinkUrl(self, link):
        self._palyLinkUrl = link
        self.playlinkChanged.emit(link)
    

class DLocalMediaContent(QMediaContent):

    def __init__(self, url):
        super(DLocalMediaContent, self).__init__(QUrl.fromLocalFile(url))
        self._url = url
        self.content = DRealLocalMediaContent(url)

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
        self._mediaContents = {}

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
                    self.addLocalMedia(url)
                    self._urls.append(url)
                    medias = self.medias
                    self.mediasChanged.emit(medias)
        if url in self._urls:
            index = self._urls.index(url)
            self.setCurrentIndex(index)

    def addLocalMedia(self, url):
        mediaContent = DLocalMediaContent(url)
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
        self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', '游鸿明 - 往快乐逃.flac']))
        self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', 'Track01.wav']))
        self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', '游鸿明-下沙.ape']))
        self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', '游鸿明 - 红糖水.flac']))
        self.addMediaToTemporary(os.sep.join([os.path.dirname(os.getcwd()), 'music', '1.mp3']))
        self.addMediaToTemporary(os.sep.join(['/usr/share/deepin-sample-music/邓入比_我们的情歌.mp3']))
        self.addMediaToTemporary(os.sep.join(['/usr/share/deepin-sample-music/郭一凡_说走就走的旅行.mp3']))
        self.addMediaToTemporary(os.sep.join(['/usr/share/deepin-sample-music/胡彦斌_依然是你.mp3']))


        
        self.addMediaToFavorite(os.sep.join(['/usr/share/deepin-sample-music/邓入比_我们的情歌.mp3']))
        self.addMediaToFavorite(os.sep.join(['/usr/share/deepin-sample-music/郭一凡_说走就走的旅行.mp3']))
        self.addMediaToFavorite(os.sep.join([os.path.dirname(os.getcwd()), 'music', '1.mp3']))
        self.addMediaToFavorite(os.sep.join(['/usr/share/deepin-sample-music/胡彦斌_依然是你.mp3']))

    def savePlaylists(self):
        result = OrderedDict()

        for name in self._playlists:
            result['name'] = name
            result['']

        with open('DeepinMusic3.playlist', 'wb') as f:
            json.dump(self._d)


    def loadPlaylists(self):
        pass


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
