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

    downloadSuccessed = pyqtSignal('QString')

    @registerContext
    def __init__(self, parent=None):
        super(CoverWorker, self).__init__(parent)
        self._covers = {}

        # self.downloadSuccessed.connect(self.)

    @classmethod
    def md5(cls, string):
        import hashlib
        md5Value = hashlib.md5(string)
        return md5Value.hexdigest()

    @classmethod
    def getCoverPathByUrl(cls, url):
        coverID = cls.md5(url)
        filename = '%s' % coverID
        filepath = os.path.join(CoverPath, filename)
        return filepath

    @pyqtSlot('QString')
    @dthread
    def downloadCover(self, coverUrl):
        try:
            r = requests.get(coverUrl)
            with open(filepath, "wb") as f:
                f.write(r.content)
        except:
            return
        self.downloadSuccessed.emit()
