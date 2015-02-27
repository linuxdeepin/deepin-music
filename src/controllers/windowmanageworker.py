#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot

from .utils import registerContext


class WindowManageWorker(QObject):

    mainWindowShowed = pyqtSignal()
    simpleWindowShowed = pyqtSignal()
    miniWindowShowed = pyqtSignal()

    __contextName__ = 'WindowManageWorker'

    @registerContext
    def __init__(self, parent=None):
        super(WindowManageWorker, self).__init__(parent)
