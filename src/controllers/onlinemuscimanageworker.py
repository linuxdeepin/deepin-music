#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import time
import json
import datetime
from PyQt5.QtCore import (QObject, pyqtSignal,
                pyqtSlot, pyqtProperty, QDir, 
                QDirIterator, QTimer, QThread, QRunnable,
                QThreadPool, QAbstractListModel, Qt, QModelIndex, QVariant)
from PyQt5.QtGui import QImage
from PyQt5.QtQml import QJSValue
from PyQt5.QtWidgets import QFileDialog
from .utils import registerContext, contexts
from dwidgets.tornadotemplate import template
from models import *
from dwidgets import dthread, LevelJsonDict
from dwidgets.mediatag.song import Song as SongDict
from collections import OrderedDict
from UserList import UserList
from config.constants import LevevDBPath, CoverPath, MusicManagerPath
from .coverworker import CoverWorker

from dwidgets import DListModel, ModelMetaclass
from .web360apiworker import Web360ApiWorker


class QmlOnlineSongObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('url', 'QString'),
        ('title', 'QString'),
        ('artist', 'QString'),
        ('album', 'QString'),
        ('tracknumber', int),
        ('discnumber', int),
        ('genre', 'QString'),
        ('date', 'QString'),
        ('size', int),
        ('mediaType', 'QString'),
        ('duration', int),
        ('bitrate', int),
        ('sample_rate', int),
        ('cover', 'QString'),
        ('created_date', 'QString'),

        ('songId', int),
        ('singerId', int),
        ('albumId', int),

        ('serviceEngName', 'QString'),
        ('serviceName', 'QString'),
        ('serviceUrl', 'QString'),

        ('albumImage_100x100', 'QString'),
        ('albumImage_500x500', 'QString'),
        ('playlinkUrl', 'QString'),

        ('created_date', float),
    )

    coverReady = pyqtSignal('QString')

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)
        self.cover = CoverWorker.getOnlineCover(self.title, self.artist, self.album)
        self.coverChanged.connect(self.checkCover)

    def checkCover(self, cover):
        self.coverReady.emit(cover)

    def getCover(self):
        _cover = CoverWorker.getOnlineCover(self.title, self.artist, self.album)
        self.cover = _cover
        self.coverReady.emit(_cover)


class RequestSongRunnable(QRunnable):

    def __init__(self, worker, url):
        super(RequestSongRunnable, self).__init__()
        self.worker = worker
        self.url = url

    def run(self):
        result = Web360ApiWorker.request(self.url)
        if result:
            result.update({'url': self.url})
            self.worker.updateSongSignal.emit(result)


class OnlineMusicManageWorker(QObject):

    downloadOnlineSongCover = pyqtSignal('QString', 'QString', 'QString')
    downloadOnlineAlbumCover = pyqtSignal('QString', 'QString', 'QString', list)
    downloadAlbumCover = pyqtSignal('QString', 'QString')

    updateSongSignal = pyqtSignal(dict)
    updateAlbumSongSignal = pyqtSignal(dict)

    _songObjs = OrderedDict()

    _songsDict = LevelJsonDict(os.path.join(LevevDBPath, 'onlineSongs'))


    __keys__ = {
        'url': 'url',
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

    def __init__(self):
        super(OnlineMusicManageWorker, self).__init__()
        self._songsDict.open()
        self._albums = {}
        self.initConnect()
        self.loadDB()

    def initConnect(self):
        self.updateSongSignal.connect(self.updateSongAndDownloadCover)

    def loadDB(self):
        for url, _songDict in self._songsDict.items():
            self._songObjs[url] = QmlOnlineSongObject(**_songDict)

    @classmethod
    def getSongObjByUrl(cls, url):
        if url in cls._songObjs:
            return cls._songObjs[url]
        else:
            return None

    @classmethod
    def getSongDictByUrl(cls, url):
        if url in cls._songsDict:
            return cls._songsDict[url]
        else:
            return None

    def addSong(self, media):
        _songDict = self.updateTags(media)
        url = media['url']
        self._songsDict[url] = _songDict
        songObj  = QmlOnlineSongObject(**_songDict)
        self._songObjs[url] = songObj

        if not songObj.playlinkUrl:
            d = RequestSongRunnable(self, url)
            QThreadPool.globalInstance().start(d)
        else:
            self.downloadCover(_songDict)

    def addSongs(self, medias):
        if self.isMediasBeyondToSameAlbum(medias):
            self._albums[medias[0]['albumName']] = medias
        for media in medias:
            self.addSong(media)

    def isMediasBeyondToSameAlbum(self, medias):
        _m = set()
        for media in medias:
            if 'albumName' in media:
                _m.add(media['albumName'])
        if len(_m) == 1:
            return True
        else:
            return False

    def updateTags(self, media):
        _songDict = {}
        for key in self.__keys__:
            _key = self.__keys__[key]
            if _key in media:
                _songDict[key] = media[_key]
        return _songDict

    def downloadCover(self, media):
        url = media['url']
        artist = media['artist']
        title = media['title']
        album = media['album']
        if album not in self._albums:
            if not CoverWorker.isOnlineSongCoverExisted(artist, title):
                albumImage_500x500 = media['albumImage_500x500']
                albumImage_100x100 = media['albumImage_100x100']
                if albumImage_100x100:
                    self.downloadOnlineSongCover.emit(artist, title, albumImage_100x100, )
                elif albumImage_500x500:
                    self.downloadOnlineSongCover.emit(artist, title, albumImage_500x500)
                else:
                    self.downloadAlbumCover.emit(artist, album)
        else:
            if not CoverWorker.isOnlineSongCoverExisted(artist, title):
                albumImage_500x500 = media['albumImage_500x500']
                albumImage_100x100 = media['albumImage_100x100']
                if albumImage_100x100:
                    self.downloadOnlineAlbumCover.emit(artist, title, albumImage_100x100, self._albums[album])
                elif albumImage_500x500:
                    self.downloadOnlineAlbumCover.emit(artist, title, albumImage_500x500, self._albums[album])
                else:
                    self.downloadAlbumCover.emit(artist, album)
            else:
                self.updateSongCover(artist, title, CoverWorker.onlineSongCoverPath(artist, title))

    def updateSongAndDownloadCover(self, result):
        _songDict = self.updateTags(result)
        url = result['url']
        if url in self._songsDict:
            self._songsDict[url] = _songDict
        if url in self._songObjs:
            self._songObjs[url].setDict(_songDict)
        self.downloadCover(_songDict)

    def updateSongCover(self, artist, albumtitle, coverUrl):
        from mediaplayer import mediaPlayer
        if mediaPlayer.playlist:
            dlistModel = mediaPlayer.playlist._medias
        else:
            dlistModel = None
        for url , songObj in  self._songObjs.items():
            _songDict = self._songsDict[url]
            if (songObj.title == albumtitle or songObj.album == albumtitle) and songObj.artist == artist:
                cover = coverUrl
                _songDict['cover'] = cover
                self._songsDict[url] = _songDict

                if dlistModel and songObj in dlistModel.data:
                    index = dlistModel.data.index(songObj)
                    dlistModel.setProperty(index, 'cover', cover)

onlineMusicManageWorker = OnlineMusicManageWorker()
