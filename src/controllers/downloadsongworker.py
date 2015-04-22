#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import copy
import json
import re
import time
import traceback
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                         QThreadPool, QRunnable, QTimer)
from PyQt5.QtGui import QImage
import requests
from log import logger
from .utils import registerContext
from dwidgets import dthread, LevelJsonDict
from collections import OrderedDict
from config.constants import LevevDBPath, DownloadSongPath
from dwidgets import DListModel, ModelMetaclass


class DownloadSongObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('url','QString'),
        ('ext', 'QString'),
        ('bitrate', 'QString'),
        ('hdDesc', 'QString'),
        ('size', 'QString'),
        ('songId', int),
        ('name', 'QString'),
        ('singerName', 'QString'),
        ('originalServiceEngName', 'QString'),
        ('serviceEngName', 'QString'),
        ('serviceName', 'QString'),
        ('downloadUrl', 'QString'),
        ('progress', int)
    )

    downloadFinished = pyqtSignal(bool)
    progressUpdated = pyqtSignal(float)

    deleteSelf = pyqtSignal(int)
    updateDBPoperty = pyqtSignal(int, float)

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)
        self.start_size = 0
        self.stopDownloaded = False
        self.isFinished = False
        self.filename = DownloadSongWorker.getSongPath(self.singerName, self.name, self.ext)
        print self.filename
        self.speedTimer = QTimer()
        self.speedTimer.setInterval(100)
        self.speedTimer.timeout.connect(self.calSpeed)
        self.initConnect()
        self.startDownLoad()

    def initConnect(self):
        self.progressUpdated.connect(self.updatePorgress)
        self.downloadFinished.connect(self.setFinished)

    def updatePorgress(self, progress):
        self.progress = progress
        self.updateDBPoperty.emit(self.songId, progress)

    def setFinished(self, finished):
        self.isFinished = finished
        self.deleteSelf.emit(self.songId)

    def startDownLoad(self):
        if os.path.exists(self.filename):
            with open(self.filename, 'r') as f:
                self.start_size = self.getCurrentSize()
        d = DownLoadRunnable(self)
        QThreadPool.globalInstance().start(d)

    def stopDownload(self):
        self.stopDownloaded = True

    def calSpeed(self):
        if self.isFinished:
            self.speedTimer.stop()
        currentSize = self.getCurrentSize()
        if currentSize is not None and self.start_size is not None:
            if currentSize > self.start_size:
                speed = (currentSize - self.start_size) / 1024
                self.start_size = currentSize
                print '\n', currentSize, self.start_size, '\n%s KB/s' % speed

    def getCurrentSize(self):
        if os.path.exists(self.filename):
            with open(self.filename, 'r') as f:
                size = len(f.read())
            return size
        else:
            return None


class DownloadSongListModel(DListModel):

    __contextName__ = 'DownloadSongListModel'

    @registerContext
    def __init__(self, dataTye):
        super(DownloadSongListModel, self).__init__(dataTye)


