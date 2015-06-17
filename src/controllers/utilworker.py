#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl)
from PyQt5.QtGui import QCursor, QDesktopServices
from .utils import registerContext, duration_to_string


class UtilWorker(QObject):

    __contextName__ = 'UtilWorker'

    @registerContext
    def __init__(self, parent=None):
        super(UtilWorker, self).__init__(parent)

    @pyqtSlot(int, result='QString')
    def int_to_string(self, value):
        return str(value)

    @pyqtSlot(int, result='QString')
    def duration_to_string(self, duration):
        return duration_to_string(duration)

    @pyqtSlot(int, result='QString')
    def size_to_string(self, size):
        ''' convert file size byte to MB. '''
        return '%.2f MB' % (float(size) / (1024 * 1024))

    @pyqtSlot(int, result='QString')
    def bitrate_to_string(self, bitrate):
        ''' convert file size byte to K bit. '''
        return '%.2f K' % (float(bitrate) / 1000)

    @pyqtSlot(int, result='QString')
    def sampleRate_to_string(self, sampleRate):
        ''' convert file size byte to KHz. '''
        return '%.2f KHz' % (float(sampleRate) / 1000)

    @pyqtSlot(int, result='QString')
    def progress_to_string(self, progress):
        ''' convert progress to string like 99%. '''
        return '%d%%' % float(progress)

    @pyqtSlot('QString', result='QString')
    def basename(self, path):
        return os.path.basename(path)

    @pyqtSlot('QString')
    def openUrl(self, path):
        QDesktopServices.openUrl(QUrl.fromLocalFile(path))


utilWorker = UtilWorker()
