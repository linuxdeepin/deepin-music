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
from .onlinemuscimanageworker import QmlOnlineSongObject, OnlineMusicManageWorker
from .signalmanager import signalManager
from .downloadsongworker import DownloadSongWorker


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
    urlChanged = pyqtSignal('QString')

    def __init__(self, name=''):
        super(DMediaPlaylist, self).__init__()
        self._name = name
        self._url = ''
        self._urls = []
        self._medias = DListModel(QmlSongObject)

        self.currentIndexChanged.connect(self.updateUrl)

    @pyqtProperty('QString', notify=nameChanged)
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        self._name = name
        self.nameChanged.emit()

    @pyqtProperty('QString', notify=urlChanged)
    def url(self):
        return self._url

    @url.setter
    def url(self, value):
        self._url = value
        self.urlChanged.emit(value)

    def updateUrl(self, index):
        if index < len(self._urls):
            self.url = self._urls[index]

    @pyqtProperty(DListModel, notify=mediasChanged)
    def medias(self):
        return self._medias

    @medias.setter
    def medias(self, medias):
        self._medias = medias
        self.mediasChanged.emit(medias)

    @pyqtProperty('QVariant')
    def urls(self):
        return self._urls

    @pyqtProperty(int, notify=countChanged)
    def count(self):
        return self.mediaCount()

    @pyqtSlot()
    def clearMedias(self):
        flag = self.clear()
        if flag:
            self._urls = []
            self._medias.clear()

    def addMedia(self, url):
        if url not in self._urls:
            if url.startswith('http'):
                self.addOnlineMedia(url)
            else:
                if os.path.exists(url):
                    self.addLocalMedia(url)

    def addMedias(self, urls):
        for url in urls:
            self.addMedia(url)

    def addLocalMedia(self, url):
        mediaContent = DLocalMediaContent(url)
        songObj = MusicManageWorker.getSongObjByUrl(url)
        if songObj:
            self._medias.append(songObj)
        super(DMediaPlaylist, self).addMedia(mediaContent)
        self._urls.append(url)

    def insertLocalMedia(self, index, url):
        mediaContent = DLocalMediaContent(url)
        songObj = MusicManageWorker.getSongObjByUrl(url)
        if songObj:
            self._medias.insert(index, songObj)
        else:
            songObj = MusicManageWorker.generateSongObjByUrl(url)
            self._medias.insert(index, songObj)

        super(DMediaPlaylist, self).insertMedia(index, mediaContent)
        self._urls.insert(index, url)

    def addOnlineMedia(self, url):
        mediaContent = DOnlineMediaContent(url)
        songObj = OnlineMusicManageWorker.getSongObjByUrl(url)
        if songObj:
            self._medias.append(songObj)
        super(DMediaPlaylist, self).addMedia(mediaContent)
        self._urls.append(url)

    def removeMediaByUrl(self, url):
        if url in self._urls:
            index = self._urls.index(url)
            self._urls.remove(url)
            self.removeMedia(index)
            if index < self._medias.count:
                self._medias.remove(index)

    def insertMediaByUrl(self, onlineUrl, localUrl):
        for index, url in enumerate(self._urls):
            if url == onlineUrl:
                self.removeMediaByUrl(onlineUrl)
                self.insertLocalMedia(index, localUrl)


