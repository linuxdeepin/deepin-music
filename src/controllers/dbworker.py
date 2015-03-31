#!/usr/bin/python
# -*- coding: utf-8 -*-
# from __future__ import unicode_literals

import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QDate, QDir)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from models import *
from log import logger

class DBWorker(QObject):

    def __init__(self):
        super(DBWorker, self).__init__()
        db.connect()
        db.create_tables([Song, Singer, Album, Folder, Playlist, SongPlaylist], safe=True)
        # self.initDB()
        print self.getSongCount()
        self.initDB()

    def initDB(self):
        # print sys.setdefaultencoding('utf-8')
        print sys.getfilesystemencoding()
        print "怎样才能使它工作正常呢?"
        basePath =  '/usr/share/deepin-sample-music/'
        d = QDir(basePath)
        filters = ["*.wav", "*.wma", "*.mp2", "*.mp3", "*.mp4", "*.m4a", "*.flac", "*.ogg"]
        d.setNameFilters(filters)
        # d.setFilter(QDir.Files)
        print d.entryList()

    def getSongCount(self):
        return Song.select().count()

    # def initDB(self):
    #     basePath = '/home/djf/workspace/github/musicplayer-qml/music'
    #     url = os.path.join(basePath, '1.mp3')
    #     song = Song.createLocalInstanceByUrl(url)
    #     print(song.pprint())
