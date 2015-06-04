#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext
from .signalmanager import signalManager


class WindowManageWorker(QObject):

    mainWindowShowed = pyqtSignal()
    simpleWindowShowed = pyqtSignal()
    miniWindowShowed = pyqtSignal()
    currentMusicManagerPageNameChanged = pyqtSignal('QString')

    switchPageByID = pyqtSignal('QString')

    __contextName__ = 'WindowManageWorker'

    @registerContext
    def __init__(self, parent=None):
        super(WindowManageWorker, self).__init__(parent)

        self._windowMode = 'Full'
        self._lastWindowMode = 'Full'
        self._currentMusicManagerPageName = 'ArtistPage'

        self.initConnect()

    def initConnect(self):
        signalManager.simpleFullToggle.connect(self.actionSimpleFullToggle)
        signalManager.miniFullToggle.connect(self.actionMiniFullToggle)
        signalManager.fullMode.connect(self.actionFullMode)
        signalManager.simpleMode.connect(self.actionSimpleMode)
        signalManager.miniMode.connect(self.actionMiniMode)

    @pyqtProperty('QPoint')
    def cursorPos(self):
        return QCursor.pos()

    @pyqtProperty('QString')
    def windowMode(self):
        return self._windowMode

    @windowMode.setter
    def windowMode(self, mode):
        self._windowMode = mode

    @pyqtProperty('QString')
    def lastWindowMode(self):
        return self._lastWindowMode

    @lastWindowMode.setter
    def lastWindowMode(self, mode):
        self._lastWindowMode = mode

    @pyqtSlot()
    def showNormal(self):
        if self.lastWindowMode == "Full":
            self.mainWindowShowed.emit()
        elif self.lastWindowMode == "Simple":
            self.simpleWindowShowed.emit()
        else:
            pass

    @pyqtProperty('QString', notify=currentMusicManagerPageNameChanged)
    def currentMusicManagerPageName(self):
        return self._currentMusicManagerPageName

    @currentMusicManagerPageName.setter
    def currentMusicManagerPageName(self, value):
        self._currentMusicManagerPageName = value
        self.currentMusicManagerPageNameChanged.emit(value)

    def actionSimpleFullToggle(self):
        if self._windowMode == 'Full':
            self.simpleWindowShowed.emit()
        elif self._windowMode == 'Simple':
            self.mainWindowShowed.emit()

    def actionMiniFullToggle(self):
        if self._windowMode == 'Full':
            self.miniWindowShowed.emit()
        elif self._windowMode == 'Mini':
            self.mainWindowShowed.emit()

    def actionFullMode(self):
        self.mainWindowShowed.emit()

    def actionSimpleMode(self):
        self.simpleWindowShowed.emit()

    def actionMiniMode(self):
        self.miniWindowShowed.emit()

windowManageWorker = WindowManageWorker()
