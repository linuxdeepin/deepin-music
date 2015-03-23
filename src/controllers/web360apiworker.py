#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import json
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QUrl
import requests
from .utils import registerContext, duration_to_string
from .mediaplayer import MediaPlayer
from dwidgets import dthread
import copy
from log import logger


songlist = {
    u'新歌榜': 'http://music.haosou.com/data/top/top_new.json',
    u'热歌榜': 'http://music.haosou.com/data/top/top_hot.json',
    u'咪咕音乐榜': 'http://music.haosou.com/data/top/top_migu.json',
    u'网络流行榜': 'http://music.haosou.com/data/top/top_internet.json',
    u'中国歌曲排行榜': 'http://music.haosou.com/data/top/top_zggqphb.json',
    u'日本ORICON': 'http://music.haosou.com/data/top/top_gongxin.json',
    u'英国UK Official': 'http://music.haosou.com/data/top/top_uk.json',
    u'美国The Billboard': 'http://music.haosou.com/data/top/top_bill.json',
    u'Channel V': 'http://music.haosou.com/data/top/top_channelv.json',
    u'香港中文龙虎榜': 'http://music.haosou.com/data/top/top_zwlhb.json',
    u'经典老歌': 'http://music.haosou.com/data/top/top_classic.json'
}


class Web360ApiWorker(QObject):

    playMediaContent = pyqtSignal('QVariant')
    swicthMediaContent = pyqtSignal('QVariant')

    addMediaContents = pyqtSignal(list)
    addMediaContent = pyqtSignal('QVariant')

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

    @classmethod
    def request(cls, url, count=5):
        result = None
        i = 0
        while i < count:
            try:
                ret = requests.get(url)
                result = ret.json()
                if isinstance(result, dict):
                    break
                else:
                    i = i + 1
            except ValueError:
                result = ret.text
                break
            except:
                i = i + 1
        return result

    def getResultById(self, musicId):
        url = self.getUrlByID(musicId)
        result = self.getResultByUrl(url)

        return result

    def getResultByUrl(self, url):
        tags = self.request(url)
        result = {
            'url': url,
            'tags': tags,
            'updated': False
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
            self.addMediaContent.emit(result)
            self.playMediaContent.emit(result)

    @dthread
    @pyqtSlot('QString')
    def playMediaByUrl(self, url):
        result = self.getResultByUrl(url)
        if result:
            self.playMediaContent.emit(result)

    @dthread
    @pyqtSlot('QString')
    def switchMediaByUrl(self, url):
        result = self.getResultByUrl(url)
        if result:
            self.swicthMediaContent.emit(result)

    @pyqtSlot('QString')
    def playMusicBySonglist(self, songlistName):
        url = songlist[songlistName]
        result = self.request(url)
        if result:
            header = 'cb_' + url.split('/')[-1].split('.')[0]
            content = result[len(header) + 1: -1]
            ret = json.loads(content)

            results = []
            for song in ret['songList']:
                url = self.getUrlByID(song['songId'])
                tags = song
                result = {
                    'url': url,
                    'tags': tags,
                    'updated': True
                }
                results.append(result)
            self.addMediaContents.emit(results)

            url = self.getUrlByID(ret['songList'][0]['songId'])
            self.playMediaByUrl(url)

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
