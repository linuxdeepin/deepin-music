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
from .playlistworker import DRealLocalMediaContent, DRealOnlineMediaContent
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

    currentIndexChanged = pyqtSignal(int)


    titleChanged = pyqtSignal('QString')
    artistChanged = pyqtSignal('QString')
    coverChanged = pyqtSignal('QString')

    coverdownloaded = pyqtSignal('QVariant')

    @registerContext
    def __init__(self):
        super(MediaPlayer, self).__init__()

        self.player = PlayerBin()
        self._playlist = None

        self._state = 0
        self._isPlaying = False

        self._title = ''
        self._artist = ''
        self._cover = ''

        self.initPlayer()

        self.initConnect()

    def initPlayer(self):
        self.setNotifyInterval(50)

    def initConnect(self):
        self.player.mediaStatusChanged.connect(self.mediaStatusChanged)
        self.player.mediaStatusChanged.connect(self.monitorMediaStatus)
        self.player.positionChanged.connect(self.positionChanged)
        self.player.durationChanged.connect(self.durationChange)
        self.player.bufferStatusChanged.connect(self.bufferChange)
        self.player.error.connect(self.monitorError)

    @pyqtProperty('QVariant', notify=playlistChanged)
    def playlist(self):
        return self._playlist

    @pyqtSlot('QMediaPlaylist')
    def setPlaylist(self, playlist):
        if self._playlist:
            self._playlist.currentIndexChanged.disconnect(self.setCurrentMedia)
            self._playlist.currentIndexChanged.disconnect(self.currentIndexChanged)

        self._playlist = playlist
        self._playlist.currentIndexChanged.connect(self.setCurrentMedia)
        self._playlist.currentIndexChanged.connect(self.currentIndexChanged)
        self.playlistChanged.emit(playlist.name)

    @pyqtSlot('QString')
    def setPlaylistByName(self, name):
        playlistWorker = contexts['PlaylistWorker']
        configWorker = contexts['ConfigWorker']

        playbackMode = configWorker.playbackMode
        playlist = playlistWorker.getPlaylistByName(name)
        playlist.setPlaybackMode(playbackMode)
        self.setPlaylist(playlist)

    @pyqtProperty(int)
    def playbackMode(self):
        return self.playlist.playbackMode()

    @pyqtSlot(int)
    def setPlaybackMode(self, playbackMode):
        configWorker = contexts['ConfigWorker']
        configWorker.playbackMode = playbackMode
        self.playlist.setPlaybackMode(playbackMode)
        self.playbackModeChanged.emit(playbackMode)

    @pyqtProperty(bool)
    def playing(self):
        return self._isPlaying

    @pyqtProperty(int)
    def position(self):
        return self.player.position()

    @pyqtSlot(int)
    def setPosition(self, pos):
        self.player.setPosition(pos)
        self.positionChanged.emit(pos)

    @pyqtProperty(int, notify=volumeChanged)
    def volume(self):
        return self.player.volume()

    @volume.setter
    def volume(self, value):
        self.player.setVolume(value)
        self.volumeChanged.emit(value)

    @pyqtProperty(bool, notify=mutedChanged)
    def muted(self):
        return self.player.isMuted()

    @muted.setter
    def muted(self, muted):
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

    @pyqtSlot(int)
    def durationChange(self, duration):
        index = self._playlist.currentIndex()
        urls = self._playlist.urls
        mediaContents =  self._playlist.mediaContents
        if index < len(urls):
            mediaContent = mediaContents[urls[index]]
            mediaContent.duration =  duration_to_string(self.player.duration())

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

    def monitorError(self, error):
        errors = {
            0: "No error has occurred.",
            1: "A media resource couldn't be resolved",
            2: "The format of a media resource isn't (fully) supported. Playback may still be possible, but without an audio or video component",
            3: "A network error occurred",
            4: "There are not the appropriate permissions to play a media resource",
            5: "A valid playback service was not found, playback cannot proceed."
        }
        print(errors[error])

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
    def setCurrentIndex(self, index):
        self._playlist.setCurrentIndex(index)

    @pyqtSlot(int)
    def setCurrentMedia(self, index):
        urls = self._playlist.urls
        mediaContents =  self._playlist.mediaContents

        mediaContent = mediaContents[urls[index]]
        url = mediaContent.url
        if isinstance(mediaContent, DRealLocalMediaContent):
            playurl = mediaContent.url
        elif isinstance(mediaContent, DRealOnlineMediaContent):
            playurl = mediaContent.playlinkUrl

        if playurl:
            self.setMediaUrl(playurl)
            self.title = mediaContent.title
            self.artist = mediaContent.artist
            # self.coverdownloaded.emit(mediaContent)

    def bufferChange(self, progress):
        self.bufferStatusChanged.emit(progress)

    @pyqtSlot('QVariant')
    def playLocalMedia(self, url):
        self._playlist.addMedia(url)
        self.playToggle(True)

    @pyqtSlot('QVariant')
    def playOnlineMedia(self, result):
        self._playlist.addMedia(result['url'], result['tags'], result['updated'])
        self.playToggle(True)

    @pyqtProperty(int, notify=currentIndexChanged)
    def currentIndex(self):
        return self._playlist.currentIndex()

    @pyqtProperty('QString', notify=titleChanged)
    def title(self):
        return self._title

    @title.setter
    def title(self, value):
        self._title = value
        self.titleChanged.emit(value)

    @pyqtProperty('QString', notify=artistChanged)
    def artist(self):
        return self._artist

    @artist.setter
    def artist(self, value):
        self._artist = value
        self.artistChanged.emit(value)

    @pyqtProperty('QString', notify=coverChanged)
    def cover(self):
        return self._cover

    @cover.setter
    def cover(self, value):
        self._cover = value
        self.coverChanged.emit(value)