class PlaylistWorker(QObject):

    __contextName__ = 'PlaylistWorker'

    nameExisted = pyqtSignal('QString')
    playlistNamesChanged = pyqtSignal('QVariant')
    allPlaylistNamesChanged = pyqtSignal('QVariant')
    currentPlaylistChanged = pyqtSignal('QString')

    _playlists = OrderedDict()

    @registerContext
    def __init__(self, parent=None):
        super(PlaylistWorker, self).__init__(parent)

        self._playlistNames = []
        self._currentPlaylist = None

        self.emptyListModel = DListModel(QmlSongObject)
        registerObj('EmptyModel', self.emptyListModel)

        self.initConnect()

    def initConnect(self):
        signalManager.addtoFavorite.connect(self.addToFavorite)
        signalManager.removeFromFavorite.connect(self.removeFromFavorite)
        signalManager.switchOnlinetoLocal.connect(self.updateAllPlaylist)
        signalManager.addAlltoDownloadlist.connect(self.downloadPlaylist)

        signalManager.addNewPlaylist.connect(self.createPlaylistByName)
        signalManager.addSongsToMultiPlaylist.connect(self.addSongsToMultiPlaylist)

    def savePlaylists(self):
        result = OrderedDict()
        for name, playlist in self._playlists.items():
            _urls = playlist.urls
            result[name] = _urls
        playlistPath = os.path.join(PlaylistPath, 'DeepinMusic3.playlist')
        with open(playlistPath, 'wb') as f:
            json.dump(result, f, indent=4)

    def loadPlaylists(self):
        playlistPath = os.path.join(PlaylistPath, 'DeepinMusic3.playlist')
        if os.path.exists(playlistPath):
            with open(playlistPath, 'r') as f:
                results = json.load(f, object_pairs_hook=OrderedDict)
            for name, urls in results.items():
                playlist = self.createPlaylistByName(name)
                playlist.addMedias(urls)
        else:
            self.createPlaylistByName('temporary')
            self.createPlaylistByName('favorite')

    def savePlaylistByName(self, name):
        f = QFile(os.sep.join([PlaylistPath, '%s.m3u' % name]))
        flag = f.open(QIODevice.ReadWrite)
        if flag:
            self._playlists[name].save(f, 'm3u')
            f.close()

    @classmethod
    def md5(cls, value):
        import hashlib
        s = '%s' % (value)
        md5Value = hashlib.md5(s.encode('utf-8'))
        return md5Value.hexdigest()

    def setContext(self, name, obj):
        signalManager.registerQmlObj.emit('Playlist_%s' % self.md5(name), obj)

    @pyqtSlot('QString', result=DListModel)
    def getMediasByName(self, name):
        if name in self._playlists:
            return self._playlists[name]._medias

    @pyqtSlot('QString', result=bool)
    def isFavorite(self, url):
        if 'favorite' in self._playlists:
            playlist =  self._playlists['favorite']
            if url in playlist.urls:
                return True
            else:
                return False
        else:
            return False

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

    @pyqtProperty('QVariant', notify=allPlaylistNamesChanged)
    def allPlaylistNames(self):
        _names = [{'name': 'favorite'}, {'name': 'temporary'}] + self._playlistNames
        return _names
  
    @pyqtSlot('QString')
    def createPlaylistByName(self, name):
        names = self._playlistNames + ['favorite', 'temporary']
        if name in self._playlists:
            self.nameExisted.emit(name)
        else:
            self._playlists[name] = DMediaPlaylist(name)
            self.setContext(name, self._playlists[name]._medias)
            if name not in ['favorite', 'temporary']:
                self._playlistNames.insert(0, {'name': name})
                self.playlistNames = self._playlistNames
        return self._playlists[name]

    @pyqtSlot('QString')
    def deletePlaylist(self, name):
        self._playlistNames.remove({'name': name})
        self._playlists.pop(name)
        self.playlistNamesChanged.emit(self._playlistNames)

    @pyqtSlot('QString', result='QVariant')
    def getUrlsByName(self, name):
        if name in self._playlists:
            return self._playlists[name].urls

    def getLocalUrlforOnline(self, media):
        url = media['url']
        artist = media['singerName']
        title = media['songName']
        flag, path = MusicManageWorker.isSongExistedInDataBase(artist, title)
        if flag and path:
            return path
        else:
            return url

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
        url = self.getLocalUrlforOnline(media)
        playlist.addMedia(url)

    def addOnlineMediasToTemporary(self, medias):
        playlist = self.temporaryPlaylist
        self.currentPlaylistChanged.emit('temporary')
        urls = []
        for media in medias:
            url = self.getLocalUrlforOnline(media)
            urls.append(url)
        playlist.addMedias(urls)

    def addOnlineMediaToFavorite(self, media):
        url = self.getLocalUrlforOnline(media)
        self.addToFavorite(url)

    def addToFavorite(self, url):
        playlist = self.favoritePlaylist
        playlist.addMedia(url)

    def removeFromFavorite(self, url):
        playlist = self.favoritePlaylist
        playlist.removeMediaByUrl(url)

    def removeFavoriteMediaContent(self, url):
        self.removeFromFavorite(url)

    def removeFromPlaylist(self, playlistName, url):
        if playlistName in self._playlists:
            playlist =  self._playlists[playlistName]
            playlist.removeMediaByUrl(url)

    def addSongToPlaylist(self, url, playlistName):
        if playlistName in self._playlists:
            playlist =  self._playlists[playlistName]
            if playlistName == 'favorite':
                signalManager.addtoFavorite.emit(url)
            else:
                playlist.addMedia(url)

    def addSongsToPlaylist(self, value, playlistName, _type):
        if playlistName in self._playlists:
            playlist =  self._playlists[playlistName]
            if _type == "Artist":
                urls = MusicManageWorker.getUrlsByArtist(value)
                playlist.addMedias(urls)
            elif _type == 'Album':
                urls = MusicManageWorker.getUrlsByAlbum(value)
                playlist.addMedias(urls)
            elif _type == "Folder":
                urls = MusicManageWorker.getUrlsByFolder(value)
                playlist.addMedias(urls)

    def updateAllPlaylist(self, onlineUrl, localUrl):
        for name, playlist in self._playlists.items():
            playlist.insertMediaByUrl(onlineUrl, localUrl)

    def downloadPlaylist(self, playlistName):
        playlist = self._playlists[playlistName]
        for obj in playlist.medias.data:
            if isinstance(obj, QmlOnlineSongObject):
                signalManager.addtoDownloadlist.emit(obj.songId)

    def addSongsToMultiPlaylist(self, _id, _type, flags):
        for index, flag in enumerate(flags):
            if flag:
                playlistName = self.allPlaylistNames[index]['name']
                if _type in ['Artist', 'Album', 'Folder']:
                    self.addSongsToPlaylist(_id, playlistName, _type)
                else:
                    self.addSongToPlaylist(_id, playlistName)


playlistWorker = PlaylistWorker()
