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
        ('date', 'QString'),
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
        ('artist', 'QString'),
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
        ('cover', 'QString'),
        ('songs', dict),
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)


class SongListModel(DListModel):

    __contextName__ = 'SongListModel'

    @registerContext
    def __init__(self, dataTye):
        super(SongListModel, self).__init__(dataTye)

class ArtistListModel(DListModel):

    __contextName__ = 'ArtistListModel'

    @registerContext
    def __init__(self, dataTye):
        super(ArtistListModel, self).__init__(dataTye)

class AlbumListModel(DListModel):

    __contextName__ = 'AlbumListModel'

    @registerContext
    def __init__(self, dataTye):
        super(AlbumListModel, self).__init__(dataTye)

class FolderListModel(DListModel):

    __contextName__ = 'FolderListModel'

    @registerContext
    def __init__(self, dataTye):
        super(FolderListModel, self).__init__(dataTye)


class MusicManageWorker(QObject):

    #py2py
    scanfileChanged = pyqtSignal('QString')
    scanfileFinished = pyqtSignal()
    saveSongToDB = pyqtSignal(dict)
    saveSongsToDB = pyqtSignal(list)
    restoreSongsToDB = pyqtSignal(list)
    addSongToPlaylist = pyqtSignal(dict)
    addSongsToPlaylist = pyqtSignal(list)
    playSongByUrl = pyqtSignal('QString')

    downloadArtistCover = pyqtSignal('QString')
    downloadAlbumCover = pyqtSignal('QString', 'QString')

    #property signal
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
        self.initConnect()
        self.initData()

    def initData(self):
        self._songsDict = LevelJsonDict(os.path.join(LevevDBPath, 'song'))
        self._artistsDict =  LevelJsonDict(os.path.join(LevevDBPath, 'artist'))
        self._albumsDict = LevelJsonDict(os.path.join(LevevDBPath, 'album'))
        self._foldersDict =  LevelJsonDict(os.path.join(LevevDBPath, 'folder'))

        self._songObjs = OrderedDict()
        self._artistObjs = OrderedDict()
        self._albumObjs = OrderedDict()
        self._folderObjs = OrderedDict()

        self._songObjsListModel = SongListModel(QmlSongObject)
        self._artistObjsListModel = ArtistListModel(QmlArtistObject)
        self._albumObjsListModel = AlbumListModel(QmlAlbumObject)
        self._folderObjsListModel = FolderListModel(QmlFolderObject)

    def initConnect(self):
        self.searchAllDriver.connect(self.searchAllDriverMusic)
        self.searchOneFolder.connect(self.searchOneFolderMusic)
        self.playArtist.connect(self.playArtistMusic)
        self.playAlbum.connect(self.playAlbumMusic)
        self.playFolder.connect(self.playFolderMusic)
        self.playSong.connect(self.playSongMusic)

        self.scanfileChanged.connect(self.updateSonglist)
        self.scanfileFinished.connect(self.saveSongs)

    def restoreDB(self):
        if Song.select().count() == 0:
            self.restoreSongsToDB.emit(self._songsDict.values())
        else:
            self.loadDB()

    @dthread
    def loadDB(self):
        # if Song.select().count() > 0:
        for song in Song.select().order_by(Song.title):
            songDict = song.toDict()
            self._songsDict[song.url] = songDict
            songObj = QmlSongObject(**songDict)
            self._songObjs[song.url] = songObj
            self._songObjsListModel.append(songObj)

        # if Artist.select().count() > 0:
        for artist in Artist.select().order_by(Artist.name):
            artistDict = {
                'name': artist.name,
                'count': artist.songs.count(),
                'cover': CoverWorker.getCoverPathByArtist(artist.name),
                'songs': {}
            }
            songs = artistDict['songs']
            for song in artist.songs:
                songs.update({song.toDict()['url']: song.toDict()})
            self._artistsDict[artist.name] = artistDict
            artistObj = QmlArtistObject(**artistDict)
            self._artistObjs[artist.name] = artistObj
            self._artistObjsListModel.append(artistObj)

            if not CoverWorker.isArtistCoverExisted(artist.name):
                self.downloadArtistCover.emit(artist.name)

        # if Album.select().count() > 0:
        for album in Album.select().order_by(Album.name):
            albumDict = {
                'name': album.name,
                'artist': album.artist,
                'count': album.songs.count(),
                'cover': CoverWorker.getCoverPathByArtistAlbum(album.artist, album.name),
                'songs': {}
            }
            songs = albumDict['songs']
            for song in album.songs:
                songs.update({song.toDict()['url']: song.toDict()})

            self._albumsDict[album.name] = albumDict
            albumObj = QmlAlbumObject(**albumDict)
            self._albumObjs[album.name] = albumObj
            self._albumObjsListModel.append(albumObj)

            if not CoverWorker.isAlbumCoverExisted(album.artist, album.name):
                self.downloadAlbumCover.emit(album.artist, album.name)

        # if Folder.select().count() > 0:
        for folder in Folder.select().order_by(Folder.name):
            folderDict = {
                'name': folder.name,
                'count': folder.songs.count(),
                'cover':CoverWorker.getFolderCover(),
                'songs': {}
            }
            songs = folderDict['songs']
            for song in folder.songs:
                songs.update({song.toDict()['url']: song.toDict()})
            self._foldersDict[folder.name] = folderDict
            folderObj = QmlFolderObject(**folderDict)
            self._folderObjs[folder.name] = folderObj
            self._folderObjsListModel.append(folderObj)

        # for artistObj in self._artistObjsListModel.data:
        #     artist = artistObj.name
        #     if not CoverWorker.isArtistCoverExisted(artist):
        #         self.downloadArtistCover.emit(artist)

        # for albumObj in self._albumObjsListModel.data:
        #     album = albumObj.name
        #     artist = albumObj.artist
        #     if not CoverWorker.isAlbumCoverExisted(artist, album):
        #         self.downloadAlbumCover.emit(artist, album)

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
        if urls:
            self.addSongFiles(urls)

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
            time.sleep(0.02)
            if fsize >= 1:
                self.scanfileChanged.emit(fpath)
                self.tipMessageChanged.emit(fpath)
        self.tipMessageChanged.emit('')
        self.scanfileFinished.emit()

    def saveSongs(self):
        self.saveSongsToDB.emit(self._tempSongs.values())
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

        #add or update song view
        songObj = QmlSongObject(**songDict)
        self._songObjs[url] = songObj
        self._songObjsListModel.append(songObj)

        # add or update artist view
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

        if artist not in self._artistObjs:
            artistObj = QmlArtistObject(**_artistDict)
            self._artistObjs[artist] = artistObj
            self._artistObjsListModel.append(artistObj)
        else:
            artistObj = self._artistObjs[artist]
            index = self._artistObjs.keys().index(artist)
            artistObj.count = _artistDict['count']
            self._artistObjsListModel.setProperty(index, 'count', _artistDict['count'])

        # add or update album view
        album = songDict['album']
        if album not in self._albumsDict:
            self._albumsDict[album] = {
                'name': album,
                'artist': artist,
                'count': 0,
                'cover':CoverWorker.getCoverPathByArtistAlbum(artist, album),
                'songs': {}
            }
        _albumDict = self._albumsDict[album]
        songs = _albumDict['songs']
        songs.update({url: songDict})
        _albumDict['count'] = len(songs)

        if album not in self._albumObjs:
            albumObj = QmlAlbumObject(**_albumDict)
            self._albumObjs[album] = albumObj
            self._albumObjsListModel.append(albumObj)
        else:
            albumObj = self._albumObjs[album]
            index = self._albumObjs.keys().index(album)
            albumObj.count = _albumDict['count']
            self._albumObjsListModel.setProperty(index, 'count', _albumDict['count'])

        # add or update folder view
        folder = songDict['folder']
        if folder not in self._foldersDict:
            self._foldersDict[folder] = {
                'name': folder,
                'count': 0,
                'cover':CoverWorker.getFolderCover(),
                'songs': {}
            }
        _folderDict = self._foldersDict[folder]         
        songs = _folderDict['songs']
        songs.update({url: songDict})
        _folderDict['count'] = len(songs)
        if folder not in self._folderObjs:
            folderObj = QmlFolderObject(**_folderDict)
            self._folderObjs[folder] = folderObj
            self._folderObjsListModel.append(folderObj)
        else:
            folderObj = self._folderObjs[folder]
            index = self._folderObjs.keys().index(folder)
            folderObj.count = _folderDict['count']
            self._folderObjsListModel.setProperty(index, 'count', _folderDict['count'])

        self.songCountChanged.emit(len(self._songsDict))

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
                    index = self._artistObjs.keys().index(artistName)
                    artistObj = self._artistObjs[artistName]
                    artistObj.cover = url
                    self._artistObjsListModel.setProperty(index, 'cover', url)

    def updateAlbumCover(self, artist, album, url):
        if album in self._albumsDict:
            _albumDict = self._albumsDict[album]
            url = CoverWorker.getCoverPathByArtistAlbum(artist, album)
            if url:
                _albumDict['cover'] = url
                index = self._albumObjs.keys().index(album)
                albumObj = self._albumObjs[album]
                albumObj.cover = url
                self._albumObjsListModel.setProperty(index, 'cover', url)

    def stopUpdate(self):
        print('stop update')

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
