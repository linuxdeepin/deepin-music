#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import time
import json
from PyQt5.QtCore import (QObject, pyqtSignal,
                pyqtSlot, pyqtProperty, QDir, QDirIterator)

from PyQt5.QtWidgets import QFileDialog
from .utils import registerContext, contexts
from dwidgets.tornadotemplate import template
from models import *
from dwidgets import dthread, LevelJsonDict
from dwidgets.mediatag.song import Song as SongDict
from collections import OrderedDict
from config.constants import CoverPath, MusicManagerPath



class MusicManageWorker(QObject):

    #py2py
    scanfileChanged = pyqtSignal('QString')
    saveSongToDB = pyqtSignal(dict)
    addSongsToPlaylist = pyqtSignal(list)

    #property signal
    songsChanged = pyqtSignal('QVariant')
    artistsChanged = pyqtSignal('QVariant')
    albumsChanged = pyqtSignal('QVariant')
    foldersChanged = pyqtSignal('QVariant')
    categoriesChanged = pyqtSignal('QVariant')
    songCountChanged = pyqtSignal(int)

    #py2qml
    tipMessageChanged = pyqtSignal('QString')

    #qml2py
    searchAllDriver =pyqtSignal()
    searchOneFolder = pyqtSignal()
    playArtist = pyqtSignal('QString')
    playAlbum = pyqtSignal('QString')
    playFolder = pyqtSignal('QString')

    __contextName__ = 'MusicManageWorker'

    songsPath = os.path.join(MusicManagerPath, 'songs.json')
    artistsPath = os.path.join(MusicManagerPath, 'artists.json')
    albumsPath = os.path.join(MusicManagerPath, 'albums.json')
    foldersPath = os.path.join(MusicManagerPath, 'folders.json')

    @registerContext
    def __init__(self, parent=None):
        super(MusicManageWorker, self).__init__(parent)
        self.initData()

        self.initConnect()
        self.loadDB()

    def initData(self):
        self._songs = []
        self._artists = []
        self._albums = []
        self._folders = []

        self._songsDict = {}
        self._artistsDict =  {}
        self._albumsDict = {}
        self._foldersDict =  {}

    def initConnect(self):
        self.searchAllDriver.connect(self.searchAllDriverMusic)
        self.searchOneFolder.connect(self.searchOneFolderMusic)
        self.playArtist.connect(self.playArtistMusic)
        self.playAlbum.connect(self.playAlbumMusic)
        self.playFolder.connect(self.playFolderMusic)

        self.scanfileChanged.connect(self.updateSonglist)

    def loadDB(self):
        if os.path.exists(self.songsPath):
            with open(self.songsPath, 'r') as f:
                self._songsDict = json.load(f)

        if os.path.exists(self.artistsPath):
            with open(self.artistsPath, 'r') as f:
                self._artistsDict = json.load(f)

        if os.path.exists(self.albumsPath):
            with open(self.albumsPath, 'r') as f:
                self._albumsDict = json.load(f)

        if os.path.exists(self.foldersPath):
            with open(self.foldersPath, 'r') as f:
                self._foldersDict= json.load(f)
        # self._songsDB = LevelJsonDict('/tmp/songs')

        # self._artistsDB = LevelJsonDict('/tmp/artist')
        # self._albumsDB = LevelJsonDict('/tmp/album')
        # self._foldersDB = LevelJsonDict('/tmp/folder')

        # self.clearDB()
        # self._songsDict.update(self._songsDB)
        # self._artistsDict.update(self._artistsDB)
        # self._albumsDict.update(self._albumsDB)
        # self._foldersDict.update(self._foldersDB)

        self.updateSongs()
        self.updateArtists()
        self.updateAlbumss()
        self.updateFolders()

    def saveDB(self):
        with open(self.songsPath, 'wb') as f:
            json.dump(self._songsDict, f, indent=4)

        with open(self.artistsPath, 'wb') as f:
            json.dump(self._artistsDict, f, indent=4)

        with open(self.albumsPath, 'wb') as f:
            json.dump(self._albumsDict, f, indent=4)

        with open(self.foldersPath, 'wb') as f:
            json.dump(self._foldersDict, f, indent=4)

    def clearDB(self):
        self._songsDB.clear()
        self._artistsDB.clear()
        self._albumsDB.clear()
        self._foldersDB.clear()

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
        return len(self._songsDict)

    @pyqtProperty('QVariant', notify=songsChanged)
    def songs(self):
        return self._songs 

    @songs.setter
    def songs(self, value):
        self._songs = value
        self.songsChanged.emit(self._songs) 

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
    
    def searchAllDriverMusic(self):
        self.scanFolder(QDir.homePath())

    def searchOneFolderMusic(self):
        url = QFileDialog.getExistingDirectory()
        if url:
            self.scanFolder(url)

    def addSongFile(self):
        urls, _ = QFileDialog.getOpenFileNames(
            caption="Select one or more files to open", 
            directory="/home", 
            filter="music(*mp2 *.mp3 *.mp4 *.m4a *wma *wav)"
        )
        self.addSongFiles(urls)

    @dthread
    def addSongFiles(self, urls):
        for url in urls:
            self.updateSonglist(url)

    @dthread
    def scanFolder(self, path):
        filters = QDir.Files
        nameFilters = ["*.wav", "*.wma", "*.mp2", "*.mp3", "*.mp4", "*.m4a", "*.flac", "*.ogg"]
        qDirIterator = QDirIterator(path, nameFilters, filters, QDirIterator.Subdirectories)

        while qDirIterator.hasNext():
            qDirIterator.next()
            fileInfo = qDirIterator.fileInfo()
            fdir = fileInfo.absoluteDir().absolutePath()
            fpath = qDirIterator.filePath()
            fsize = fileInfo.size() / (1024 * 1024)
            time.sleep(0.05)
            if fsize >= 1:
                self.scanfileChanged.emit(fpath)
                self.tipMessageChanged.emit(fpath)
        self.tipMessageChanged.emit('')

        # self._songsDB.update(self._songsDict)
        # self._artistsDB.update(self._artistsDict)
        # self._albumsDB.update(self._albumsDict)
        # self._foldersDB.update(self._foldersDict)

    def updateSonglist(self, fpath):
        songDict = SongDict(fpath)
        url = songDict['url']
        self._songsDict[url] = songDict

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

        self.updateSongs()
        self.updateArtists()
        self.updateAlbumss()
        self.updateFolders()

        # self.saveSongToDB.emit(songDict)

    def updateSongs(self):
        if self._songs != self._songsDict.values():
            self.songs = self._songsDict.values()
        self.songCountChanged.emit(len(self._songsDict))

    def updateArtists(self):
        if self._artists != self._artistsDict.values():
            self.artists = self._artistsDict.values()

    def updateAlbumss(self):
        if self._albums != self._albumsDict.values():
            self.albums = self._albumsDict.values()

    def updateFolders(self):
        if self._folders != self._foldersDict.values():
            self.folders = self._foldersDict.values()

    def playArtistMusic(self, name):
        urls = self._artistsDict[name]['urls']
        self.postSongs(urls)

    def playAlbumMusic(self, name):
        urls = self._albumsDict[name]['urls']
        self.postSongs(urls)

    def playFolderMusic(self, name):
        urls = self._foldersDict[name]['urls']
        self.postSongs(urls)

    def postSongs(self, urls):
        songs = []
        for url in urls:
            songs.append(self._songsDict[url])
        self.addSongsToPlaylist.emit(songs)
