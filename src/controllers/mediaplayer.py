#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QDate)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from PyQt5.QtMultimedia import QMediaPlayer, QMediaContent, QMediaPlaylist
from .playlistworker import DLocalMediaContent, DOnlineMediaContent
from log import logger



class PlayerBin(QMediaPlayer):

    def __init__(self):
        super(PlayerBin, self).__init__()
        self.setNotifyInterval(50)


class MediaPlayer(QObject):

    __contextName__ = "MediaPlayer"

    musicInfoChanged = pyqtSignal('QString', 'QString')

    positionChanged = pyqtSignal('qint64')
    volumeChanged = pyqtSignal(int)
    mutedChanged = pyqtSignal(bool)
    notifyIntervalChanged = pyqtSignal(int)
    playbackModeChanged = pyqtSignal(int)
    stateChanged = pyqtSignal(int)
    mediaStatusChanged = pyqtSignal('QMediaPlayer::MediaStatus')
    bufferStatusChanged = pyqtSignal(int)

    playlistChanged = pyqtSignal('QString')

    coverChanged = pyqtSignal('QString', 'QString','QString')

    @registerContext
    def __init__(self):
        super(MediaPlayer, self).__init__()

        self.player = PlayerBin()
        self._playlist = None

        self._state = 0
        self._isPlaying = False

        self.initPlayer()

        self.initConnect()

    def initPlayer(self):
        self.setNotifyInterval(50)

    def initConnect(self):
        self.player.mediaStatusChanged.connect(self.mediaStatusChanged)
        self.player.mediaStatusChanged.connect(self.monitorMediaStatus)
        self.player.positionChanged.connect(self.positionChanged)
        self.player.bufferStatusChanged.connect(self.bufferChange)


    @pyqtProperty('QVariant', notify=playlistChanged)
    def playlist(self):
        return self._playlist

    @pyqtSlot('QMediaPlaylist')
    def setPlaylist(self, playlist):
        if self._playlist:
            self._playlist.currentIndexChanged.disconnect(self.setCurrentMedia)

        self._playlist = playlist
        self._playlist.currentIndexChanged.connect(self.setCurrentMedia)
        self.playlistChanged.emit(playlist.name)

    @pyqtSlot('QString')
    def setPlaylistByName(self, name):
        playlistWorker = contexts['PlaylistWorker']
        configWorker = contexts['ConfigWorker']

        playbackMode = configWorker.playbackMode
        playlist = playlistWorker.getPlaylistByName(name)
        playlist.setPlaybackMode(playbackMode)
        self.setPlaylist(playlist)

    @pyqtSlot(int)
    def setPlaybackMode(self, playbackMode):
        configWorker = contexts['ConfigWorker']
        configWorker.playbackMode = playbackMode
        self.playlist.setPlaybackMode(playbackMode)
        self.playbackModeChanged.emit(playbackMode)

    @pyqtProperty(int)
    def postion(self):
        return self.player.position()

    @pyqtSlot(int)
    def setPosition(self, pos):
        self.player.setPosition(pos)
        self.positionChanged.emit(pos)

    @pyqtProperty(int)
    def volume(self):
        return self.player.volume()

    @pyqtSlot(int)
    def setVolume(self, value):
        self.player.setVolume(value)
        self.volumeChanged.emit(value)

    @pyqtProperty(bool)
    def muted(self):
        return self.player.isMuted()

    @pyqtSlot(bool)
    def setMuted(self, muted):
        self.player.setMuted(muted)
        self.mutedChanged.emit(muted)

    @pyqtProperty(int)
    def notifyInterval(self):
        return self.player.notifyInterval()

    @pyqtSlot(int)
    def setNotifyInterval(self, interval):
        self.player.setNotifyInterval(interval)
        self.notifyIntervalChanged.emit(interval)

    @pyqtProperty(int)
    def duration(self):
        return self.player.duration()

    @pyqtProperty(bool)
    def seekable(self):
        return self.player.isSeekable()

    @pyqtProperty(str)
    def errorString(self):
        return self.player.errorString()


    def monitorMediaStatus(self, status):
        if status == 7:
            if self._playlist:
                if self._playlist.playbackMode() == 1:
                    self.playToggle(self._isPlaying)
                elif self._playlist.playbackMode() in [3, 4]:
                    self.next()

    @pyqtSlot(bool)
    def playToggle(self, playing):
        if playing:
            self.play()
        else:
            self.pause()

        self._isPlaying = playing

    @pyqtSlot()
    def stop(self):
        self.player.stop()
        self._state = 0
        self.stateChanged.emit(self._state)

    @pyqtSlot()
    def play(self):
        self.player.play()
        self._state = 1
        self.stateChanged.emit(self._state)

    @pyqtSlot()
    def pause(self):
        self.player.pause()
        self._state = 2
        self.stateChanged.emit(self._state)

    @pyqtProperty(int, notify=stateChanged)
    def state(self):
        return self._state

    @pyqtProperty('QString')
    def positionString(self):
        position = self.player.position()
        return duration_to_string(position)

    @pyqtProperty('QString')
    def durationString(self):
        duration = self.player.duration()
        return duration_to_string(duration)

    @pyqtSlot('QString', result='QString')
    def metaData(self, key):
        return self.player.metaData(key)

    def showMetaData(self):
        import json
        metaData = {}
        for key in self.availableMetaData():
            v = self.metaData(key)
            if isinstance(v, QDate):
                v = v.toString('yyyy.MM.dd')
            metaData.update({key: v})
        logger.info(metaData)
        path = os.sep.join(
            [os.path.dirname(os.getcwd()), 'music',
             '%s.json' % self.metaData('Title')])
        f = open(path, 'w')
        f.write(json.dumps(metaData, indent=4))
        f.close()

    @pyqtSlot('QString')
    def setMediaUrl(self, url):
        if url.startswith('http://') or url.startswith('https://'):
            _url = QUrl(url)
        else:
            _url = QUrl.fromLocalFile(url)

        self.player.setMedia(QMediaContent(_url))
        self.playToggle(self._isPlaying)

    @pyqtSlot()
    def previous(self):
        if self._playlist:
            self._playlist.previous()
            if self._playlist.playbackMode() == 1:
                count = self._playlist.mediaCount()
                currentIndex = self._playlist.currentIndex()
                if currentIndex == 0:
                    index = count - 1
                else:
                    index = currentIndex - 1
                self._playlist.setCurrentIndex(index)

    @pyqtSlot()
    def next(self):
        if self._playlist:
            self._playlist.next()
            if self._playlist.playbackMode() == 1:
                count = self._playlist.mediaCount()
                currentIndex = self._playlist.currentIndex()
                if currentIndex == count - 1:
                    index = 0
                else:
                    index = currentIndex + 1
                self._playlist.setCurrentIndex(index)

    @pyqtSlot(int)
    def setCurrentMedia(self, index):
        urls = self._playlist.urls
        mediaContents =  self._playlist.mediaContents
        
        mediaContent = mediaContents[urls[index]]
        if isinstance(mediaContent, DLocalMediaContent):
            url = mediaContent.url
            cover = ''
        elif isinstance(mediaContent, DOnlineMediaContent):
            url = mediaContent.playLinkUrl
            if 'albumImage_500x500' in mediaContent.song and mediaContent.song['albumImage_500x500']:
                cover = mediaContent.song['albumImage_500x500']
            elif 'albumImage_100x100' in mediaContent.song and mediaContent.song['albumImage_100x100']:
                cover = mediaContent.song['albumImage_100x100']
            else:
                cover = ''
        if url:
            self.setMediaUrl(url)
            title = mediaContent.song['title']
            artist = mediaContent.song['artist']

            self.musicInfoChanged.emit(title, artist)
            self.coverChanged.emit(cover, title, artist)

    def bufferChange(self, progress):
        self.bufferStatusChanged.emit(progress)

    @pyqtSlot('QVariant')
    def addOnlineMedia(self, result):
        self._playlist.addMedia(result['url'], result['ret'])
