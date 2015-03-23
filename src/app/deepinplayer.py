#!/usr/bin/python
# -*- coding: utf-8 -*-

from PyQt5.QtCore import QCoreApplication, QObject, QUrl, QThread
from PyQt5.QtGui import QScreen
from views import MainWindow

from controllers import contexts, Web360ApiWorker, MusicManageWorker
from controllers import MenuWorker, WindowManageWorker
from controllers import MediaPlayer, PlaylistWorker, CoverWorker
from controllers import ConfigWorker, DBWorker

from models import MusicDataBase

import config


class DeepinPlayer(QObject):

    def __init__(self):
        super(DeepinPlayer, self).__init__()
        self.initApplication()
        self.loadDB()
        self.initView()
        self.initControllers()
        self.initConnect()
        self.initQMLContext()
        self.loadConfig()

    @property
    def qApp(self):
        return QCoreApplication.instance()

    def initApplication(self):
        self.qApp.setApplicationName(config.applicationName)
        self.qApp.setApplicationVersion(config.applicationVersion)
        self.qApp.setOrganizationName(config.organizationName)

    def loadDB(self):
        pass

    def initView(self):
        self.mainWindow = MainWindow()

    def initControllers(self):
        self.configWorker = ConfigWorker()
        
        self.windowManageWorker = WindowManageWorker()
        self.web360ApiWorker = Web360ApiWorker()
        self.musicManageWorker = MusicManageWorker()
        self.menuWorker = MenuWorker()

        self.mediaPlayer = MediaPlayer()
        self.coverWorker = CoverWorker()
        self.playlistWorker = PlaylistWorker()

        self.musicDataBase = MusicDataBase()
        self.dbworker = DBWorker()

    def initQMLContext(self):
        self.mainWindow.setContexts(contexts)
        self.mainWindow.setSource(QUrl('views/Main.qml'))

    def initConnect(self):
        self.web360ApiWorker.playMediaContent.connect(self.mediaPlayer.playOnlineMedia)
        self.web360ApiWorker.swicthMediaContent.connect(self.mediaPlayer.swicthOnlineMedia)

        self.web360ApiWorker.addMediaContent.connect(self.playlistWorker.addOnlineMediaToTemporary)
        self.web360ApiWorker.addMediaContents.connect(self.playlistWorker.addOnlineMediasToTemporary)

        self.mediaPlayer.requestMusic.connect(self.web360ApiWorker.switchMediaByUrl)
        self.mediaPlayer.downloadCover.connect(self.coverWorker.downloadCoverByUrl)

        self.coverWorker.downloadCoverSuccessed.connect(self.mediaPlayer.updateCover)

        self.qApp.aboutToQuit.connect(self.close)

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

    def close(self):
        self.mediaPlayer.stop()
        self.playlistWorker.savePlaylists()
