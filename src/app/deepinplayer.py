#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from PyQt5.QtCore import (QCoreApplication, QObject,
                          QUrl, QThread, QTimer,
                          QThreadPool, QPoint)
from PyQt5.QtGui import QScreen, QIcon
from views import MainWindow, LrcWindowManager, QmlDialog

from controllers import *
from controllers.mediaplayer import gPlayer
from deepin_utils.file import get_parent_dir
import config
from config import isWebengineUsed


class DeepinPlayer(QObject):

    _instance = None

    def __init__(self):
        super(DeepinPlayer, self).__init__()
        self.initApplication()
        self.initView()
        self.initControllers()
        self.initConnect()
        self.initQMLContext()

        self.loadDB()

        self.timer = QTimer()
        self.timer.timeout.connect(self.clearCache)
        self.timer.start(2000)

    @classmethod
    def instance(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    @classmethod
    def getMainWindow(cls):
        instance = cls.instance()
        if instance:
            return instance.mainWindow

    def initApplication(self):
        self.qApp = QCoreApplication.instance()
        self.qApp.setApplicationName(config.applicationName)
        self.qApp.setApplicationVersion(config.applicationVersion)
        self.qApp.setOrganizationName(config.organizationName)
        self.qApp.setWindowIcon(QIcon(config.windowIcon))

    def loadDB(self):
        QTimer.singleShot(100, musicManageWorker.restoreDB)
        signalManager.downloadSuggestPlaylist.emit()

    def initView(self):
        self.mainWindow = MainWindow()
        self.lrcWindowManager = LrcWindowManager(self.mainWindow)

    def initControllers(self):
        self.web360Thread = QThread(self)
        web360ApiWorker.moveToThread(self.web360Thread)
        self.web360Thread.start()

        self.playerBinThread = QThread()
        gPlayer.moveToThread(self.playerBinThread)
        self.playerBinThread.start()

        self.dbThread = QThread()
        dbWorker.moveToThread(self.dbThread)
        self.dbThread.start()

    def initQMLContext(self):
        self.mainWindow.setContexts(contexts)
        self.mainWindow.setSource(QUrl.fromLocalFile(
            os.path.join(get_parent_dir(__file__, 2), 'views', 'Main.qml')))

        self.qmlDialog = QmlDialog(self.mainWindow.engine())
        self.mainWindow.setContext('QmlDialog', self.qmlDialog)

    def initConnect(self):
        self.web360ApiWorkerConnect()
        self.mediaPlayerConnect()
        self.playlistWorkerConnect()
        self.coverWorkerConnect()
        self.musicManageWorkerConnect()
        self.onlineMusicManageWorkerConnect()
        self.menuWorkerConnect()
        self.dbWorkerConnect()
        self.downloadSongWorkerConnect()

        signalManager.registerQmlObj.connect(self.mainWindow.setContext)
        signalManager.exited.connect(self.qApp.aboutToQuit)
        self.qApp.aboutToQuit.connect(self.close)

    def web360ApiWorkerConnect(self):
        web360ApiWorker.playMediaContent.connect(
            mediaPlayer.playOnlineMedia)
        web360ApiWorker.swicthMediaContent.connect(
            mediaPlayer.swicthOnlineMedia)

        web360ApiWorker.addMediaContent.connect(
            onlineMusicManageWorker.addSong)
        web360ApiWorker.addMediaContents.connect(
            onlineMusicManageWorker.addSongs)

        web360ApiWorker.addMediaContent.connect(
            playlistWorker.addOnlineMediaToTemporary)
        web360ApiWorker.addMediaContents.connect(
            playlistWorker.addOnlineMediasToTemporary)

        web360ApiWorker.addMediaContentToFavorite.connect(
            onlineMusicManageWorker.addSong)
        web360ApiWorker.addMediaContentToFavorite.connect(
            playlistWorker.addOnlineMediaToFavorite)
        web360ApiWorker.removeMediaContentFromFavorite.connect(
            playlistWorker.removeFavoriteMediaContent)

        web360ApiWorker.downloadSongConetent.connect(
            downloadSongWorker.downloadSong)

    def mediaPlayerConnect(self):
        mediaPlayer.requestMusic.connect(
            web360ApiWorker.switchMediaByUrl)

    def playlistWorkerConnect(self):
        playlistWorker.currentPlaylistChanged.connect(
            mediaPlayer.setPlaylistByName)

    def coverWorkerConnect(self):
        coverWorker.updateArtistCover.connect(
            musicManageWorker.updateArtistCover)
        coverWorker.updateAlbumCover.connect(
            musicManageWorker.updateAlbumCover)
        coverWorker.updateOnlineSongCover.connect(
            onlineMusicManageWorker.updateSongCover)
        coverWorker.updateAlbumCover.connect(
            onlineMusicManageWorker.updateSongCover)

    def musicManageWorkerConnect(self):
        musicManageWorker.saveSongToDB.connect(dbWorker.addSong)
        musicManageWorker.saveSongsToDB.connect(dbWorker.addSongs)
        musicManageWorker.restoreSongsToDB.connect(
            dbWorker.restoreSongs)
        musicManageWorker.addSongToPlaylist.connect(
            playlistWorker.addLocalMediaToTemporary)
        musicManageWorker.addSongsToPlaylist.connect(
            playlistWorker.addLocalMediasToTemporary)
        musicManageWorker.playSongByUrl.connect(
            mediaPlayer.playLocalMedia)

        musicManageWorker.downloadArtistCover.connect(
            coverWorker.downloadArtistCover)
        musicManageWorker.downloadAlbumCover.connect(
            coverWorker.downloadAlbumCover)

        musicManageWorker.loadDBSuccessed.connect(
            playlistWorker.loadPlaylists)
        musicManageWorker.loadDBSuccessed.connect(
            self.loadConfig)

    def onlineMusicManageWorkerConnect(self):
        onlineMusicManageWorker.downloadOnlineSongCover.connect(
            coverWorker.downloadOnlineSongCover)
        onlineMusicManageWorker.downloadOnlineAlbumCover.connect(
            coverWorker.downloadOnlineAlbumCover)
        onlineMusicManageWorker.downloadAlbumCover.connect(
            coverWorker.downloadAlbumCover)
        pass

    def menuWorkerConnect(self):
        # setting menu
        menuWorker.addSongFile.connect(musicManageWorker.addSongFile)
        menuWorker.addSongFolder.connect(
            musicManageWorker.searchOneFolderMusic)

        # artist menu
        menuWorker.playArtist.connect(musicManageWorker.playArtist)
        menuWorker.removeFromDatabaseByArtistName.connect(
            musicManageWorker.removeFromDatabaseByArtistName)
        menuWorker.removeFromDriverByArtistName.connect(
            musicManageWorker.removeFromDriverByArtistName)

        # album menu
        menuWorker.playAlbum.connect(musicManageWorker.playAlbum)
        menuWorker.removeFromDatabaseByAlbumName.connect(
            musicManageWorker.removeFromDatabaseByAlbumName)
        menuWorker.removeFromDriverByAlbumName.connect(
            musicManageWorker.removeFromDriverByAlbumName)

        # song menu

        menuWorker.orderByKey.connect(musicManageWorker.orderByKey)
        menuWorker.openSongFolder.connect(
            musicManageWorker.openSongFolder)
        menuWorker.removeFromDatabaseByUrl.connect(
            musicManageWorker.removeFromDatabaseByUrl)
        menuWorker.removeFromDriveByUrl.connect(
            musicManageWorker.removeFromDriveByUrl)

        # folder menu
        menuWorker.playFolder.connect(musicManageWorker.playFolder)
        menuWorker.removeFromDatabaseByFolderName.connect(
            musicManageWorker.removeFromDatabaseByFolderName)
        menuWorker.removeFromDriverByFolderName.connect(
            musicManageWorker.removeFromDriverByFolderName)

        #playlist menu
        menuWorker.removeFromPlaylist.connect(playlistWorker.removeFromPlaylist)

        #playlist navigation menu
        menuWorker.deletePlaylist.connect(playlistWorker.deletePlaylist)

        #download menu
        menuWorker.playMusicByIdSignal.connect(web360ApiWorker.playMusicByIdSignal)
        menuWorker.removeFromDownloadList.connect(downloadSongWorker.delSongObj)

        #public menu:
        menuWorker.addSongToPlaylist.connect(playlistWorker.addSongToPlaylist)
        menuWorker.addSongsToPlaylist.connect(playlistWorker.addSongsToPlaylist)

    def dbWorkerConnect(self):
        dbWorker.restoreSongsSuccessed.connect(
            musicManageWorker.loadDB)

    def downloadSongWorkerConnect(self):
        downloadSongWorker.addDownloadSongToDataBase.connect(musicManageWorker.addLocalSongToDataBase)

    def loadConfig(self):
        mediaPlayer.setPlaylistByName(configWorker.lastPlaylistName)
        mediaPlayer.setCurrentIndex(configWorker.lastPlayedIndex)
        mediaPlayer.volume = configWorker.volume
        mediaPlayer.playbackMode = configWorker.playbackMode

    def showMainWindow(self):
        self.mainWindow.show()
        self.mainWindow.setRoundMask()
        self.qApp.setActivationWindow(self.mainWindow)

    def show(self):
        self.showMainWindow()

    def clearCache(self):
        if isWebengineUsed:
            self.mainWindow.engine().clearComponentCache()
            self.mainWindow.engine().collectGarbage()
            self.mainWindow.engine().trimComponentCache()
        else:
            self.mainWindow.engine().collectGarbage()
            self.mainWindow.engine().trimComponentCache()

    def close(self):
        QThreadPool.globalInstance().clear()
        mediaPlayer.stop()
        configWorker.save()
        playlistWorker.savePlaylists()
