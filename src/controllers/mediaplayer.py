#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import time
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QDate)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from PyQt5.QtMultimedia import QMediaPlayer, QMediaContent, QMediaPlaylist
from .coverworker import CoverWorker
from log import logger
from .muscimanageworker import MusicManageWorker
from .onlinemuscimanageworker import OnlineMusicManageWorker
from .web360apiworker import Web360ApiWorker
from .signalmanager import signalManager


class PlayerBin(QMediaPlayer):

    def __init__(self):
        super(PlayerBin, self).__init__()
        self.setNotifyInterval(50)

gPlayer = PlayerBin()


class MediaPlayer(QObject):

    __contextName__ = "MediaPlayer"

    played = pyqtSignal()
    stoped = pyqtSignal()
    paused = pyqtSignal()

    musicInfoChanged = pyqtSignal('QString', 'QString')

    playingChanged = pyqtSignal(bool)

    positionChanged = pyqtSignal('qint64')
    volumeChanged = pyqtSignal(int)
    mutedChanged = pyqtSignal(bool)
    notifyIntervalChanged = pyqtSignal(int)
    playbackModeChanged = pyqtSignal(int)
    stateChanged = pyqtSignal(int)
    mediaStatusChanged = pyqtSignal('QMediaPlayer::MediaStatus')
    bufferStatusChanged = pyqtSignal(int)

    playlistChanged = pyqtSignal('QString')
    urlChanged = pyqtSignal('QString')
    songIdChanged = pyqtSignal('QVariant')

    currentIndexChanged = pyqtSignal(int)

    titleChanged = pyqtSignal('QString')
    artistChanged = pyqtSignal('QString')
    albumChanged = pyqtSignal('QString')
    coverChanged = pyqtSignal('QString')
    songObjChanged = pyqtSignal('QVariant')

    downloadCover = pyqtSignal('QString', 'QString')

    requestMusic = pyqtSignal('QString')

    @registerContext
    def __init__(self):
        super(MediaPlayer, self).__init__()
        self._playlist = None

        self._state = 0
        self._isPlaying = False


        self._playbackMode = 4
        self._volume = 0

        self._songId = 0
        self._url = ''
        self._title = ''
        self._artist = ''
        self._album = ''
        self._cover = ''

        self._songObj = None

        self._isLyricUpdated = True

        self.initPlayer()

        self.initConnect()

    def initPlayer(self):
        self.notifyInterval = 50

    def initConnect(self):

        self.played.connect(gPlayer.play)
        self.stoped.connect(gPlayer.stop)
        self.paused.connect(gPlayer.pause)

        gPlayer.mediaStatusChanged.connect(self.mediaStatusChanged)
        gPlayer.mediaStatusChanged.connect(self.monitorMediaStatus)
        gPlayer.positionChanged.connect(self.positionChanged)
        gPlayer.durationChanged.connect(self.updateDuration)
        gPlayer.bufferStatusChanged.connect(self.bufferChange)
        gPlayer.error.connect(self.monitorError)

        self.playingChanged.connect(signalManager.playingChanged)
        self.positionChanged.connect(self.updateLrc)

        signalManager.previousSong.connect(self.previous)
        signalManager.playToggle.connect(self.playToggle)
        signalManager.nextSong.connect(self.next)
        signalManager.volumnIncrease.connect(self.actionVolumeIncrease)
        signalManager.volumnDecrease.connect(self.actionVolumeDecrease)
        signalManager.playbackModeChanged.connect(self.setPlaybackMode)

        signalManager.noLrcFound.connect(self.stopUpdateLyric)
        signalManager.updateLrc.connect(self.startUpdateLyric)

        signalManager.playMusicByLocalUrl.connect(self.playLocalMedia)

    def updateLrc(self, pos):
        from app.deepinplayer import DeepinPlayer
        if DeepinPlayer.instance().lrcWindowManager.isVisible and self._isLyricUpdated:
            signalManager.lrcPositionChanged.emit(pos, self.duration)

    def stopUpdateLyric(self):
        self._isLyricUpdated = False

    def startUpdateLyric(self):
         self._isLyricUpdated = True

    @pyqtProperty('QString', notify=urlChanged)
    def url(self):
        return self._url

    @url.setter
    def url(self, value):
        self._url = value
        self.urlChanged.emit(value)

    @pyqtProperty('QVariant', notify=songIdChanged)
    def songId(self):
        return self._songId

    @songId.setter
    def songId(self, value):
        self._songId = value
        self.songIdChanged.emit(value)

    @pyqtProperty('QVariant', notify=playlistChanged)
    def playlist(self):
        return self._playlist

    @pyqtSlot('QVariant')
    def setPlaylist(self, playlist):
        if self._playlist:
            self._playlist.currentIndexChanged.disconnect(self.currentIndexChanged)

        self._playlist = playlist
        self._playlist.currentIndexChanged.connect(self.currentIndexChanged)
        self.playlistChanged.emit(playlist.name)

    @pyqtSlot('QString')
    def setPlaylistByName(self, name):
        playlistWorker = contexts['PlaylistWorker']
        playbackMode = self._playbackMode
        playlist = playlistWorker.getPlaylistByName(name)
        if playlist:
            if self._playlist and self._playlist.name == playlist.name:
                return
            playlist.setPlaybackMode(playbackMode)
            self.setPlaylist(playlist)
            self.setCurrentIndex(0)

    @pyqtProperty(int, notify=playbackModeChanged)
    def playbackMode(self):
        return self._playbackMode

    @playbackMode.setter
    def playbackMode(self, playbackMode):
        self._playbackMode = playbackMode
        if self._playlist:
            self._playlist.setPlaybackMode(playbackMode)
        self.playbackModeChanged.emit(playbackMode)

    def setPlaybackMode(self, value):
        self.playbackMode = value

    @pyqtProperty(bool, notify=playingChanged)
    def playing(self):
        return self._isPlaying

    @pyqtProperty(int)
    def position(self):
        return gPlayer.position()

    @position.setter
    def position(self, pos):
        gPlayer.setPosition(pos)
        self.positionChanged.emit(pos)

    @pyqtProperty(int, notify=volumeChanged)
    def volume(self):
        return self._volume

    @volume.setter
    def volume(self, value):
        self._volume = value
        gPlayer.setVolume(value)
        self.volumeChanged.emit(value)

    def setVolume(self, value):
        self.volume = value

    def actionVolumeIncrease(self):
        if self.volume < 100:
            self.volume += 1

    def actionVolumeDecrease(self):
        if self.volume > 0:
            self.volume -= 1

    @pyqtProperty(bool, notify=mutedChanged)
    def muted(self):
        return gPlayer.isMuted()

    @muted.setter
    def muted(self, muted):
        gPlayer.setMuted(muted)
        self.mutedChanged.emit(muted)

    @pyqtProperty(int)
    def notifyInterval(self):
        return gPlayer.notifyInterval()

    @notifyInterval.setter
    def notifyInterval(self, interval):
        gPlayer.setNotifyInterval(interval)
        self.notifyIntervalChanged.emit(interval)

    @pyqtProperty(int)
    def duration(self):
        return gPlayer.duration()

    @pyqtSlot(int)
    def updateDuration(self, duration):
        try:
            index = self._playlist.currentIndex()
            urls = self._playlist.urls
            if index < len(urls):
                url = urls[index]

        except Exception, e:
            raise e

    @pyqtProperty('QString')
    def positionString(self):
        position = gPlayer.position()
        return duration_to_string(position)

    @pyqtProperty('QString')
    def durationString(self):
        duration = gPlayer.duration()
        if duration <= 0:
            index = self._playlist.currentIndex()
            urls = self._playlist.urls
            # if index < len(urls):
            #     mediaContent = mediaContents[urls[index]]
            #     if 'duration' in mediaContent.tags:
            #         duration = mediaContent.tags['duration']
        return duration_to_string(duration)

    @pyqtProperty(bool)
    def seekable(self):
        return gPlayer.isSeekable()

    @pyqtProperty(str)
    def errorString(self):
        return gPlayer.errorString()

    def monitorMediaStatus(self, status):
        if status == 7:
            if self._playlist:
                if self._playlist.playbackMode() == 1:
                    self.playToggle(self._isPlaying)
                elif self._playlist.playbackMode() in [3, 4]:
                    self.next()
        elif status == 4:
            self.buffingBeginTime = time.time()
            self.stop()
            self.play()
        elif status in [3, 6]:
            self.playToggle(self._isPlaying)

    def monitorError(self, error):
        errors = {
            0: "No error has occurred.",
            1: "A media resource couldn't be resolved",
            2: "The format of a media resource isn't (fully) supported. Playback may still be possible, but without an audio or video component",
            3: "A network error occurred",
            4: "There are not the appropriate permissions to play a media resource",
            5: "A valid playback service was not found, playback cannot proceed."
        }
        print(error, errors[error])
        if error == 3:
            url = self.getUrlID()
            if url:
                self.requestMusic.emit(url)

    @pyqtSlot(bool)
    def playToggle(self, playing):
        if playing:
            self.play()
        else:
            self.pause()

        self._isPlaying = playing

        self.playingChanged.emit(self._isPlaying)

    @pyqtSlot()
    def stop(self):
        self.stoped.emit()
        self.state = 0

    @pyqtSlot()
    def play(self):
        self.played.emit()
        self.state = 1

    @pyqtSlot()
    def pause(self):
        self.paused.emit()
        self.state = 2

    @pyqtProperty(int, notify=stateChanged)
    def state(self):
        return self._state

    @state.setter
    def state(self, value):
        self._state = value
        self.stateChanged.emit(value)

    @pyqtSlot('QString')
    def setMediaUrl(self, url):
        if url.startswith('http'):
            _url = QUrl(url)
        else:
            _url = QUrl.fromLocalFile(url)

        gPlayer.setMedia(QMediaContent(_url))
        self.playToggle(self._isPlaying)

    @pyqtSlot()
    def previous(self):
        if self._playlist:
            self._playlist.previous()
            currentIndex = self._playlist.currentIndex()
            if self._playlist.playbackMode() == 1:
                count = self._playlist.mediaCount()
                if currentIndex == 0:
                    index = count - 1
                else:
                    index = currentIndex - 1
                self._playlist.setCurrentIndex(index)
            currentIndex = self._playlist.currentIndex()
            self.playMediaByIndex(currentIndex)

    @pyqtSlot()
    def next(self):
        if self._playlist:
            self._playlist.next()
            currentIndex = self._playlist.currentIndex()
            if self._playlist.playbackMode() == 1:
                count = self._playlist.mediaCount()
                
                if currentIndex == count - 1:
                    index = 0
                else:
                    index = currentIndex + 1
                self._playlist.setCurrentIndex(index)
            currentIndex = self._playlist.currentIndex()
            self.playMediaByIndex(currentIndex)

    @pyqtSlot(int)
    def setCurrentIndex(self, index):
        if index < 0:
            index = 0
        if self._playlist:
            if index < self._playlist.mediaCount():
                self._playlist.setCurrentIndex(index)
                self.playMediaByIndex(index)

    @pyqtSlot(int)
    def playMediaByIndex(self, index):
        urls = self._playlist.urls
        if index < len(urls) and index >= 0:
            url = urls[index]
            if url.startswith('http'):
                self.requestMusic.emit(url)
            else:
                self.playLocalMedia(url)

    def getUrlID(self):
        if self._playlist:
            currentIndex = self._playlist.currentIndex()
            urls = self._playlist.urls
            url = urls[currentIndex]
            return url
        else:
            return None

    def bufferChange(self, progress):
        self.bufferStatusChanged.emit(progress)

    @pyqtSlot('QString')
    def playLocalMedia(self, url):
        urls = self._playlist.urls
        if url in urls:
            index = urls.index(url)
            self._playlist.setCurrentIndex(index)

            self.updateMediaView(url)
            self.playMediaByUrl(url)

            self.url = url

    @pyqtSlot('QVariant')
    def playOnlineMedia(self, result):
        url = result['url']
        urls = self._playlist.urls
        if url in urls:
            index = urls.index(url)
            self._playlist.setCurrentIndex(index)
            if url in urls:
                self.updateMediaView(url)

            if 'playlinkUrl' in result:
                playlinkUrl = result['playlinkUrl']
                self.playMediaByUrl(playlinkUrl)

                self.url = url

    def updateMediaView(self, url):
        if url.startswith('http'):
            songObj = OnlineMusicManageWorker.getSongObjByUrl(url)
            if songObj:
                self.songId = songObj.songId
        else:
            songObj = MusicManageWorker.getSongObjByUrl(url)
        
        if self._songObj:
            self._songObj.coverReady.disconnect(self.setCover)
            self._songObj = None

        self._songObj = songObj
        self._songObj.coverReady.connect(self.setCover)
        self._songObj.getCover()

        if songObj:
            self.title = songObj.title
            self.artist = songObj.artist
            self.album = songObj.album
            signalManager.downloadLrc.emit(self.artist, self.title)

    def playMediaByUrl(self, url):
        self.setMediaUrl(url)
        self.playToggle(True)

    @pyqtSlot('QVariant')
    def swicthOnlineMedia(self, result):
        if 'url' in result:
            url = result['url']
            self.updateMediaView(url)
        if 'playlinkUrl' in result:
            playlinkUrl = result['playlinkUrl']
            self.playMediaByUrl(playlinkUrl)

    @pyqtProperty(int, notify=currentIndexChanged)
    def currentIndex(self):
        if self._playlist:
            return self._playlist.currentIndex()
        else:
            return -1

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

    @pyqtProperty('QString', notify=albumChanged)
    def album(self):
        return self._album

    @album.setter
    def album(self, value):
        self._album = value
        self.albumChanged.emit(value)

    @pyqtProperty('QString', notify=coverChanged)
    def cover(self):
        return self._cover

    @cover.setter
    def cover(self, value):
        self._cover = value
        self.coverChanged.emit(self._cover)

    def setCover(self, value):
        self.cover = value

    def getCover(self):
        if CoverWorker.isSongCoverExisted(self.artist, self.title):
            _cover = CoverWorker.getCoverPathByArtistSong(self.artist, self.title)
        elif CoverWorker.isOnlineSongCoverExisted(self.artist, self.title):
            _cover = CoverWorker.getOnlineCoverPathByArtistSong(self.artist, self.title)
        elif CoverWorker.isAlbumCoverExisted(self.artist, self.album):
            _cover = CoverWorker.getCoverPathByArtistAlbum(self.artist, self.album)
        else:
            _cover = CoverWorker.getCoverPathByArtist(self.artist)
        return _cover

    @pyqtSlot('QString', result='QString')
    def metaData(self, key):
        return gPlayer.metaData(key)

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


mediaPlayer = MediaPlayer()
