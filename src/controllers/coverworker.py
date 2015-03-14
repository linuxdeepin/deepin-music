#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import copy
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot
import requests
from log import logger
from .utils import registerContext
from dwidgets import dthread
from config.constants import CoverPath

class CoverWorker(QObject):

    __contextName__ = 'CoverWorker'

    coverUpdated = pyqtSignal('QString')

    @registerContext
    def __init__(self, parent=None):
        super(CoverWorker, self).__init__(parent)
        self._covers = {}

    @classmethod
    def md5(cls, string):
        import hashlib
        md5Value = hashlib.md5(string)
        return md5Value.hexdigest()

    @pyqtSlot('QString', 'QString', 'QString')
    @dthread
    def saveCover(self, url, title='', album=''):
        url = str(url)
        if url in self._covers:
            self.coverUpdated.emit(self._covers[url])
            return
        else:
            coverID = self.md5(url)
            filename = '%s_%s_%s.%s' % (title, album, coverID, url.split('.')[-1])
            filepath = os.path.join(CoverPath, filename)
            if os.path.exists(filepath):
                self._covers.update({url: filepath})
                self.coverUpdated.emit(filepath)
                return
            else:
                try:
                    r = requests.get(url)
                    with open(filepath, "wb") as f:
                        f.write(r.content)
                except:
                    return
                self._covers.update({url: filepath})

                self.coverUpdated.emit(filepath)
