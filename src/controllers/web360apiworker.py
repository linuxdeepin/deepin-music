#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot
import requests
from .utils import registerContext
from dwidgets import dthread
import threading
import copy
from log import logger


class Web360ApiWorker(QObject):

    playMediaContent = pyqtSignal('QVariant')

    requestSuccessed = pyqtSignal('QString', int, dict)

    __contextName__ = 'Web360ApiWorker'

    @registerContext
    def __init__(self, parent=None):
        super(Web360ApiWorker, self).__init__(parent)
        self.playedMusics = {}
        self.recommendedMusics = {}
        self.recommendedmusicIds = {}
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

    @classmethod
    def getUrlByID(cls, musicId):
        sign = cls.md5(musicId)
        params = {
            'id': musicId,
            'src': 'linuxdeepin',
            'sign': sign
        }

        url = 'http://s.music.haosou.com/player/songForPartner?id=%s&src=%s&sign=%s'\
            %(params['id'], params['src'], params['sign'])
        return url

    def getResultById(self, musicId):
        url = self.getUrlByID(musicId)
        result = self.getResultByUrl(url)
        return result

    def getResultByUrl(self, url):
        tags = None
        maxQueryCount = 5
        i = 0
        while i < maxQueryCount:
            try:
                ret = requests.get(url)
                tags = ret.json()
                if isinstance(tags, dict):
                    break
                else:
                    i = i + 1
            except:
                i = i + 1

        result = {
            'url': url,
            'tags': tags
        }
        if isinstance(tags, list) and not tags:
            result = None
        return result

    @pyqtSlot(int)
    def playMusicById(self, musicId):
        self.playMediaById(musicId)

    @dthread
    @pyqtSlot(int)
    def playMediaById(self, musicId):
        result = self.getResultById(musicId)
        if result:
            self.playMediaContent.emit(result)

    @dthread
    @pyqtSlot('QString')
    def playMediaByUrl(self, url):
        result = self.getResultByUrl(url)
        if result:
            self.playMediaContent.emit(result)

    @dthread
    def getQueueResults(self, musicIdString, musicId):
        result = self.getResultById(musicId)
        if result:
            self.requestSuccessed.emit(musicIdString, musicId, result)

    @pyqtSlot('QString')
    def playMusicByIds(self, musicIdString):
        _musicIds = [int(k) for k in musicIdString.split('_')]
        self.recommendedMusics[musicIdString] = {}
        self.recommendedmusicIds[musicIdString] = _musicIds
        for musicId in _musicIds:
            self.getQueueResults(musicIdString, musicId)

    @pyqtSlot(' QString', int, dict)
    def collectResults(self, musicIdString, musicId, result):
        self.recommendedMusics[musicIdString].update({musicId: result})
        _results = self.recommendedMusics[musicIdString]
        _musicIds = self.recommendedmusicIds[musicIdString]
        if len(_results) == len(_musicIds):
            for musicId in  _musicIds:
                result = _results[musicId]
                self.playMediaContent.emit(result)
            self.playMediaContent.emit(_results[_musicIds[0]])
