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

    __contextName__ = "CoverWorker"

    downloadCoverSuccessed = pyqtSignal('QString', 'QString')

    @registerContext
    def __init__(self, parent=None):
        super(CoverWorker, self).__init__(parent)
        self._covers = {}

        self.initConnect()

    def initConnect(self):
        pass

    @classmethod
    def md5(cls, string):
        import hashlib
        md5Value = hashlib.md5(string)
        return md5Value.hexdigest()

    @classmethod
    def getCoverPathByMediaUrl(cls, url):
        if isinstance(url, unicode):
            url = url.encode('utf-8')
        coverID = cls.md5(url)
        filename = '%s' % coverID
        filepath = os.path.join(CoverPath, filename)
        return filepath

    @pyqtSlot('QString', 'QString')
    @dthread
    def downloadCoverByUrl(self, mediaUrl, coverUrl):
        filepath = self.getCoverPathByMediaUrl(mediaUrl)
        try:
            r = requests.get(coverUrl)
            with open(filepath, "wb") as f:
                f.write(r.content)
            self.downloadCoverSuccessed.emit(mediaUrl, filepath)
        except:
            pass
