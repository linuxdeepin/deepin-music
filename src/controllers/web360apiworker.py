#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot

from .utils import registerContext
from dwidgets import dthread
import threading
import copy
from log import logger


class Web360ApiWorker(QObject):

    addMediaContent = pyqtSignal('QVariant')

    requestSuccessed = pyqtSignal(int, dict)

    __contextName__ = 'Web360ApiWorker'

    @registerContext
    def __init__(self, parent=None):
        super(Web360ApiWorker, self).__init__(parent)
        self._musicIds = []
        self._results = {}
        self.initConnect()

    def initConnect(self):
        self.requestSuccessed.connect(self.collectResults)

    @classmethod
    def md5(cls, musicId):
        import hashlib
        s = 'id=%d_projectName=linuxdeepin' % (musicId)
        md5Value = hashlib.md5(s)
        return md5Value.hexdigest()

    def getResult(self, musicId):
        import hashlib
        import requests
        sign = self.md5(musicId)
        params = {
            'id': musicId,
            'src': 'linuxdeepin',
            'sign': sign
        }
        ret = requests.get("http://s.music.haosou.com/player/songForPartner", params=params)

        result = {
            'url': ret.url,
            'ret': ret.json()
        }

        return result

    @dthread
    @pyqtSlot(int)
    def getMusicUrlById(self, musicId):
        result = self.getResult(musicId)
        self.addMediaContent.emit(result)

    @dthread
    def getQueueResults(self, musicId):
        result = self.getResult(musicId)
        self.requestSuccessed.emit(musicId, result)

    @pyqtSlot('QString')
    def getMusicUrlByIds(self, musicIds):
        musicIds = [int(k) for k in musicIds.split('_')]
        for musicId in musicIds:
            self._musicIds.append(musicId)
            self.getQueueResults(musicId)

    @pyqtSlot(int, dict)
    def collectResults(self, musicId, result):
        self._results.update({musicId: result})
        if len(self._results) == len(self._musicIds):
            results = copy.deepcopy(self._results)
            musicIds =copy.deepcopy(self._musicIds)
            for musicId in  musicIds:
                result = results[musicId]
                self.addMediaContent.emit(result)
                self._musicIds.remove(musicId) 
                self._results.pop(musicId)
            self.addMediaContent.emit(results[musicIds[0]])
