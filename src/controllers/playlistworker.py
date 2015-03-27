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

from .utils import registerContext, contexts, duration_to_string
from dwidgets.mediatag.song import Song
from dwidgets import dthread
from .coverworker import CoverWorker
from config.constants import PlaylistPath
from config.constants import CoverPath
from log import logger


class PlayerBin(QMediaPlayer):

    def __init__(self):
        super(PlayerBin, self).__init__()
        self.setNotifyInterval(50)


class BaseMediaContent(QObject):

    titleChanged = pyqtSignal('QString')
    artistChanged = pyqtSignal('QString')
    coverChanged = pyqtSignal('QString')
    coverDownloadSuccessed = pyqtSignal('QString')
    durationChanged = pyqtSignal('QString')

    def __init__(self, url):
        super(BaseMediaContent, self).__init__()
        self._url = url
        self._title = ''
        self._artist = ''
        self._cover = ''
        self._duration = '00:00'

    def _initConnect(self):
        self.coverDownloadSuccessed.connect(self.setCover)

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

    @pyqtProperty('QString', notify=durationChanged)
    def duration(self):
        return self._duration

    @duration.setter
    def duration(self, value):
        self._duration = value
        self.durationChanged.emit(value)

    @pyqtSlot('QString')
    def setCover(self, cover):
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
        self.duration = duration_to_string(self._tags['duration'])

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

        'playlinkUrl': 'playlinkUrl',
        'duration': 'duration'
    }

    def __init__(self, url, onlineTags=None):
        super(DRealOnlineMediaContent, self).__init__(url)
        self._palyLinkUrl = ''
        self._onlineTags = onlineTags
        self._tags = {}

        self.player = PlayerBin()
        self.player.setMuted(True)
        self.player.mediaStatusChanged.connect(self.monitorMediaStatus)

        self.tagsUpdated.connect(self.updateTags)
        if onlineTags:
            self.updateTags(onlineTags)
        else:
            # QTimer.singleShot(5000, self.updateTagsByUrl)
            pass

    def updateTags(self, tags):
        for key in self.__keys__:
            k = self.__keys__[key]
            if k in tags:
                self._tags[key] = tags[k]
        self.title = self._tags['title']
        self.artist = self._tags['artist']

        if 'duration' in self._tags:
            self.duration = duration_to_string(self._tags['duration'])

        coverfile = CoverWorker.getCoverPathByMediaUrl(self._url)
        if os.path.exists(coverfile):
            self.cover = coverfile
        elif 'albumImage_500x500' in tags and tags['albumImage_500x500']:
            self.cover = tags['albumImage_500x500']
        elif 'albumImage_100x100' in tags and tags['albumImage_100x100']:
            self.cover = tags['albumImage_100x100']
        # else:
            # print('********')

        if 'playlinkUrl' in self._tags:
            self.playlinkUrl = self._tags['playlinkUrl']
            # if self.duration == u"00:00":
            #     QTimer.singleShot(5000, self.updateDuration)

        self.tags.update(tags)

    def updateDuration(self):
        if self.playlinkUrl:
            self.setMedia(self.playlinkUrl)

    @dthread
    @pyqtSlot('QString')
    def updateTagsByUrl(self, url=''):
        url = self.url
        import requests
        maxQueryCount = 5
        i = 0
        while i < maxQueryCount:
            try:
                ret = requests.get(url)
                tags = ret.json()
                if isinstance(tags, list):
                    i = i + 1
                else:
                    break
            except:
                i = i + 1
        if isinstance(tags, dict):
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

    def getCover(self):
        if 'albumImage_500x500' in self.tags and self.tags['albumImage_500x500']:
            url = self.tags['albumImage_500x500']
        elif 'albumImage_100x100' in self.tags and self.tags['albumImage_100x100']:
            url = self.tags['albumImage_100x100']
        title = self.tags['songName']
        artist = self.tags['singerName']
        filepath = CoverWorker.getLocalCoverPath(url, title, artist)
        return filepath

    def setMedia(self, url):
        self.player.setMedia(QMediaContent(QUrl(url)))

    def monitorMediaStatus(self, status):
        if status == 3:
            duration = self.player.duration()
            self.tags['duration'] = duration
            self.duration = duration_to_string(duration)
            # self.player.deleteLater()


class DLocalMediaContent(QMediaContent):

    def __init__(self, url, tags=None):
        super(DLocalMediaContent, self).__init__(QUrl.fromLocalFile(url))
        self._url = url
        self.content = DRealLocalMediaContent(url, tags)

    @pyqtProperty('QString')
    def url(self):
        return self._url

    @pyqtProperty('QString')
    def type(self):
        return 'DLocalMediaContent'


