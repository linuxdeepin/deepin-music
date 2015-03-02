#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext


class ConfigWorker(QObject):

    __contextName__ = 'ConfigWorker'

    @registerContext
    def __init__(self, parent=None):
        super(ConfigWorker, self).__init__(parent)

        self._mainWindowWidth = 882

    @pyqtProperty(str)
    def mainWindowWidth(self):
        return self._mainWindowWidth

    @mainWindowWidth.setter
    def mainWindowWidth(self, width):
        self._mainWindowWidth = width
