#!/usr/bin/python
# -*- coding: utf-8 -*-

from PyQt5.QtCore import QCoreApplication, QObject, QUrl, QThread
from PyQt5.QtGui import QScreen
from views import MainWindow, SimpleWindow, MiniWindow

from controllers import contexts, Web360ApiWorker, MusicManageWorker
from controllers import MenuWorker, WindowManageWorker
from controllers import MediaPlayer

import config


class DeepinPlayer(QObject):

    def __init__(self):
        super(DeepinPlayer, self).__init__()
        self.initApplication()
        self.loadDB()
        self.initView()
        self.initControllers()
        self.initQMLContext()

        self.initConnect()

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
        self.simpleWindow = SimpleWindow()
        self.miniWindow = MiniWindow()

    def initControllers(self):
        self.windowManageWorker = WindowManageWorker()
        self.web360ApiWorker = Web360ApiWorker()
        self.musicManageWorker = MusicManageWorker()
        self.menuWorker = MenuWorker()

        self.mediaPlayer = MediaPlayer()

    def initQMLContext(self):
        self.mainWindow.setContexts(contexts)
        self.mainWindow.setSource(QUrl('views/Main.qml'))

        self.simpleWindow.setContexts(contexts)
        self.simpleWindow.setSource(QUrl('views/SimpleWindow.qml'))

        self.miniWindow.setContexts(contexts)
        self.miniWindow.setSource(QUrl('views/MiniWindow.qml'))

    def initConnect(self):
        self.windowManageWorker.mainWindowShowed.connect(self.showMainWindow)
        self.windowManageWorker.simpleWindowShowed.connect(self.showSimpleWindow)
        self.windowManageWorker.miniWindowShowed.connect(self.showMiniWindow)

    def showMainWindow(self):
        self.simpleWindow.hide()
        self.miniWindow.hide()

        self.mainWindow.show()
        self.qApp.setActivationWindow(self.mainWindow)

    def showSimpleWindow(self):
        self.mainWindow.hide()
        self.miniWindow.hide()

        self.simpleWindow.setPosition(self.mainWindow.position())
        self.simpleWindow.show()
        self.qApp.setActivationWindow(self.simpleWindow)

    def showMiniWindow(self):
        self.mainWindow.hide()
        self.simpleWindow.hide()

        self.miniWindow.setPosition(QScreen.size().width() - 100, 100)
        self.miniWindow.show()
        self.qApp.setActivationWindow(self.miniWindow)

    def show(self):
        self.showMainWindow()
