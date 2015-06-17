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
                QThreadPool, QAbstractListModel, Qt, 
                QModelIndex, QVariant, QFile)
from PyQt5.QtGui import QImage
from PyQt5.QtQml import QJSValue
from PyQt5.QtWidgets import QFileDialog
from .utils import registerContext, contexts, openLocalUrl
from models import *
from dwidgets import dthread, LevelJsonDict
from dwidgets.mediatag.song import Song as SongDict
from collections import OrderedDict
from UserList import UserList
from config.constants import LevevDBPath, CoverPath, MusicManagerPath
from .coverworker import CoverWorker
from .signalmanager import signalManager
from dwidgets import DListModel, ModelMetaclass
from dwidgets.xpinyin import Pinyin
from windowmanageworker import windowManageWorker


def is_chinese(chinese):
    """check chinese or not"""
    if isinstance(chinese, str):
        chinese = chinese.decode('utf-8')
    uchar = chinese[0]
    if uchar >= u'\u4e00' and uchar<=u'\u9fa5':
        return True
    else:
        return False

p = Pinyin()

def getPinyin(chinese):
    return  p.get_pinyin(chinese, '')


class QmlSongObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('url', 'QString'),
        ('folder', 'QString'),
        ('title', 'QString'),
        ('artist', 'QString'),
        ('album', 'QString'),
        ('composer', 'QString'),
        ('tracknumber', int),
        ('discnumber', int),
        ('genre', 'QString'),
        ('date', 'QString'),
        ('size', int),
        ('ext', 'QString'),
        ('mediaType', 'QString'),
        ('duration', int),
        ('bitrate', int),
        ('sample_rate', int),
        ('cover', 'QString'),
        ('playCount ', int),
        ('created_date', float),
    )

    coverReady = pyqtSignal('QString')

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)
        self.cover = CoverWorker.getCover(self.title, self.artist, self.album)
        self.coverChanged.connect(self.checkCover)

    def checkCover(self, cover):
        self.coverReady.emit(cover)

    def getCover(self):
        _cover = CoverWorker.getCover(self.title, self.artist, self.album)
        self.cover = _cover
        self.coverReady.emit(_cover)


class QmlArtistObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('name', 'QString'),
        ('count', int),
        ('cover', 'QString'),
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
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)

class QmlFolderObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('name', 'QString'),
        ('count', int),
        ('cover', 'QString'),
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

class DetailSongListModel(DListModel):

    __contextName__ = 'DetailSongListModel'

    @registerContext
    def __init__(self, dataTye):
        super(DetailSongListModel, self).__init__(dataTye)


