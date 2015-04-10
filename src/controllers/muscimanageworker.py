#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import time
import json
import datetime
from PyQt5.QtCore import (QObject, pyqtSignal,
                pyqtSlot, pyqtProperty, QDir, 
                QDirIterator, QTimer, QThread,
                QThreadPool, QAbstractListModel, Qt, QModelIndex, QVariant)
from PyQt5.QtGui import QImage
from PyQt5.QtWidgets import QFileDialog
from .utils import registerContext, contexts
from dwidgets.tornadotemplate import template
from models import *
from dwidgets import dthread, LevelJsonDict
from dwidgets.mediatag.song import Song as SongDict
from collections import OrderedDict
from UserList import UserList
from config.constants import CoverPath, MusicManagerPath
from .coverworker import CoverWorker

from dwidgets import ModelMetaclass



class ListModel(QAbstractListModel):
    
    _roles = {}

    def __init__(self, fields, parent=None):
        super(ListModel, self).__init__(parent)
        for i in fields:
            index = fields.index(i)
            role = '%sRole' % i[0]
            setattr(self, role, Qt.UserRole + index + 1)
            self._roles[Qt.UserRole + index + 1] = i[0]
        print self._roles
        self._items = []

    def addItem(self, item):
        self.beginInsertRows(QModelIndex(), self.rowCount(), self.rowCount())
        self._items.append(item)
        self.endInsertRows()

    def rowCount(self, parent=QModelIndex()):
        return len(self._items)

    def data(self, index, role=Qt.DisplayRole):
        try:
            item = self._items[index.row()]
        except IndexError:
            return QVariant()
        for key, value in self._roles.items():
            if role == getattr(self, key):
                return item[value]

        return QVariant()

    def roleNames(self):
        print('++++++++++++++++')
        return self._roles


class QmlSongObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('url', 'QString'),
        ('folder', 'QString'),
        ('title', 'QString'),
        ('artist', 'QString'),
        ('album', 'QString'),
        ('tracknumber', int),
        ('discnumber', int),
        ('genre', 'QString'),
        ('date', int),
        ('size', int),
        ('mediaType', 'QString'),
        ('duration', int),
        ('bitrate', int),
        ('sample_rate', int),
        ('cover', 'QString'),
        ('created_date', 'QString'),
    )

    def initialize(self, *agrs, **kwargs):
        if 'created_date' in kwargs:
            kwargs['created_date'] = kwargs['created_date'].strftime('%Y-%m-%d %H:%M:%S')
        self.setDict(kwargs)


class QmlArtistObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('name', 'QString'),
        ('count', int),
        ('cover', 'QString'),
        ('songs', dict),
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)

class QmlAlbumObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('name', 'QString'),
        ('count', int),
        ('cover', 'QString'),
        ('songs', dict),
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)

class QmlFolderObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('name', 'QString'),
        ('count', int),
        ('songs', dict),
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)