class DownLoadRunnable(QRunnable):

    def __init__(self, songObj):
        super(DownLoadRunnable, self).__init__()
        self.block = 1024
        self.total = 0
        self.size = 0
        self.songObj = songObj
        self.filename = songObj.filename
        self.url = songObj.downloadUrl

    def run(self):
        self.download(self.url, self.filename)

    def touch(self, filename):
        with open(filename, 'w') as fin:
            pass

    def remove_nonchars(self, name):
        (name, _) = re.subn(ur'[\\\/\:\*\?\"\<\>\|]', '', name)
        return name

    def support_continue(self, url):
        '''
            check support continue download or not
        '''
        headers = {
            'Range': 'bytes=0-4'
        }
        try:
            r = requests.head(url, headers=headers)
            crange = r.headers['content-range']
            self.total = int(re.match(ur'^bytes 0-4/(\d+)$', crange).group(1))
            return True
        except:
            logger.error(traceback.print_exc())
        try:
            self.total = int(r.headers['content-length'])
        except:
            logger.error(traceback.print_exc())
            self.total = 0
        return False

    def download(self, url, filename, headers={}):
        self.songObj.isFinished = False
        block = self.block
        local_filename = filename

        isSupportContinued = self.support_continue(url)
        if isSupportContinued:
            try:
                if os.path.exists(local_filename):
                    with open(local_filename, 'rb') as fin:
                        self.size = len(fin.read())
                else:
                    self.touch(local_filename)
                    self.size = 0
            except:
                logger.error(traceback.print_exc())
            finally:
                headers['Range'] = "bytes=%d-" % (self.size, )
        else:
            self.touch(local_filename)
            self.size = 0

        total = self.total
        size = self.size
        r = requests.get(url, stream=True, verify=False, headers=headers)
        start_t = time.time()
        with open(local_filename, 'ab+') as f:
            f.seek(len(f.read()))
            f.truncate()
            try:
                for chunk in r.iter_content(chunk_size=block):
                    if chunk and not self.songObj.stopDownloaded:
                        f.write(chunk)
                        size += len(chunk)
                        f.flush()
                        self.progress = (float(size) / float(total)) * 100
                        self.songObj.progressUpdated.emit(self.progress)
                    else:
                        break
                self.songObj.downloadFinished.emit(True)
            except:
                logger.error(traceback.print_exc())


class DownloadSongWorker(QObject):

    __contextName__ = "DownloadSongWorker"

    _songsDict = LevelJsonDict(os.path.join(LevevDBPath, 'downloadSong'))
    _songObjs = OrderedDict()

    _downloadSongListModel = DownloadSongListModel(DownloadSongObject)

    def __init__(self, parent=None):
        super(DownloadSongWorker, self).__init__(parent)

    def downloadSong(self, songDict):
        songId = songDict['songId']

        singerName = songDict['singerName']
        name = songDict['name']
        ext = songDict['ext']

        if self.isSongExisted(singerName, name, ext):
            logger.info(singerName, name, ext, 'exists')
            return

        songDict['progress'] = 0

        songObj = DownloadSongObject(**songDict)
        songObj.deleteSelf.connect(self.delSongObj)
        songObj.updateDBPoperty.connect(self.updateModel)

        self._songObjs[songId] = songObj
        self._songsDict[songId] = songDict
        self._downloadSongListModel.append(songObj)

    def delSongObj(self, songId):
        if songId in self._songObjs:
            songObj = self._songObjs[songId]
            songObj.deleteSelf.disconnect(self.delSongObj)
            songObj.updateDBPoperty.disconnect(self.updateModel)

        del self._songObjs[songId]
        del self._songsDict[songId]
        for index, songObj in  enumerate(self._downloadSongListModel.data):
            if songObj.songId == songId:
                self._downloadSongListModel.remove(index)

    def updateModel(self, songId, progress):
        if songId in self._songsDict:
            songDict = self._songsDict[songId]
            songDict['progress'] = progress
            self._songsDict[songId] = songDict
        for index, songObj in  enumerate(self._downloadSongListModel.data):
            if songObj.songId == songId:
                self._downloadSongListModel.setProperty(index, 'progress', progress)
                print songId, progress, 'update data for UI'

    @classmethod
    def getSongPath(cls, singerName, name, ext):
        return os.path.join(DownloadSongPath, '%s-%s.%s'%(singerName, name, ext))

    @classmethod
    def isSongExisted(cls, singerName, name, ext):
        return os.path.exists(cls.getSongPath(singerName, name, ext))


if __name__ == '__main__':
    from PyQt5.QtGui import QGuiApplication
    app = QGuiApplication(sys.argv)
    url = "http://360.media.duomi.com/dm//duomial/L201YV83NS8wNi8wNC8zNzg1MTY0Ml82OTY3LTEyMzAwNDAx.m4a?type=0&pos=1&uid="

    song = Song('liudehua', 'ai ni yi wan nian', url)
    downloadWorker = DownloadSongWorker()
    song.startDownLoad()
    exitCode = app.exec_()
    sys.exit(exitCode)
