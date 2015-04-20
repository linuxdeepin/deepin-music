#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import copy
import json
import re
import time
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QThreadPool, QRunnable
from PyQt5.QtGui import QImage
import requests
# from log import logger
# from .utils import registerContext
# from dwidgets import dthread, CoverRunnable
# from config.constants import ArtistCoverPath, AlbumCoverPath, SongCoverPath, OnlineSongCoverPath
# import urllib


class Song(QObject):

    """docstring for Song"""

    def __init__(self, artist, title, url):
        super(Song, self).__init__()
        self.artist = artist
        self.title = title
        self.url = url
        self.filename = u'%s-%s' % (artist, title)


class DownLoadRunnable(QRunnable):

    def __init__(self, songObj):
        super(DownLoadRunnable, self).__init__()
        self.block = 1024
        self.total = 0
        self.size = 0
        self.filename = songObj.filename
        self.url = songObj.url

    def run(self):
        self.download(self.url, self.filename)

    def touch(self, filename):
        with open(filename, 'w') as fin:
            pass

    def remove_nonchars(self, name):
        (name, _) = re.subn(ur'[\\\/\:\*\?\"\<\>\|]', '', name)
        return name

    def support_continue(self, url):
        headers = {
            'Range': 'bytes=0-4'
        }
        try:
            r = requests.head(url, headers=headers)
            crange = r.headers['content-range']
            self.total = int(re.match(ur'^bytes 0-4/(\d+)$', crange).group(1))
            return True
        except:
            pass
        try:
            self.total = int(r.headers['content-length'])

        except:
            self.total = 0
        return False

    def download(self, url, filename, headers={}):
        finished = False
        block = self.block
        local_filename = self.remove_nonchars(filename)
        size = self.size
        if self.support_continue(url):  # 支持断点续传
            try:
                with open(local_filename, 'rb') as fin:
                    self.size = len(fin.read())
                    self.size = self.size + 1
                    print self.size, self.total
            except:
                import traceback
                print traceback.print_exc()
            finally:
                headers['Range'] = "bytes=%d-" % (self.size, )
        else:
            # self.touch(tmp_filename)
            self.touch(local_filename)

        total = self.total
        r = requests.get(url, stream=True, verify=False, headers=headers)
        start_t = time.time()
        with open(local_filename, 'ab+') as f:
            f.seek(self.size)
            f.truncate()
            try:
                for chunk in r.iter_content(chunk_size=block):
                    if chunk:
                        f.write(chunk)
                        size += len(chunk)
                        f.flush()
                    sys.stdout.write(
                        '\b' * 64 + 'Now: %d, Total: %s' % (size, total))
                    sys.stdout.flush()
                finished = True
                spend = int(time.time() - start_t)
                if spend > 0:
                    speed = int((size - self.size) / 1024 / spend)
                    sys.stdout.write(
                        '\nDownload Finished!\nTotal Time: %ss, Download Speed: %sk/s\n' % (spend, speed))
                    sys.stdout.flush()
            except:
                import traceback
                print traceback.print_exc()
                print "\nDownload pause.\n"


class DownloadWorker(QObject):

    __contextName__ = "DownloadWorker"

    def __init__(self, parent=None):
        super(DownloadWorker, self).__init__(parent)


if __name__ == '__main__':
    from PyQt5.QtGui import QGuiApplication
    app = QGuiApplication(sys.argv)
    url = 'http://sv51.yunpan.cn//Download.outputAudio//1900039346//5bdbd706ed0340a98c7d6bb276b13905bcd232b4//51_51.180c19bd67ba7e809caefd53736fe959//1.0//openapi//14295493432814//0//d900fdcc7e7c510e2e91fd1b52ae263b//%E6%81%AD%E5%96%9C%E5%8F%91%E8%B4%A2_71363.mp3'
    song = Song('liudehua', 'ai ni yi wan nian', url)
    downloadWorker = DownloadWorker()
    # downloadWorker.
    d = DownLoadRunnable(song)
    QThreadPool.globalInstance().start(d)
    exitCode = app.exec_()
    sys.exit(exitCode)
