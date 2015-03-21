#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty

from .utils import registerContext
from dwidgets.tornadotemplate import template


class MusicManageWorker(QObject):

    playUrl = pyqtSignal(unicode)

    __contextName__ = 'MusicManageWorker'

    @registerContext
    def __init__(self, parent=None):
        super(MusicManageWorker, self).__init__(parent)

    @pyqtProperty('QVariant')
    def categories(self):
        return []
