#!/usr/bin/python
# -*- coding: utf-8 -*-

from PyQt5.QtCore import QCoreApplication, QObject, QUrl, QThread
from views import MainWindow

from controllers import contexts, Web360ApiWorker

import config


class DeepinPlayer(QObject):

    def __init__(self):
        super(DeepinPlayer, self).__init__()
        self.initApplication()
        self.loadDB()
        self.initView()
        self.initControllers()
        self.initQMLContext()

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
        self.qApp.setActivationWindow(self.mainWindow)

    def initControllers(self):
        self.web360ApiWorker = Web360ApiWorker()

    def initQMLContext(self):
        self.mainWindow.setContexts(contexts)

    def show(self):
        self.mainWindow.setSource(QUrl('views/Main.qml'))
        self.mainWindow.show()


        self.mainWindow.quickItems['webMusic360Page'].playMusicByID.connect(self.web360ApiWorker.getMusicURLByID)
        self.web360ApiWorker.playUrl.connect(self.mainWindow.quickItems['MainMusic'].playMusic)
