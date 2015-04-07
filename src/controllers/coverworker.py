#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import copy
import json
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QThreadPool
import requests
from log import logger
from .utils import registerContext
from dwidgets import dthread, CoverRunnable
from config.constants import ArtistCoverPath, AlbumCoverPath, SongCoverPath


class CoverWorker(QObject):

    __contextName__ = "CoverWorker"

    downloadCoverSuccessed = pyqtSignal('QString', 'QString')

    downloadArtistCoverSuccessed = pyqtSignal('QString', 'QString')
    downloadAlbumCoverSuccessed = pyqtSignal('QString', 'QString', 'QString')

    updateArtistCover = pyqtSignal('QString', 'QString')
    updateAlbumCover = pyqtSignal('QString', 'QString', 'QString')

    @registerContext
    def __init__(self, parent=None):
        super(CoverWorker, self).__init__(parent)
        QThreadPool.globalInstance().setMaxThreadCount(4)
        self.artistCovers = {}
        self.albumCovers = {}
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

    def cacheAlbumCover(self, artist, album, url):
        self.albumCovers[album] = url
        self.updateAlbumCover.emit(artist, album, url)

    def downloadArtistCover(self, artist):
        f = self.artistCoverPath(artist)
        if os.path.exists(f):
            return
        d = CoverRunnable(self, artist, qtype="artist")
        QThreadPool.globalInstance().start(d)

    def downloadAlbumCover(self, artist, album):
        f = self.albumCoverPath(artist, album)
        if os.path.exists(f):
            return
        d = CoverRunnable(self, artist, album, qtype="album")
        QThreadPool.globalInstance().start(d)

    def getCoverBySongName(self, name):
        pass

    def getCoverByAlbumName(self, name):
        pass

    def getCoverBySignerName(self, name):
        pass 

    @classmethod
    def getCoverPathByArtist(cls, artist):
        filepath = os.path.join(ArtistCoverPath, artist)
        if os.path.exists(filepath):
            return filepath
        else:
            return os.path.join(os.getcwd(), 'skin', 'images','bg1.jpg')

    @classmethod
    def getCoverPathByArtistAlbum(cls, artist, album):
        filepath = os.path.join(AlbumCoverPath, '%s-%s' % (artist, album))
        if os.path.exists(filepath):
            return filepath
        else:
            return os.path.join(os.getcwd(), 'skin', 'images','bg2.jpg')

    @classmethod
    def getCoverPathByArtistSong(cls, artist, title):
        filepath = os.path.join(SongCoverPath, '%s-%s' % (artist, title))
        if os.path.exists(filepath):
            return filepath
        else:
            return os.path.join(os.getcwd(), 'skin', 'images','bg3.jpg')

    @classmethod
    def artistCoverPath(self, artist):
        filepath = os.path.join(ArtistCoverPath, artist)
        return filepath

    @classmethod
    def albumCoverPath(self, artist, album):
        filepath = os.path.join(AlbumCoverPath, '%s-%s' % (artist, album))
        return filepath

    @classmethod
    def songCoverPath(cls, artist, title):
        filepath = os.path.join(SongCoverPath, '%s-%s' % (artist, title))
        return filepath
