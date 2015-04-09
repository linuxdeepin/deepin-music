#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext, duration_to_string


class UtilWorker(QObject):

    __contextName__ = 'UtilWorker'

    @registerContext
    def __init__(self, parent=None):
        super(UtilWorker, self).__init__(parent)

    @pyqtSlot(int, result='QString')
    def duration_to_string(self, duration):
        return duration_to_string(duration)
