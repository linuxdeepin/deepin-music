#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import copy
import json
import re
import time
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


class SongObject(QObject):

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
    progressChanged = pyqtSignal(float)

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)
        self.start_size = 0
        self.stopDownloaded = False
        self.isFinished = False
        self.filename = os.path.join(DownloadSongPath, '%s-%s.%s'%(self.singerName, self.name, self.ext))

        self.speedTimer = QTimer()
        self.speedTimer.setInterval(100)
        self.speedTimer.timeout.connect(self.calSpeed)

        self.initConnect()

    def initConnect(self):
        self.progressChanged.connect(self.setPorgress)
        self.downloadFinished.connect(self.setFinished)

    def setPorgress(self, progress):
        self.progress = progress
        print self.progress

    def setFinished(self, finished):
        self.isFinished = finished

    def startDownLoad(self):
        if os.path.exists(self.filename):
            with open(self.filename, 'r') as f:
                self.start_size = self.getCurrentSize()
        d = DownLoadRunnable(self)
        QThreadPool.globalInstance().start(d)
        # self.speedTimer.start()

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
            print r.headers
            crange = r.headers['content-range']
            self.total = int(re.match(ur'^bytes 0-4/(\d+)$', crange).group(1))
            return True
        except:
            raise
            pass
        try:
            self.total = int(r.headers['content-length'])
        except:
            raise
            self.total = 0
        return False

    def download(self, url, filename, headers={}):
        print self.url
        print filename
        self.songObj.isFinished = False
        block = self.block
        local_filename = self.remove_nonchars(filename)

        isSupportContinued = self.support_continue(url)
        print(isSupportContinued)
        if isSupportContinued:
            try:
                if os.path.exists(local_filename):
                    with open(local_filename, 'rb') as fin:
                        self.size = len(fin.read())
                else:
                    self.touch(local_filename)
                    self.size = 0
            except:
                import traceback
                print traceback.print_exc()
            finally:
                headers['Range'] = "bytes=%d-" % (self.size, )

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
                            self.songObj.progressChanged.emit(self.progress)
                        else:
                            break
                    self.songObj.downloadFinished.emit(True)
                except:
                    import traceback
                    print traceback.print_exc()
        else:
            self.size = 0
            total = self.total
            size = self.size
            r = requests.get(url, stream=True)
            with open(filename, 'w') as f:
                f.write(r.content)
            self.songObj.downloadFinished.emit(True)


class DownloadSongWorker(QObject):

    __contextName__ = "DownloadSongWorker"

    _songsDict = LevelJsonDict(os.path.join(LevevDBPath, 'downloadSong'))
    _songObjs = OrderedDict()

    def __init__(self, parent=None):
        super(DownloadSongWorker, self).__init__(parent)

    def downloadSong(self, songDict):
        songId = songDict['songId']
        songObj = SongObject(**songDict)
        self._songObjs[songId] = songObj
        self._songsDict[songId] = songDict

        songObj.startDownLoad()



if __name__ == '__main__':
    from PyQt5.QtGui import QGuiApplication
    app = QGuiApplication(sys.argv)
    url = "http://360.media.duomi.com/dm//duomial/L201YV83NS8wNi8wNC8zNzg1MTY0Ml82OTY3LTEyMzAwNDAx.m4a?type=0&pos=1&uid="

    song = Song('liudehua', 'ai ni yi wan nian', url)
    downloadWorker = DownloadSongWorker()
    song.startDownLoad()
    exitCode = app.exec_()
    sys.exit(exitCode)
