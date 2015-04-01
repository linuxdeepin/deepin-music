#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import time
from PyQt5.QtCore import (QObject, pyqtSignal,
                pyqtSlot, pyqtProperty, QDir, QDirIterator)

from .utils import registerContext, contexts
from dwidgets.tornadotemplate import template
from models import *
from dwidgets import dthread
from dwidgets.mediatag.song import Song as SongDict
from collections import OrderedDict


class MusicManageWorker(QObject):

    categoriesChanged = pyqtSignal('QVariant')
    songCountChanged = pyqtSignal(int)

    searchAllDriver =pyqtSignal()
    scanfileChanged = pyqtSignal('QString')
    searchOneFolder = pyqtSignal()

    saveSongToDB = pyqtSignal(dict)

    artistsChanged = pyqtSignal('QVariant')
    albumsChanged = pyqtSignal('QVariant')
    foldersChanged = pyqtSignal('QVariant')

    __contextName__ = 'MusicManageWorker'

    @registerContext
    def __init__(self, parent=None):
        super(MusicManageWorker, self).__init__(parent)

        self._artists = []
        self._albums = []
        self._folders = []

        self._artistsDict = OrderedDict()
        self._albumsDict = OrderedDict()
        self._foldersDict = OrderedDict()

        self.initConnect()

    def initConnect(self):
        self.searchAllDriver.connect(self.searchAllDriverMusic)
        self.searchOneFolder.connect(self.searchOneFolderMusic)

        self.scanfileChanged.connect(self.updateSonglist)

    @pyqtProperty('QVariant', notify=categoriesChanged)
    def categories(self):
        i18nWorker = contexts['I18nWorker']

        categories = [
            {'name': i18nWorker.artist},
            {'name': i18nWorker.album},
            {'name': i18nWorker.song},
            {'name': i18nWorker.folder}
        ]
        return categories

    @pyqtProperty('QVariant', notify=songCountChanged)
    def songCount(self):
        return Song.select().count()

    @pyqtProperty('QVariant', notify=artistsChanged)
    def artists(self):
        return self._artists 

    @artists.setter
    def artists(self, value):
        self._artists = value
        self.artistsChanged.emit(self._artists)

    @pyqtProperty('QVariant', notify=albumsChanged)
    def albums(self):
        return self._albums 

    @albums.setter
    def albums(self, value):
        self._albums = value
        self.albumsChanged.emit(self._albums)

    @pyqtProperty('QVariant', notify=foldersChanged)
    def folders(self):
        return self._folders 

    @folders.setter
    def folders(self, value):
        self._folders = value
        self.foldersChanged.emit(self._folders)

    @dthread
    def searchAllDriverMusic(self):
        self.scanFolder(QDir.rootPath())

    @dthread
    def searchOneFolderMusic(self):
        print('searchOneFolderMusic')

    def scanFolder(self, path):
        filters = QDir.Files
        nameFilters = ["*.wav", "*.wma", "*.mp2", "*.mp3", "*.mp4", "*.m4a", "*.flac", "*.ogg"]
        qDirIterator = QDirIterator(path, nameFilters, filters, QDirIterator.Subdirectories)
        self._folders = []
        self._folderDict = {}
        while qDirIterator.hasNext():
            qDirIterator.next()
            fileInfo = qDirIterator.fileInfo()
            fdir = fileInfo.absoluteDir().absolutePath()
            fpath = qDirIterator.filePath()
            fsize = fileInfo.size() / (1024 * 1024)
            time.sleep(0.05)
            if fsize >= 1:
                self.scanfileChanged.emit(fpath)
        # self.scanfileChanged.emit('')/

    def updateSonglist(self, fpath):
        songDict = SongDict(fpath)
        url = songDict['url']
        artist = songDict['artist']
        if artist not in self._artistsDict:
            self._artistsDict[artist] = {
                'name': artist,
                'count': 0,
                'urls': []
            }
        _artistDict = self._artistsDict[artist]
        if url not in _artistDict['urls']:
            urls = _artistDict['urls']
            urls.append(url)
            _artistDict['count'] = len(urls)
        self.updateArtists(artist)


        album = songDict['album']
        if album not in self._albumsDict:
            self._albumsDict[album] = {
                'name': album,
                'count': 0,
                'urls': []
            }
        _albumDict = self._albumsDict[album]
        if url not in _albumDict['urls']:
            urls = _albumDict['urls']
            urls.append(url)
            _albumDict['count'] = len(urls)

        self.updateAlbumss(album)


        folder = songDict['folder']
        if folder not in self._foldersDict:
            self._foldersDict[folder] = {
                'name': folder,
                'count': 0,
                'urls': []
            }
        _folderDict = self._foldersDict[folder]
        if url not in _folderDict['urls']:
            urls = _folderDict['urls']
            urls.append(url)
            _folderDict['count'] = len(urls)
        self.updateFolders(folder)

        self.saveSongToDB.emit(songDict)


    def updateArtists(self, artist):
        self.artists = self._artistsDict.values()

    def updateAlbumss(self, album):
        self.albums = self._albumsDict.values()

    def updateFolders(self, folder):
        self.folders = self._foldersDict.values()