class MusicManageWorker(QObject):

    #py2py
    scanfileChanged = pyqtSignal('QString')
    scanfileFinished = pyqtSignal()
    saveSongToDB = pyqtSignal(dict)
    saveSongsToDB = pyqtSignal(list)
    restoreSongsToDB = pyqtSignal(list)
    loadDBSuccessed = pyqtSignal()
    addSongToPlaylist = pyqtSignal('QString')
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

    switchPage = pyqtSignal('QString')

    #qml2qml 
    detailArtist = pyqtSignal('QString', int)
    detailAlbum = pyqtSignal('QString', int)
    detailFolder = pyqtSignal('QString', int)

    __contextName__ = 'MusicManageWorker'

    _songsDict = LevelJsonDict(os.path.join(LevevDBPath, 'song'))
    _artistsDict =  LevelJsonDict(os.path.join(LevevDBPath, 'artist'))
    _albumsDict = LevelJsonDict(os.path.join(LevevDBPath, 'album'))
    _foldersDict =  LevelJsonDict(os.path.join(LevevDBPath, 'folder'))

    _songObjs = OrderedDict()
    _artistObjs = OrderedDict()
    _albumObjs = OrderedDict()
    _folderObjs = OrderedDict()

    _songObjsListModel = SongListModel(QmlSongObject)
    _artistObjsListModel = ArtistListModel(QmlArtistObject)
    _albumObjsListModel = AlbumListModel(QmlAlbumObject)
    _folderObjsListModel = FolderListModel(QmlFolderObject)
    _detailSongObjsListModel = DetailSongListModel(QmlSongObject)

    @registerContext
    def __init__(self, parent=None):
        super(MusicManageWorker, self).__init__(parent)
        self._songsDict.open()
        self._artistsDict.open()
        self._albumsDict.open()
        self._foldersDict.open()
        self.initConnect()

    def initConnect(self):
        self.searchAllDriver.connect(self.searchAllDriverMusic)
        self.searchOneFolder.connect(self.searchOneFolderMusic)
        self.playArtist.connect(self.playArtistMusic)
        self.playAlbum.connect(self.playAlbumMusic)
        self.playFolder.connect(self.playFolderMusic)

        self.switchPage.connect(self.updatePage)

        self.scanfileChanged.connect(self.addSong)
        self.scanfileFinished.connect(self.saveSongs)

        signalManager.addLocalSongToDataBase.connect(self.addLocalSongToDataBase)
        signalManager.addLocalSongsToDataBase.connect(self.addLocalSongsToDataBase)

        signalManager.playMusicByLocalUrl.connect(self.playSongMusic)

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
        
        self.updateArtist()

        # if Album.select().count() > 0:
        self.updateAlbum()

        # if Folder.select().count() > 0:
        self.updateFolder()

        self.loadDBSuccessed.emit()

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

    @classmethod
    def getSongDownloadPath(cls, singerName, name, ext):
        configWorker = contexts['ConfigWorker']
        downloadSongPath = configWorker.DownloadSongPath
        return os.path.join(downloadSongPath, '%s-%s.%s' % (singerName, name, ext))

    @classmethod
    def isSongExistedInDataBase(cls, artist, title):
        from dwidgets.mediatag.common import TRUST_AUDIO_EXT
        for ext in TRUST_AUDIO_EXT:
            path = cls.getSongDownloadPath(artist, title, ext)
            if cls.getSongObjByUrl(path):
                return True, path
        return False, ''

    @classmethod
    def getSongObjByUrl(cls, url):
        if url in cls._songObjs:
            return cls._songObjs[url]
        else:
            return None

    @pyqtSlot('QString', result=QVariant)
    def updateDetailSongObjsByArtist(self, artist):
        self._detailSongObjsListModel.clear()
        for url, obj in self._songObjs.items():
            if obj.artist == artist:
                self._detailSongObjsListModel.append(obj)

    @classmethod
    def getSongObjsByArtist(cls, artist):
        songObjs = []
        for url, obj in cls._songObjs.items():
            if obj.artist == artist:
                songObjs.append(obj)
        return songObjs

    @classmethod
    def getUrlsByArtist(cls, artist):
        urls = []
        for url, obj in cls._songObjs.items():
            if obj.artist == artist:
                urls.append(url)
        return urls

    @pyqtSlot('QString', result=QVariant)
    def updateDetailSongObjsByAlbum(self, album):
        self._detailSongObjsListModel.clear()
        for url, obj in self._songObjs.items():
            if obj.album == album:
                self._detailSongObjsListModel.append(obj)

    @classmethod
    def getSongObjsByAlbum(cls, album):
        songObjs = []
        for url, obj in cls._songObjs.items():
            if obj.album == album:
                songObjs.append(obj)
        return songObjs

    @classmethod
    def getUrlsByAlbum(cls, album):
        urls = []
        for url, obj in cls._songObjs.items():
            if obj.album == album:
                urls.append(url)
        return urls

    @pyqtSlot('QString', result=QVariant)
    def updateDetailSongObjsByFolder(self, folder):
        self._detailSongObjsListModel.clear()
        for url, obj in self._songObjs.items():
            if obj.folder == folder:
                self._detailSongObjsListModel.append(obj)

    @classmethod
    def getSongObjsByFolder(cls, folder):
        songObjs = []
        for url, obj in cls._songObjs.items():
            if obj.folder == folder:
                songObjs.append(obj)
        return songObjs

    @classmethod
    def getUrlsByFolder(cls, folder):
        urls = []
        for url, obj in cls._songObjs.items():
            if obj.folder == folder:
                urls.append(url)
        return urls

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

    @pyqtSlot('QString')
    def addLocalSongToDataBase(self, url):
        urls = [url]
        self.addSongFiles(urls)

    @pyqtSlot('QString')
    def addLocalSongsToDataBase(self, urls):
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
            time.sleep(0.05)
            if fsize >= 1:
                self.scanfileChanged.emit(fpath)
                self.tipMessageChanged.emit(fpath)
        self.tipMessageChanged.emit('')
        self.scanfileFinished.emit()

    def saveSongs(self):
        self.saveSongsToDB.emit(self._tempSongs.values())

    def addSong(self, fpath):
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

        songDict['created_date'] = time.time()

        url = songDict['url']
        if url in self._songsDict:
            self._songsDict[url] = songDict
            self._tempSongs[url] = songDict
            return
        else:
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
                'cover': CoverWorker.getCoverPathByArtist(artist)
            }
        _artistDict = self._artistsDict[artist]
        _artistDict['count'] = _artistDict['count'] + 1
        self._artistsDict[artist] = _artistDict

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
                'cover':CoverWorker.getCoverPathByArtistAlbum(artist, album)
            }
        _albumDict = self._albumsDict[album]
        _albumDict['count'] = _albumDict['count'] + 1
        self._albumsDict[album] = _albumDict

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
                'cover':CoverWorker.getFolderCover()
            }
        _folderDict = self._foldersDict[folder]
        _folderDict['count'] = _folderDict['count'] + 1
        self._foldersDict[folder] = _folderDict

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

        # if windowManageWorker.currentMusicManagerPageName == "ArtistPage":
        if not CoverWorker.isArtistCoverExisted(artist):
                self.downloadArtistCover.emit(artist)

        if not CoverWorker.isAlbumCoverExisted(artist, album):
                self.downloadAlbumCover.emit(artist, album)

    def updateArtistCover(self, artist, url):
        for artistName in  self._artistsDict:
            if artist in artistName:
                _artistDict = self._artistsDict[artistName]
                artistCoverUrl = CoverWorker.getCoverPathByArtist(artistName)
                if artistCoverUrl:
                    _artistDict['cover'] = artistCoverUrl
                    keys = self._artistObjs.keys()
                    if artistName in keys:
                        index = keys.index(artistName)
                        artistObj = self._artistObjs[artistName]
                        artistObj.cover = artistCoverUrl
                        self._artistObjsListModel.setProperty(index, 'cover', artistCoverUrl)

                    for songUrl, songObj in self._songObjs.items():
                        if artist in songObj.artist:
                            albumCoverUrl = CoverWorker.getCoverPathByArtistAlbum(songObj.artist, songObj.album)
                            if albumCoverUrl != CoverWorker.defaultAlbumCover:
                                break
                            else:
                                songObj.cover = artistCoverUrl

    def updateAlbumCover(self, artist, album, url):
        if album in self._albumsDict:
            _albumDict = self._albumsDict[album]
            albumCoverUrl = CoverWorker.getCoverPathByArtistAlbum(artist, album)
            if albumCoverUrl:
                _albumDict['cover'] = albumCoverUrl
                keys = self._albumObjs.keys()
                if album in keys:
                    index = keys.index(album)
                    albumObj = self._albumObjs[album]
                    albumObj.cover = albumCoverUrl
                    self._albumObjsListModel.setProperty(index, 'cover', albumCoverUrl)

                for url, songObj in self._songObjs.items():
                    if artist in songObj.artist and album in songObj.album:
                        songObj.cover = albumCoverUrl

    def stopUpdate(self):
        print('stop update')

    @classmethod
    def generateSongObjByUrl(cls, fpath):
        songDict = SongDict(fpath)
        songObj = QmlSongObject(**songDict)
        return songObj

    def playArtistMusic(self, name):
        urls = self.getUrlsByArtist(name)
        self.postSongs(urls)

    def playAlbumMusic(self, name):
        urls = self.getUrlsByAlbum(name)
        self.postSongs(urls)

    def playFolderMusic(self, name):
        urls = self.getUrlsByFolder(name)
        self.postSongs(urls)

    def postSongs(self, urls):
        self.addSongsToPlaylist.emit(urls)
        self.playSongByUrl.emit(urls[0])

    def playSongMusic(self, url):
        self.addSongToPlaylist.emit(url)

    def openSongFolder(self, url):
        songObj = self._songObjs[url]
        openLocalUrl(songObj.folder)

    def orderByKey(self, modelType, key):
        '''
            order _songObjsListModel and _detailSongObjsListModel by key
        '''
        if modelType == 'AllSongs':
            model = self._songObjsListModel
        elif modelType == 'DetailSubSongs':
            model = self._detailSongObjsListModel
        songObjs = {}
        chineseObjs = {}
        for songObj in model.data:
            if key in ['title', 'artist', 'album']:
                value = getattr(songObj, key)
                if is_chinese(value):
                    pinkey = getPinyin(value)
                    if pinkey not in chineseObjs:
                        chineseObjs[pinkey] = [songObj]
                    else:
                        chineseObjs[pinkey].append(songObj)
                else:
                    pinkey = getattr(songObj, key)
                    if pinkey not in songObjs:
                        songObjs[pinkey] = [songObj]
                    else:
                        songObjs[pinkey].append(songObj)
            else:
                pinkey = getattr(songObj, key)
                if pinkey not in songObjs:
                    songObjs[pinkey] = [songObj]
                else:
                    songObjs[pinkey].append(songObj)

        model.clear()
        data = [songObjs[k] for k in sorted(songObjs.keys())]
        for objs in data:
            for obj in objs:
                model.append(obj)

        data = [chineseObjs[k] for k in sorted(chineseObjs.keys())]
        for objs in data:
            for obj in objs:
                model.append(obj)

    def removeFromDatabaseByUrl(self, url, actionType="removeSongByUrl"):
        '''
            remove song from DataBase by url
        '''
        if url in self._songsDict:
            songDict = self._songsDict[url]
            del self._songsDict[url]
            if url in self._songObjs:
                del self._songObjs[url]

            self.removeFormModel('url', url, self._songObjsListModel)
            self.removeFormModel('url', url, self._detailSongObjsListModel)

            if actionType == "removeSongByUrl":
                songInstance = Song.getRecord(**{'url': url})
                if songInstance:
                        songInstance.delete_instance()

            artistName = songDict['artist']
            albumName = songDict['album']
            folderName = songDict['folder']
            self.updateArtistByUrl(artistName)
            self.updateAlbumByUrl(albumName)
            self.updateFolderByUrl(folderName)
        self.songCountChanged.emit(len(self._songsDict))

    def removeFromDriveByUrl(self, url, actionType="removeSongByUrl"):
        '''
            remove song from Drive by url
        '''
        df = QFile(url)
        flag = df.remove()
        if flag:
            self.removeFromDatabaseByUrl(url, actionType)

    def removeFormModel(self, key, value, model):
        '''
            update listview model by url 
        '''
        delete_index = None
        for index, obj in enumerate(model.data):
            if getattr(obj, key) == value:
                delete_index = index
                break
        if delete_index is not None:
            model.remove(delete_index)

    def updateArtistByUrl(self, artistName):
        '''
            update artist DataBase and listview model when delete song by url
        '''
        artist = Artist.getRecord(**{'name': artistName})
        if artist:
            artistDict = {
                'name': artist.name,
                'count': artist.songs.count(),
                'cover': CoverWorker.getCoverPathByArtist(artist.name)
            }
            if artistDict['count'] > 0:
                self._artistsDict[artist.name] = artistDict
                artistObj = QmlArtistObject(**artistDict)
                self._artistObjs[artist.name] = artistObj
            else:
                if artist.name in self._artistsDict:
                    del self._artistsDict[artist.name]
                if artist.name in self._artistObjs:
                    del self._artistObjs[artist.name]

            for index, artistObj in enumerate(self._artistObjsListModel.data):
                if artistObj.name == artistName:
                    if artistDict['count'] > 0:
                        self._artistObjsListModel.setProperty(index, 'count', artistDict['count'])
                    else:
                        self._artistObjsListModel.remove(index)

    def updateAlbumByUrl(self, albumName):
        '''
            update album DataBase and listview model when delete song by url
        '''
        album = Album.getRecord(**{'name': albumName})
        if album:
            albumDict = {
                'name': album.name,
                'count': album.songs.count(),
                'cover': CoverWorker.getCoverPathByArtist(album.name)
            }
            if albumDict['count'] > 0:
                self._albumsDict[album.name] = albumDict
                albumObj = QmlArtistObject(**albumDict)
                self._albumObjs[album.name] = albumObj
            else:
                if album.name in self._albumsDict:
                    del self._albumsDict[album.name]
                if album.name in self._albumObjs:
                    del self._albumObjs[album.name]

            for index, albumObj in enumerate(self._albumObjsListModel.data):
                if albumObj.name == albumName:
                    if albumDict['count'] > 0:
                        self._albumObjsListModel.setProperty(index, 'count', albumDict['count'])
                    else:
                        self._albumObjsListModel.remove(index)

    def updateFolderByUrl(self, folderName):
        '''
            update folder DataBase and listview model when delete song by url
        '''
        folder = Folder.getRecord(**{'name': folderName})
        if folder:
            folderDict = {
                'name': folder.name,
                'count': folder.songs.count(),
                'cover': CoverWorker.getCoverPathByArtist(folder.name)
            }
            if folderDict['count'] > 0:
                self._foldersDict[folder.name] = folderDict
                folderObj = QmlArtistObject(**folderDict)
                self._folderObjs[folder.name] = folderObj
            else:
                if folder.name in self._foldersDict:
                    del self._foldersDict[folder.name]
                if folder.name in self._folderObjs:
                    del self._folderObjs[folder.name]

            for index, folderObj in enumerate(self._folderObjsListModel.data):
                if folderObj.name == folderName:
                    if folderDict['count'] > 0:
                        self._folderObjsListModel.setProperty(index, 'count', folderDict['count'])
                    else:
                        self._folderObjsListModel.remove(index)

    def removeFromDatabaseByArtistName(self, artistName):
        '''
            remove artist from DataBase and remove songs belong to this artist
        '''
        _urls = self.getSongUrlsByArtist(artistName)
        query = Song.delete().where(Song.artist == artistName)
        count = query.execute()
        for url in _urls:
            self.removeFromDatabaseByUrl(url)

    def removeFromDriverByArtistName(self, artistName):
        '''
            remove artist from Driver and remove songs belong to this artist
        '''
        _urls = self.getSongUrlsByArtist(artistName)
        query = Song.delete().where(Song.artist == artistName)
        count = query.execute()
        for url in _urls:
            self.removeFromDriveByUrl(url)

    def getSongUrlsByArtist(self, artistName):
        '''
            get song urls by artist
        '''
        artist = Artist.getRecord(**{'name': artistName})
        _urls = []
        if artist:
            for song in artist.songs:
                _urls.append(song.url)
        return _urls

    def removeFromDatabaseByAlbumName(self, albumName):
        '''
            remove album from DataBase and remove songs belong to this artist
        '''
        _urls = self.getSongUrlsByAlbum(albumName)
        query = Song.delete().where(Song.album == albumName)
        count = query.execute()
        for url in _urls:
            self.removeFromDatabaseByUrl(url)

    def removeFromDriverByAlbumName(self, albumName):
        '''
            remove album from Driver and remove songs belong to this artist
        '''
        _urls = self.getSongUrlsByAlbum(albumName)
        query = Song.delete().where(Song.album == albumName)
        count = query.execute()
        for url in _urls:
            self.removeFromDriveByUrl(url)

    def getSongUrlsByAlbum(self, albumName):
        '''
            get song urls by album
        '''
        album = Album.getRecord(**{'name': albumName})
        _urls = []
        if album:
            for song in album.songs:
                _urls.append(song.url)
        return _urls

    def removeFromDatabaseByFolderName(self, folderName):
        '''
            remove folder from Driver and remove songs belong to this artist
        '''
        _urls = self.getSongUrlsByFolder(folderName)
        query = Song.delete().where(Song.folder == folderName)
        count = query.execute()
        for url in _urls:
            self.removeFromDatabaseByUrl(url, actionType="removeByFolder")

    def removeFromDriverByFolderName(self, folderName):
        '''
            remove folder from Driver and remove songs belong to this artist
        '''
        _urls = self.getSongUrlsByFolder(folderName)
        query = Song.delete().where(Song.folder == folderName)
        query.execute()
        for url in _urls:
            self.removeFromDriveByUrl(url, actionType="removeByFolder")

    def getSongUrlsByFolder(self, folderName):
        '''
            get song urls by folder
        '''
        folder = Folder.getRecord(**{'name': folderName})
        _urls = []
        if folder:
            for song in folder.songs:
                _urls.append(song.url)
        return _urls

    def updateArtist(self):
        self._artistObjsListModel.clear()
        for artist in Artist.select().order_by(Artist.name):
            artistDict = {
                'name': artist.name,
                'count': artist.songs.count(),
                'cover': CoverWorker.getCoverPathByArtist(artist.name)
            }
            self._artistsDict[artist.name] = artistDict
            artistObj = QmlArtistObject(**artistDict)
            self._artistObjs[artist.name] = artistObj
            if artistDict['count'] > 0:
                self._artistObjsListModel.append(artistObj)
            if not CoverWorker.isArtistCoverExisted(artist.name):
                self.downloadArtistCover.emit(artist.name)

        self.orderModel('name', self._artistObjsListModel)

    def updateAlbum(self):
        self._albumObjsListModel.clear()
        for album in Album.select().order_by(Album.name):
            albumDict = {
                'name': album.name,
                'artist': album.artist,
                'count': album.songs.count(),
                'cover': CoverWorker.getCoverPathByArtistAlbum(album.artist, album.name)
            }
            self._albumsDict[album.name] = albumDict
            albumObj = QmlAlbumObject(**albumDict)
            self._albumObjs[album.name] = albumObj

            if albumDict['count'] > 0:
                self._albumObjsListModel.append(albumObj)

            if not CoverWorker.isAlbumCoverExisted(album.artist, album.name):
                self.downloadAlbumCover.emit(album.artist, album.name)

        self.orderModel('name', self._albumObjsListModel)

    def updateFolder(self):
        self._folderObjsListModel.clear()
        for folder in Folder.select().order_by(Folder.name):
            folderDict = {
                'name': folder.name,
                'count': folder.songs.count(),
                'cover':CoverWorker.getFolderCover()
            }
            self._foldersDict[folder.name] = folderDict
            folderObj = QmlFolderObject(**folderDict)
            self._folderObjs[folder.name] = folderObj
            if folderDict['count'] > 0:
                self._folderObjsListModel.append(folderObj)

        self.orderModel('name', self._folderObjsListModel)

    def orderModel(self, key, model):
        '''
            order _songObjsListModel and _detailSongObjsListModel by key
        '''

        Objs = {}
        chineseObjs = {}
        for obj in model.data:
            value = getattr(obj, key)

            if model is self._folderObjsListModel:
                value = os.path.basename(value)
            if is_chinese(value):
                pinkey = getPinyin(value)
                if pinkey not in chineseObjs:
                    chineseObjs[pinkey] = [obj]
                else:
                    chineseObjs[pinkey].append(obj)
            else:
                pinkey = value
                if pinkey not in Objs:
                    Objs[pinkey] = [obj]
                else:
                    Objs[pinkey].append(obj)

        model.clear()
        data = [Objs[k] for k in sorted(Objs.keys())]
        for objs in data:
            for obj in objs:
                model.append(obj)

        data = [chineseObjs[k] for k in sorted(chineseObjs.keys())]
        for objs in data:
            for obj in objs:
                model.append(obj)

    def updatePage(self, pageName):
        if pageName == 'ArtistPage':
            # self.updateArtist()
            pass
        elif pageName == 'AlbumPage':
            # self.updateAlbum()
            pass
        elif pageName == 'FolderPage':
            # self.updateFolder()
            pass


musicManageWorker = MusicManageWorker()
