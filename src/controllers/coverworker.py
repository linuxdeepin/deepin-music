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

    @classmethod
    def getCoverPath(cls, url):
        coverID = cls.md5(url)
        filename = '%s' % coverID
        filepath = os.path.join(CoverPath, filename)
        return filepath

    @pyqtSlot('QVariant', 'QString')
    @dthread
    def downloadCover(self, mediaContent):
        cover = mediaContent.cover
        url = mediaContent.url
        if url in self._covers:
            self.coverUpdated.emit(self._covers[url])
            return
        else:
            filepath = self.getCoverPath(url)
            if os.path.exists(filepath):
                self._covers.update({url: filepath})
                self.coverUpdated.emit(filepath)
                return
            else:
                try:
                    r = requests.get(cover)
                    with open(filepath, "wb") as f:
                        f.write(r.content)
                except:
                    return
                self._covers.update({url: filepath})
                self.coverUpdated.emit(filepath)
