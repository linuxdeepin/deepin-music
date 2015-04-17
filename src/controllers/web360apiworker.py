#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import json
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QUrl
import requests
import threading
from .utils import registerContext, duration_to_string
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
    addMediaContentToFavorite = pyqtSignal('QVariant')
    removeMediaContentFromFavorite = pyqtSignal('QString')

    playMusicByIdSignal = pyqtSignal(int)
    playMusicByIdsSignal = pyqtSignal('QString')
    playSonglistByNameSignal = pyqtSignal('QString')
    playSonglistByIdSignal = pyqtSignal(int)
    playAlbumByIdSignal = pyqtSignal(int)
    addFavoriteSignal = pyqtSignal(int)
    removeFavoriteSignal = pyqtSignal(int)

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
        self.playMusicByIdSignal.connect(self.playMusicById)
        self.playMusicByIdsSignal.connect(self.playMusicByIds)
        self.playSonglistByNameSignal.connect(self.playMusicBySonglistName)
        self.playSonglistByIdSignal.connect(self.playMusicBySonglistId)
        self.playAlbumByIdSignal.connect(self.playMusicByAlbumId)

        self.addFavoriteSignal.connect(self.addMusicToFavorite)
        self.removeFavoriteSignal.connect(self.removeMusicFromFavorite)
        pass

    @classmethod
    def md5(cls, musicId):
        import hashlib
        s = 'id=%s_projectName=linuxdeepin' % (musicId)
        md5Value = hashlib.md5(s)
        return md5Value.hexdigest()

    @classmethod
    def md5s(cls, musicIds):
        import hashlib
        s = 'ids=%s_projectName=linuxdeepin' % (musicIds)
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
    def getUrlByIDs(cls, musicIds):
        sign = cls.md5s(musicIds)
        params = {
            'ids': musicIds,
            'src': 'linuxdeepin',
            'sign': sign
        }

        url = 'http://s.music.haosou.com/player/songlistForPartner?ids=%s&src=%s&sign=%s'\
            %(params['ids'], params['src'], params['sign'])
        return url

    @classmethod
    def request(cls, url, count=5):
        result = None
        ret = None
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
                break
            except:
                i = i + 1
        return result

    def getResultById(self, musicId):
        url = self.getUrlByID(musicId)
        result = self.getResultByUrl(url)

        return result

    @classmethod
    def getResultByUrl(cls, url):
        tags = cls.request(url)
        if isinstance(tags, list) and not tags:
            tags = None
        else:
            tags.update({'url': url})

        return tags

    @pyqtSlot(int)
    def playMusicById(self, musicId):
        self.playMediaById(musicId)

    @dthread
    @pyqtSlot('QString')
    def playMusicByIds(self, musicIds):
        print(threading.currentThread())
        url = self.getUrlByIDs(musicIds)
        songLists = self.request(url)

        results = []
        for song in songLists:
            url = self.getUrlByID(song['songId'])
            song.update({'url': url})
            results.append(song)

        if results:
            self.addMediaContents.emit(results)
            url = self.getUrlByID(songLists[0]['songId'])
            self.playMediaByUrl(url)

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

    @classmethod
    def getSongList(self, ret):
        results = []
        if 'songList' in ret:
            for song in ret['songList']:
                url = self.getUrlByID(song['songId'])
                tags = song
                tags.update({'url': url})
                results.append(tags)
        return results

    @dthread
    @pyqtSlot('QString')
    def playMusicBySonglistName(self, songlistName):
        if songlistName in songlist:
            url = songlist[songlistName]
            result = self.request(url)
            if result:
                header = 'cb_' + url.split('/')[-1].split('.')[0]
                content = result[len(header) + 1: -1]
                ret = json.loads(content)
                results = self.getSongList(ret)
                self.addMediaContents.emit(results)

                url = self.getUrlByID(ret['songList'][0]['songId'])
                self.playMediaByUrl(url)

    @dthread
    @pyqtSlot(int)
    def playMusicBySonglistId(self, songlistId):
        url = "http://s.music.haosou.com/list/intfDetail?id=%d" % songlistId
        ret = self.request(url)
        if ret:
            results = self.getSongList(ret)
            self.addMediaContents.emit(results)
            url = self.getUrlByID(ret['songList'][0]['songId'])
            self.playMediaByUrl(url)

    @dthread
    @pyqtSlot(int)
    def playMusicByAlbumId(self, albumId):
        url = "http://s.music.haosou.com/album/intfDetail?id=%d" % albumId
        ret = self.request(url)
        if ret:
            results = self.getSongList(ret)
            if results:
                for result in results:
                    result.update({
                        'singerId': ret['singerId'],
                        'singerName': ret['singerName'],
                        'albumId': ret['albumId'],
                        'albumName': ret['albumName']
                    })
                self.addMediaContents.emit(results)
                url = self.getUrlByID(ret['songList'][0]['songId'])
                self.playMediaByUrl(url)


    @pyqtSlot(int)
    def addMusicToFavorite(self, musicId):
        url = self.getUrlByID(musicId)
        tags = self.request(url)
        tags.update({'url': url})
        self.addMediaContentToFavorite.emit(tags)

    @pyqtSlot(int)
    def removeMusicFromFavorite(self, musicId):
        url = self.getUrlByID(musicId)
        self.removeMediaContentFromFavorite.emit(url)

    # @dthread
    # def getQueueResults(self, musicIdString, musicId):
    #     result = self.getResultById(musicId)
    #     if result:
    #         self.requestSuccessed.emit(musicIdString, musicId, result)

    # @pyqtSlot('QString')
    # def playMusicByIds(self, musicIdString):
    #     _musicIds = [int(k) for k in musicIdString.split('_')]
    #     self.recommendedMusics[musicIdString] = {}
    #     self.recommendedmusicIds[musicIdString] = _musicIds
    #     for musicId in _musicIds:
    #         self.getQueueResults(musicIdString, musicId)

    # @pyqtSlot(' QString', int, dict)
    # def collectResults(self, musicIdString, musicId, result):
    #     self.recommendedMusics[musicIdString].update({musicId: result})
    #     _results = self.recommendedMusics[musicIdString]
    #     _musicIds = self.recommendedmusicIds[musicIdString]
    #     if len(_results) == len(_musicIds):
    #         for musicId in  _musicIds:
    #             result = _results[musicId]
    #             self.playMediaContent.emit(result)
    #         self.playMediaContent.emit(_results[_musicIds[0]])
