#!/usr/bin/python
# -*- coding: utf-8 -*-

from PyQt5.QtCore import (QCoreApplication, QObject, 
    QUrl, QThread, QTimer,
    QThreadPool)
from PyQt5.QtGui import QScreen
from views import MainWindow

from controllers import contexts, Web360ApiWorker, MusicManageWorker
from controllers import MenuWorker, WindowManageWorker
from controllers import MediaPlayer, PlaylistWorker, CoverWorker
from controllers import ConfigWorker, DBWorker, I18nWorker
from controllers import UtilWorker
from controllers.mediaplayer import gPlayer

# from models import MusicDataBase

import config


class DeepinPlayer(QObject):

    _instance = None

    def __init__(self):
        super(DeepinPlayer, self).__init__()
        self.initApplication()
        self.initView()
        self.initControllers()
        self.initConnect()
        self.initQMLContext()
        self.loadConfig()
        self.loadDB()

        self.timer = QTimer()
        self.timer.timeout.connect(self.clearCache)
        self.timer.start(2000)

    @classmethod
    def instance(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    @property
    def qApp(self):
        return QCoreApplication.instance()

    def initApplication(self):
        self.qApp.setApplicationName(config.applicationName)
        self.qApp.setApplicationVersion(config.applicationVersion)
        self.qApp.setOrganizationName(config.organizationName)

    def loadDB(self):
        # QTimer.singleShot(1000, self.musicManageWorker.loadDB)
        self.musicManageWorker.restoreDB()

    def initView(self):
        self.mainWindow = MainWindow()

    def initControllers(self):
        self.utilWorker = UtilWorker()
        self.dbWorker = DBWorker()
        self.configWorker = ConfigWorker()
        self.i18nWorker = I18nWorker()
        self.coverWorker = CoverWorker()

        self.windowManageWorker = WindowManageWorker()
        self.web360ApiWorker = Web360ApiWorker()
        self.musicManageWorker = MusicManageWorker()
        self.menuWorker = MenuWorker()

        self.mediaPlayer = MediaPlayer()
        self.playlistWorker = PlaylistWorker()

        self.web360Thread = QThread(self)
        self.web360ApiWorker.moveToThread(self.web360Thread)
        self.web360Thread.start()

        self.playerBinThread = QThread()
        gPlayer.moveToThread(self.playerBinThread)
        self.playerBinThread.start()

        self.dbThread = QThread()
        self.dbWorker.moveToThread(self.dbThread)
        self.dbThread.start()

    def initQMLContext(self):
        self.mainWindow.setContexts(contexts)
        self.mainWindow.setSource(QUrl('views/Main.qml'))

    def initConnect(self):
        self.web360ApiWorkerConnect()
        self.mediaPlayerConnect()
        self.playlistWorkerConnect()
        self.coverWorkerConnect()
        self.musicManageWorkerConnect()
        self.menuWorkerConnect()
        self.dbWorkerConnect()
        self.qApp.aboutToQuit.connect(self.close)

    def web360ApiWorkerConnect(self):
        self.web360ApiWorker.playMediaContent.connect(self.mediaPlayer.playOnlineMedia)
        self.web360ApiWorker.swicthMediaContent.connect(self.mediaPlayer.swicthOnlineMedia)

        self.web360ApiWorker.addMediaContent.connect(self.playlistWorker.addOnlineMediaToTemporary)
        self.web360ApiWorker.addMediaContents.connect(self.playlistWorker.addOnlineMediasToTemporary)

        self.web360ApiWorker.addMediaContentToFavorite.connect(self.playlistWorker.addOnlineMediaToFavorite)
        self.web360ApiWorker.removeMediaContentFromFavorite.connect(self.playlistWorker.removeFavoriteMediaContent)

    def mediaPlayerConnect(self):
        self.mediaPlayer.requestMusic.connect(self.web360ApiWorker.switchMediaByUrl)
        self.mediaPlayer.downloadCover.connect(self.coverWorker.downloadCoverByUrl)

    def playlistWorkerConnect(self):
        self.playlistWorker.currentPlaylistChanged.connect(self.mediaPlayer.setPlaylistByName)

    def coverWorkerConnect(self):
        self.coverWorker.downloadCoverSuccessed.connect(self.mediaPlayer.updateCover)
        self.coverWorker.updateArtistCover.connect(self.musicManageWorker.updateArtistCover)
        self.coverWorker.updateAlbumCover.connect(self.musicManageWorker.updateAlbumCover)
        self.coverWorker.allTaskFinished.connect(self.musicManageWorker.stopUpdate)

    def musicManageWorkerConnect(self):
        self.musicManageWorker.saveSongToDB.connect(self.dbWorker.addSong)
        self.musicManageWorker.saveSongsToDB.connect(self.dbWorker.addSongs)
        self.musicManageWorker.restoreSongsToDB.connect(self.dbWorker.restoreSongs)
        self.musicManageWorker.addSongToPlaylist.connect(self.playlistWorker.addLocalMediaToTemporary)
        self.musicManageWorker.addSongsToPlaylist.connect(self.playlistWorker.addLocalMediasToTemporary)
        self.musicManageWorker.playSongByUrl.connect(self.mediaPlayer.playLocalMedia)

        self.musicManageWorker.downloadArtistCover.connect(self.coverWorker.downloadArtistCover)
        self.musicManageWorker.downloadAlbumCover.connect(self.coverWorker.downloadAlbumCover)

    def menuWorkerConnect(self):
        self.menuWorker.addSongFile.connect(self.musicManageWorker.addSongFile)
        self.menuWorker.addSongFolder.connect(self.musicManageWorker.searchOneFolderMusic)

    def dbWorkerConnect(self):
        self.dbWorker.restoreSongsSuccessed.connect(self.musicManageWorker.loadDB)

    def loadConfig(self):
        self.mediaPlayer.setPlaylistByName(self.configWorker.lastPlaylistName);
        self.mediaPlayer.setCurrentIndex(self.configWorker.lastPlayedIndex);
        self.mediaPlayer.volume = self.configWorker.volume
        self.mediaPlayer.playbackMode = self.configWorker.playbackMode

    def showMainWindow(self):
        self.mainWindow.show()
        self.qApp.setActivationWindow(self.mainWindow)

    def show(self):
        self.showMainWindow()

    def clearCache(self):
        self.mainWindow.engine().clearComponentCache()
        self.mainWindow.engine().collectGarbage()
        self.mainWindow.engine().trimComponentCache()

    def close(self):
        QThreadPool.globalInstance().clear()
        self.mediaPlayer.stop()
        self.configWorker.save()
        self.playlistWorker.savePlaylists()
