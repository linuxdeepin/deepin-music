#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext


class WindowManageWorker(QObject):

    mainWindowShowed = pyqtSignal()
    simpleWindowShowed = pyqtSignal()
    miniWindowShowed = pyqtSignal()

    switchPageByID = pyqtSignal('QString')

    __contextName__ = 'WindowManageWorker'

    @registerContext
    def __init__(self, parent=None):
        super(WindowManageWorker, self).__init__(parent)

        self._WindowMode = 'Full'
        self._lastWindowMode = 'Full'

    @pyqtProperty('QPoint')
    def cursorPos(self):
        return QCursor.pos()

    @pyqtProperty(str)
    def windowMode(self):
        return self._WindowMode

    @windowMode.setter
    def windowMode(self, mode):
        self._WindowMode = mode

    @pyqtProperty(str)
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