class DOnlineMediaContent(QMediaContent):

    def __init__(self, url, tags=None):
        super(DOnlineMediaContent, self).__init__(QUrl(url))
        self_url = url
        self.content = DRealOnlineMediaContent(url, tags)

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
        self._medias = []
        self._mediaContents = OrderedDict()

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

    @pyqtProperty('QVariant')
    def mediaContents(self):
        return self._mediaContents

    @pyqtProperty(int, notify=countChanged)
    def count(self):
        return self.mediaCount()

    @pyqtProperty('QVariant', notify=mediasChanged)
    def medias(self):
        return self._medias

    @medias.setter
    def medias(self, medias):
        self._medias = medias
        self.mediasChanged.emit(self._medias)
        self.countChanged.emit(self.mediaCount())

    @pyqtSlot()
    def clearMedias(self):
        count = self.mediaCount()
        flag = self.clear()
        if flag:
            self._urls = []
            self._mediaContents = OrderedDict()
            self.medias = []

    def removeMediaByIndex(self, index):
        flag = super(DMediaPlaylist, self).removeMedia(index)
        if flag:
            url = self._urls[index]
            self._urls.pop(index)
            mediaContent = self._mediaContents.pop(url)
            self._medias.pop(mediaContent)
            self.medias = self._medias

    def emitSignal(self):
        for key in self._urls:
            mediaContent = self._mediaContents[key]
            if mediaContent not in self._medias:
                self._medias.append(mediaContent)
        self.medias = self._medias

    def addMedia(self, url, tags=None, updated=False):
        url = unicode(url)
        if url not in self._urls:
            if url.startswith('http://') or url.startswith('https://'):
                self.addOnlineMedia(url, tags)
                self._urls.append(url)
                self.emitSignal()
            else:
                if os.path.exists(url):
                    self.addLocalMedia(url, tags)
                    self._urls.append(url)
                    self.emitSignal()

        content = self._mediaContents[url]
        if isinstance(content, DRealOnlineMediaContent):
            content.updateTags(tags)
            if updated:
                # QTimer.singleShot(5000, content.updateTagsByUrl)
                pass

    def addMedias(self, medias):
        oldCount = len(self._urls)
        for media in medias:
            url = media['url']
            tags = media['tags'] 
            updated = media['updated']
            url = unicode(url)
            if url not in self._urls:
                if url.startswith('http://') or url.startswith('https://'):
                    self.addOnlineMedia(url, tags)
                    self._urls.append(url)
                else:
                    if os.path.exists(url):
                        self.addLocalMedia(url, tags)
                        self._urls.append(url)

            content = self._mediaContents[url]
            if isinstance(content, DRealOnlineMediaContent):
                content.updateTags(tags)
                if updated:
                    # QTimer.singleShot(5000, content.updateTagsByUrl)
                    pass
        if oldCount < len(self._urls):
            self.emitSignal()

    def updateMedia(self, url, tags, updated=False):
        url = unicode(url)
        if url in self._urls:
            content = self._mediaContents[url]
            content.updateTags(tags)

    def addLocalMedia(self, url, tags):
        mediaContent = DLocalMediaContent(url, tags)
        content = mediaContent.content
        self._mediaContents.update({url: content})
        super(DMediaPlaylist, self).addMedia(mediaContent)

    def addOnlineMedia(self, url, tags):
        mediaContent = DOnlineMediaContent(url, tags)
        content = mediaContent.content
        self._mediaContents.update({url: content})
        super(DMediaPlaylist, self).addMedia(mediaContent)


class PlaylistWorker(QObject):

    __contextName__ = 'PlaylistWorker'

    nameExisted = pyqtSignal('QString')

    playlistNamesChanged = pyqtSignal('QVariant')

    @registerContext
    def __init__(self, parent=None):
        super(PlaylistWorker, self).__init__(parent)

        self._playlists = OrderedDict()
        self._playlistNames = []
        self._currentPlaylist = None

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
        result = OrderedDict()
        for name, playlist in self._playlists.items():
            _playlist = OrderedDict()
            for url, mediaContent in playlist.mediaContents.items():
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
                medias = []
                for url, tags in _playlist.items():
                    medias.append({
                        'url' : url,
                        'tags': tags,
                        'updated': True
                    })
                playlist.addMedias(medias)

    def savePlaylistByName(self, name):
        f = QFile(os.sep.join([PlaylistPath, '%s.m3u' % name]))
        flag = f.open(QIODevice.ReadWrite)
        if flag:
            self._playlists[name].save(f, 'm3u')
            f.close()

    @pyqtProperty('QVariant')
    def temporaryPlaylist(self):
        return self._playlists['temporary']

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
            if name not in ['favorite', 'temporary']:
                self._playlistNames.insert(0, {'name': name})
                self.playlistNames = self._playlistNames

        return self._playlists[name]


    @pyqtSlot('QString', result='QVariant')
    def getMediasByName(self, name):
        if name in self._playlists:
            return self._playlists[name].medias

    def addOnlineMediaToTemporary(self, media):
        playlist = self.temporaryPlaylist
        playlist.addMedia(media['url'], media['tags'], media['updated'])

    def addOnlineMediasToTemporary(self, medias):
        playlist = self.temporaryPlaylist
        playlist.addMedias(medias)