class MusicManageWorker(QObject):

    #py2py
    scanfileChanged = pyqtSignal('QString')
    scanfileFinished = pyqtSignal()
    saveSongToDB = pyqtSignal(dict)
    saveSongsToDB = pyqtSignal(list)
    addSongToPlaylist = pyqtSignal(dict)
    addSongsToPlaylist = pyqtSignal(list)
    playSongByUrl = pyqtSignal('QString')

    downloadArtistCover = pyqtSignal('QString')
    downloadAlbumCover = pyqtSignal('QString', 'QString')

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
    playSong = pyqtSignal('QString')

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

        self.updateTimer = QTimer()
        self.updateTimer.setInterval(2000)
        self.updateTimer.timeout.connect(self.update)

    def initData(self):
        self._songs = []
        self._artists = []
        self._albums = []
        self._folders = []

        self._songsDict = {}
        self._artistsDict =  {}
        self._albumsDict = {}
        self._foldersDict =  {}

        self._songsListModel = ListModel(QmlSongObject.__Fields__)

        # self.db = LevelJsonDict('/dev/shm/artist/')

    def initConnect(self):
        self.searchAllDriver.connect(self.searchAllDriverMusic)
        self.searchOneFolder.connect(self.searchOneFolderMusic)
        self.playArtist.connect(self.playArtistMusic)
        self.playAlbum.connect(self.playAlbumMusic)
        self.playFolder.connect(self.playFolderMusic)
        self.playSong.connect(self.playSongMusic)

        self.scanfileChanged.connect(self.updateSonglist)
        self.scanfileFinished.connect(self.saveSongs)

    def loadDB(self):
        for song in Song.select():
            self._songsDict[song.url] = song.toDict()
            self._songsListModel.addItem(QmlSongObject(**song.toDict()))

        for artist in Artist.select():
            self._artistsDict[artist.name] = {
                'name': artist.name,
                'count': artist.songs.count(),
                'cover': CoverWorker.getCoverPathByArtist(artist.name),
                'songs': {}
            }
            songs = self._artistsDict[artist.name]['songs']
            for song in artist.songs:
                songs.update({song.toDict()['url']: song.toDict()})

        for album in Album.select():
            self._albumsDict[album.name] = {
                'name': album.name,
                'count': album.songs.count(),
                'cover': CoverWorker.getCoverPathByArtistAlbum(album.artist, album.name),
                'songs': {}
            }
            songs = self._albumsDict[album.name]['songs']
            for song in album.songs:
                songs.update({song.toDict()['url']: song.toDict()})


        for folder in Folder.select():
            self._foldersDict[folder.name] = {
                'name': folder.name,
                'count': folder.songs.count(),
                'songs': {}
            }
            songs = self._foldersDict[folder.name]['songs']
            for song in folder.songs:
                songs.update({song.toDict()['url']: song.toDict()})

        self.update()

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
        # print self._songsListModel.roleNames(), '++++++++++'
        # return self._songs
        return self._songsListModel

    @songs.setter
    def songs(self, value):
        # del self._songs[:]
        # self._songs.extend(value)

        objects = []
        for song in value:
            obj = QmlSongObject(**song)
            objects.append(obj)
        del self._songs[:]
        self._songs.extend(objects)
        # print self._songsListModel.roleNames()
        # print self._songsListModel._items[0]
        self.songsChanged.emit(self._songs)

    @pyqtProperty('QVariant', notify=artistsChanged)
    def artists(self):
        return self._artists

    @artists.setter
    def artists(self, value):
        # del self._artists[:]
        # self._artists.extend(value)

        objects = []
        for song in value:
            obj = QmlArtistObject(**song)
            objects.append(obj)
        del self._artists[:]
        self._artists.extend(objects)

        self.artistsChanged.emit(self._artists)

    @pyqtProperty('QVariant', notify=albumsChanged)
    def albums(self):
        return self._albums 

    @albums.setter
    def albums(self, value):
        # del self._albums[:]
        # self._albums.extend(value)

        objects = []
        for song in value:
            obj = QmlAlbumObject(**song)
            objects.append(obj)
        del self._albums[:]
        self._albums.extend(objects)

        self.albumsChanged.emit(self._albums)

    @pyqtProperty('QVariant', notify=foldersChanged)
    def folders(self):
        return self._folders 

    @folders.setter
    def folders(self, value):
        # del self._folders[:]
        # self._folders.extend(value)

        objects = []
        for song in value:
            obj = QmlFolderObject(**song)
            objects.append(obj)
        del self._folders[:]
        self._folders.extend(objects)

        self.foldersChanged.emit(self._folders)
    
    def searchAllDriverMusic(self):
        self.scanFolder(QDir.homePath())
        self.updateTimer.start(1000)

    def searchOneFolderMusic(self):
        url = QFileDialog.getExistingDirectory()
        if url:
            self.scanFolder(url)
            self.updateTimer.start(1000)

    def addSongFile(self):
        urls, _ = QFileDialog.getOpenFileNames(
            caption="Select one or more files to open", 
            directory="/home", 
            filter="music(*mp2 *.mp3 *.mp4 *.m4a *wma *wav)"
        )
        if urls:
            self.addSongFiles(urls)
            self.updateTimer.start(1000)

    @dthread
    def addSongFiles(self, urls):
        self._tempSongs = {}
        for url in urls:
            self.scanfileChanged.emit(url)
        self.scanfileFinished.emit()

    @dthread
    def scanFolder(self, path):
        self._tempSongs = {}

        filters = QDir.Files
        nameFilters = ["*.wav", "*.wma", "*.mp2", "*.mp3", "*.mp4", "*.m4a", "*.flac", "*.ogg"]
        qDirIterator = QDirIterator(path, nameFilters, filters, QDirIterator.Subdirectories)
        while qDirIterator.hasNext():
            qDirIterator.next()
            fileInfo = qDirIterator.fileInfo()
            fdir = fileInfo.absoluteDir().absolutePath()
            fpath = qDirIterator.filePath()
            fsize = fileInfo.size() / (1024 * 1024)
            time.sleep(0.1)
            if fsize >= 1:
                self.scanfileChanged.emit(fpath)
                self.tipMessageChanged.emit(fpath)
        self.tipMessageChanged.emit('')
        self.scanfileFinished.emit()

    def saveSongs(self):
        self.saveSongsToDB.emit(self._tempSongs.values())
        self.update()
        for song in self._songsDict.values():
            artist = song['artist']
            album = song['album']
            if not CoverWorker.isAlbumCoverExisted(artist, album):
                self.downloadAlbumCover.emit(artist, album)

    def updateSonglist(self, fpath):
        songDict = SongDict(fpath)
        ext, coverData = songDict.getMp3FontCover()
        if ext and coverData:
            if os.sep in songDict['artist']:
                songDict['artist'] = songDict['artist'].replace(os.sep, '')
            coverName = CoverWorker.songCoverPath(songDict['artist'], songDict['title'])
            with open(coverName, 'wb') as f:
                f.write(coverData)
            songDict['cover'] = coverName

        if isinstance(songDict['artist'], str):
            songDict['artist'] = songDict['artist'].decode('utf-8')
        if isinstance(songDict['album'], str):
            songDict['album'] = songDict['album'].decode('utf-8')
        if isinstance(songDict['folder'], str):
            songDict['folder'] = songDict['folder'].decode('utf-8')

        url = songDict['url']
        self._songsDict[url] = songDict
        self._tempSongs[url] = songDict
        self._songsListModel.addItem(QmlSongObject(**songDict))

        artist = songDict['artist']
        if artist not in self._artistsDict:
            self._artistsDict[artist] = {
                'name': artist,
                'count': 0,
                'cover': CoverWorker.getCoverPathByArtist(artist),
                'songs': {}
            }
        _artistDict = self._artistsDict[artist]
        songs = _artistDict['songs']
        songs.update({url: songDict})
        _artistDict['count'] = len(songs)

        album = songDict['album']
        if album not in self._albumsDict:
            self._albumsDict[album] = {
                'name': album,
                'count': 0,
                'cover':CoverWorker.getCoverPathByArtistAlbum(artist, album),
                'songs': {}
            }
        _albumDict = self._albumsDict[album]
        songs = _albumDict['songs']
        songs.update({url: songDict})
        _albumDict['count'] = len(songs)


        folder = songDict['folder']
        if folder not in self._foldersDict:
            self._foldersDict[folder] = {
                'name': folder,
                'count': 0,
                'songs': {}
            }
        _folderDict = self._foldersDict[folder]         
        songs = _folderDict['songs']
        songs.update({url: songDict})
        _folderDict['count'] = len(songs)

        if contexts['WindowManageWorker'].currentMusicManagerPageName == "ArtistPage":
            if not CoverWorker.isArtistCoverExisted(artist):
                self.downloadArtistCover.emit(artist)

    def updateArtistCover(self, artist, url):
        for artistName in  self._artistsDict:
            if artist in artistName:
                _artistDict = self._artistsDict[artistName]
                url = CoverWorker.getCoverPathByArtist(artistName)
                if url:
                    _artistDict['cover'] = url

    def updateAlbumCover(self, artist, album, url):
        if album in self._albumsDict:
            _albumDict = self._albumsDict[album]
            _albumDict['cover'] = CoverWorker.getCoverPathByArtistAlbum(artist, album)

    def update(self):
        self.updateSongs()
        self.updateArtists()
        self.updateAlbumss()
        self.updateFolders()

    def stopUpdate(self):
        self.update()
        self.updateTimer.stop()
        print('stop update')

    def updateSongs(self):
        self.songs = self._songsDict.values()
        self.songCountChanged.emit(len(self._songsDict))

    def updateArtists(self):
        self.artists = self._artistsDict.values()

    def updateAlbumss(self):
        self.albums = self._albumsDict.values()

    def updateFolders(self):
        self.folders = self._foldersDict.values()

    def playArtistMusic(self, name):
        songs = self._artistsDict[name]['songs']
        self.postSongs(songs)

    def playAlbumMusic(self, name):
        songs = self._albumsDict[name]['songs']
        self.postSongs(songs)

    def playFolderMusic(self, name):
        songs = self._foldersDict[name]['songs']
        self.postSongs(songs)

    def postSongs(self, songs):
        songlist = songs.values()
        self.addSongsToPlaylist.emit(songlist)
        self.playSongByUrl.emit(songlist[0]['url'])

    def playSongMusic(self, url):
        song = self._songsDict[url]
        self.addSongToPlaylist.emit(song)
