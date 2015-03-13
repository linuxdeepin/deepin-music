#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl, QFile, QIODevice
    )
from PyQt5.QtMultimedia import QMediaPlaylist, QMediaContent
from .utils import registerContext, contexts
from dwidgets.mediatag.song import Song
from dwidgets import dthread
from config.constants import PlaylistPath
from log import logger


class DLocalMediaContent(QMediaContent):

    def __init__(self, url):
        super(DLocalMediaContent, self).__init__(QUrl.fromLocalFile(url))
        self.song = Song(url)
        self._url = url

    @pyqtProperty('QString')
    def url(self):
        return self._url

    @pyqtProperty('QString')
    def title(self):
        return self.song['title']

    @pyqtProperty('QString')
    def artist(self):
        return self.song['artist']


class DOnlineMediaContent(QMediaContent):

    __keys__ = {
        'title': 'songName',
        'artist': 'singerName',
        'album': 'albumName',
        'albumImage_100x100': 'albumImage_100x100',
        'albumImage_500x500': 'albumImage_500x500',

        'songId': 'songId',
        'singerId': 'singerId',
        'albumId': 'albumId',

        'serviceEngName': 'serviceEngName',
        'serviceName': 'serviceName',
        'serviceUrl': 'serviceUrl',

        'playlinkUrl': 'playlinkUrl'
    }

    def __init__(self, url, ret):
        super(DOnlineMediaContent, self).__init__(QUrl(url))
        self.song = {}
        for key in self.__keys__:
            k = self.__keys__[key]
            if k in ret:
                self.song[key] = ret[k]

        self._url = url

    @pyqtProperty('QString')
    def url(self):
        return self._url

    @pyqtProperty('QString')
    def playLinkUrl(self):
        if 'playlinkUrl' in self.song:
            return self.song['playlinkUrl']
        else:
            return ''

    @pyqtProperty('QString')
    def title(self):
        return self.song['title']

    @pyqtProperty('QString')
    def artist(self):
        return self.song['artist']

    @classmethod
    def md5(cls, musicId):
        import hashlib
        s = 'id=%d_projectName=linuxdeepin' % (musicId)
        md5Value = hashlib.md5(s)
        return md5Value.hexdigest()

    @dthread
    @pyqtSlot(int)
    def getMusicURLByID(self, musicId):
        import hashlib
        import requests
        sign = self.md5(musicId)
        params = {
            'id': musicId,
            'src': 'linuxdeepin',
            'sign': sign
        }
        ret = requests.get("http://s.music.haosou.com/player/songForPartner", params=params)
        jsonRet = ret.json()
        self.emitURL(jsonRet)
        return jsonRet


class DMediaPlaylist(QMediaPlaylist):

    nameChanged = pyqtSignal('QString')

    mediasChanged = pyqtSignal('QVariant')

    def __init__(self, name=''):
        super(DMediaPlaylist, self).__init__()
        self._name = name
        self._urls = []
        self._mediaContents = {}

    @pyqtProperty('QString')
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        self._name = name
        self.nameChanged.emit()

    @pyqtProperty('QVariant')
    def urls(self):
        return self._urls

    @pyqtProperty('QVariant')
    def mediaContents(self):
        return self._mediaContents

    @pyqtProperty('QVariant', notify=mediasChanged)
    def medias(self):
        medias = []
        for key in self._urls:
            mediaContent = self._mediaContents[key]
            media = {'title': mediaContent.title, 'artist': mediaContent.artist}
            medias.append(media)
        return medias

    def addMedia(self, url, ret=None):
        url = str(url)
        if url not in self._urls:
            if url.startswith('http://') or url.startswith('https://'):
                self.addOnlineMedia(url, ret)
            else:
                self.addLocalMedia(url)
            self._urls.append(url)

        index = self._urls.index(url)
        self.setCurrentIndex(index)

        medias = self.medias
        self.mediasChanged.emit(medias)

    def addLocalMedia(self, url):
        mediaContent = DLocalMediaContent(url)
        self._mediaContents.update({url: mediaContent})
        super(DMediaPlaylist, self).addMedia(mediaContent)

    def addOnlineMedia(self, url, ret):
        mediaContent = DOnlineMediaContent(url, ret)
        self._mediaContents.update({url: mediaContent})
        super(DMediaPlaylist, self).addMedia(mediaContent)


class PlaylistWorker(QObject):

    __contextName__ = 'PlaylistWorker'

    nameExisted = pyqtSignal('QString')

    @registerContext
    def __init__(self, parent=None):
        super(PlaylistWorker, self).__init__(parent)

        self._playlists = {}
        self._currentPlaylist = None
        self._playlists['temporary'] = DMediaPlaylist('temporary')
        self._playlists['favorite'] = DMediaPlaylist('favorite')

        self.initPlaylist()


    def initPlaylist(self):
        # self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', '游鸿明 - 往快乐逃.flac']))
        # self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', 'Track01.wav']))
        # self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', '游鸿明-下沙.ape']))
        # self.addMediaToTemporary(os.sep.join([os.environ['HOME'], 'workspace', 'yhm', '游鸿明 - 红糖水.flac']))
        self.addMediaToTemporary(os.sep.join([os.path.dirname(os.getcwd()), 'music', '1.mp3']))
        self.addMediaToTemporary(os.sep.join(['/usr/share/deepin-sample-music/邓入比_我们的情歌.mp3']))
        self.addMediaToTemporary(os.sep.join(['/usr/share/deepin-sample-music/郭一凡_说走就走的旅行.mp3']))
        self.addMediaToTemporary(os.sep.join(['/usr/share/deepin-sample-music/胡彦斌_依然是你.mp3']))


        
        self.addMediaToFavorite(os.sep.join(['/usr/share/deepin-sample-music/邓入比_我们的情歌.mp3']))
        self.addMediaToFavorite(os.sep.join(['/usr/share/deepin-sample-music/郭一凡_说走就走的旅行.mp3']))
        self.addMediaToFavorite(os.sep.join([os.path.dirname(os.getcwd()), 'music', '1.mp3']))
        self.addMediaToFavorite(os.sep.join(['/usr/share/deepin-sample-music/胡彦斌_依然是你.mp3']))


    def savePlaylistByName(self, name):
        f = QFile(os.sep.join([PlaylistPath, '%s.m3u' % name]))
        flag = f.open(QIODevice.ReadWrite)
        if flag:
            self._playlists[name].save(f, 'm3u')
            f.close()

    @pyqtProperty('QMediaPlaylist')
    def termporaryPlaylist(self):
        return self._playlists['temporary']

    @pyqtSlot('QString', result='QMediaPlaylist')
    def getPlaylistByName(self, name):
        if name in self._playlists:
            return self._playlists[name]
        else:
            return None

    @pyqtProperty(dict)
    def playlists(self):
        return self._playlists

    @pyqtSlot('QString')
    def addMediaToTemporary(self, url):
        self._playlists['temporary'].addMedia(url)

    @pyqtSlot('QString')
    def addMediaToFavorite(self, url):
        self._playlists['favorite'].addMedia(url)

    @pyqtSlot('QString', 'QString')
    def addMediaByName(self, name, url):
        if name in self._playlists:
            self._playlists[name].addMedia(url)
        else:
            logger.info("the playlist named %s isn't existed" % name)

    @pyqtSlot('QString')
    def createPlaylistByName(self, name):
        if name in self._playlists:
            self.nameExisted.emit(name)
        else:
            self._playlists[name] = DMediaPlaylist(name)

    @pyqtSlot()
    def playOnlineMusic(self, result):
        pass



