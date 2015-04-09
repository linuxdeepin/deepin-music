#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import copy
import json
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QThreadPool
from PyQt5.QtGui import QImage
import requests
from log import logger
from .utils import registerContext
from dwidgets import dthread, CoverRunnable
from config.constants import ArtistCoverPath, AlbumCoverPath, SongCoverPath
import urllib


class CoverWorker(QObject):

    __contextName__ = "CoverWorker"

    downloadCoverSuccessed = pyqtSignal('QString', 'QString')

    downloadArtistCoverSuccessed = pyqtSignal('QString', 'QString')
    downloadAlbumCoverSuccessed = pyqtSignal('QString', 'QString', 'QString')

    allTaskFinished = pyqtSignal()

    updateArtistCover = pyqtSignal('QString', 'QString')
    updateAlbumCover = pyqtSignal('QString', 'QString', 'QString')


    defaultArtistCover = os.path.join(os.getcwd(), 'skin', 'images','bg1.jpg')
    defaultAlbumCover = os.path.join(os.getcwd(), 'skin', 'images','bg2.jpg')
    defaultSongCover = os.path.join(os.getcwd(), 'skin', 'images','bg3.jpg')

    @registerContext
    def __init__(self, parent=None):
        super(CoverWorker, self).__init__(parent)
        QThreadPool.globalInstance().setMaxThreadCount(4)
        self.artistCovers = {}
        self.albumCovers = {}
        self.taskNumber = 0
        self.initConnect()

    def initConnect(self):
        self.downloadArtistCoverSuccessed.connect(self.cacheArtistCover)
        self.downloadAlbumCoverSuccessed.connect(self.cacheAlbumCover)

    @classmethod
    def md5(cls, string):
        import hashlib
        md5Value = hashlib.md5(string)
        return md5Value.hexdigest()

    @classmethod
    def getCoverPathByMediaUrl(cls, url):
        if isinstance(url, unicode):
            url = url.encode('utf-8')
        coverID = cls.md5(url)
        filename = '%s' % coverID
        filepath = os.path.join(SongCoverPath, filename)
        return filepath

    @pyqtSlot('QString', 'QString')
    @dthread
    def downloadCoverByUrl(self, mediaUrl, coverUrl):
        filepath = self.getCoverPathByMediaUrl(mediaUrl)
        try:
            r = requests.get(coverUrl)
            with open(filepath, "wb") as f:
                f.write(r.content)
            self.downloadCoverSuccessed.emit(mediaUrl, filepath)
        except:
            pass

    def cacheArtistCover(self, artist, url):
        self.artistCovers[artist]  = url
        self.updateArtistCover.emit(artist, url)
        self.taskNumber -= 1
        if self.taskNumber == 0 and QThreadPool.globalInstance().activeThreadCount() == 0:
            self.allTaskFinished.emit()

    def cacheAlbumCover(self, artist, album, url):
        self.albumCovers[album] = url
        self.updateAlbumCover.emit(artist, album, url)
        self.taskNumber -= 1
        if self.taskNumber == 0 and QThreadPool.globalInstance().activeThreadCount() == 0:
            self.allTaskFinished.emit()

    def downloadArtistCover(self, artist):
        f = self.artistCoverPath(artist)
        if os.path.exists(f):
            return
        if ',' in artist:
            artist = artist.split(',')
            for item in artist:
                self.taskNumber += 1
                d = CoverRunnable(self, item, qtype="artist")
                QThreadPool.globalInstance().start(d)
                
        else:
            self.taskNumber += 1
            d = CoverRunnable(self, artist, qtype="artist")
            QThreadPool.globalInstance().start(d)

    def downloadAlbumCover(self, artist, album):
        f = self.albumCoverPath(artist, album)
        if os.path.exists(f):
            return
        self.taskNumber += 1
        d = CoverRunnable(self, artist, album, qtype="album")
        QThreadPool.globalInstance().start(d)

    @classmethod
    def getCoverPathByArtist(cls, artist):
        if ',' in artist:
            artist = artist.split(',')[0]
        filepath = os.path.join(ArtistCoverPath, artist)
        if isinstance(artist, unicode):
            artist = artist.encode('utf-8')
        encodefilepath = os.path.join(ArtistCoverPath, urllib.quote(artist))
        if os.path.exists(filepath):
            image = QImage()
            if image.load(filepath):
                return encodefilepath
            else:
                return cls.defaultArtistCover
        else:
            return cls.defaultArtistCover

    @classmethod
    def getCoverPathByArtistAlbum(cls, artist, album):
        filename = '%s-%s' % (artist, album)
        filepath = os.path.join(AlbumCoverPath, filename)
        if isinstance(filename, unicode):
            filename = filename.encode('utf-8')
        encodefilepath = os.path.join(AlbumCoverPath, urllib.quote(filename))
        if os.path.exists(filepath):
            image = QImage()
            if image.load(filepath):
                return encodefilepath
            else:
                return cls.defaultAlbumCover
        else:
            return cls.defaultAlbumCover

    @classmethod
    def getCoverPathByArtistSong(cls, artist, title):
        filepath = os.path.join(SongCoverPath, '%s-%s' % (artist, title))
        if os.path.exists(filepath):
            image = QImage()
            if image.load(filepath):
                return filepath
            else:
                return cls.defaultSongCover
        else:
            return cls.defaultSongCover

    @classmethod
    def artistCoverPath(cls, artist):
        filepath = os.path.join(ArtistCoverPath, artist)
        return filepath

    @classmethod
    def isArtistCoverExisted(cls, artist):
        return os.path.exists(cls.artistCoverPath(artist))

    @classmethod
    def albumCoverPath(cls, artist, album):
        filepath = os.path.join(AlbumCoverPath, '%s-%s' % (artist, album))
        return filepath
    @classmethod
    def isAlbumCoverExisted(cls, artist, album):
        return os.path.exists(cls.albumCoverPath(artist, album))

    @classmethod
    def songCoverPath(cls, artist, title):
        filepath = os.path.join(SongCoverPath, '%s-%s' % (artist, title))
        return filepath

    @classmethod
    def isSongCoverExisted(cls, artist, title):
        return os.path.exists(cls.songCoverPath(artist, title))
