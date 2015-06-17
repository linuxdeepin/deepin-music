#!/usr/bin/python
# -*- coding: utf-8 -*-
# from __future__ import unicode_literals

import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QDate, QDir, QTimer, QRunnable, QThreadPool)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from models import *
from log import logger
from dwidgets import dthread
import threading
import copy
import datetime


class DBRunnable(QRunnable):

    def __init__(self, worker, songs):
        super(DBRunnable, self).__init__()
        self.worker = worker
        self.songs = songs

    def run(self):
        self.worker.saveSongs(self.songs)


class RestoreDBRunnable(QRunnable):

    def __init__(self, worker, songs):
        super(RestoreDBRunnable, self).__init__()
        self.worker = worker
        self.songs = songs

    def run(self):
        self.worker.saveSongs(self.songs)
        self.worker.restoreSongsSuccessed.emit()

class DBWorker(QObject):

    restoreSongsSuccessed = pyqtSignal()

    def __init__(self):
        super(DBWorker, self).__init__()
        db.connect()
        db.create_tables([Song, Artist, Album, Folder, OnlineSong], safe=True)
        self._count = 0
        # self.loadDB()

    def loadDB(self):
        for song in Song.select():
            print song.pprint()

        print Song.select().count()

    def addSong(self, songDict):
        self._count += 1
        def writeToDB():
            try:
                artistDict = {'name': songDict['artist']}
                albumDict = {'name': songDict['album']}
                folderDict = {'name': songDict['folder']}

                fartist = Artist.get_create_Record(**artistDict)
                falbum = Album.get_create_Record(**albumDict)
                ffolder = Folder.get_create_Record(**folderDict)

                songDict['fartist'] = fartist
                songDict['falbum'] = falbum
                songDict['ffolder'] = ffolder
                song = Song.get_create_Record(**songDict)
                print(song.id, threading.currentThread())
            except Exception, e:
                logger.error(songDict.pprint())
                logger.error(e)
        QTimer.singleShot(100 * self._count, writeToDB)

    def addSongs(self, songs):
        d = DBRunnable(self, songs)
        QThreadPool.globalInstance().start(d)

    def restoreSongs(self, songs):
        d = RestoreDBRunnable(self, songs)
        QThreadPool.globalInstance().start(d)

    @classmethod
    def saveSongs(self, songs):
        # created_date = datetime.datetime.now()
        # for song in songs:
        #     if 'created_date' not in songs:
        #         song['created_date'] = created_date
        songs = copy.deepcopy(songs)
        artists = []
        albums = []
        folders = []

        for song in songs:
            artists.append({
                'name': song['artist'],
                # 'created_date': song['created_date']
                })
            albums.append({
                'name': song['album'], 
                'artist': song['artist'],
                # 'created_date': song['created_date']
                })
            folders.append({
                'name': song['folder'],
                # 'created_date': song['created_date']
            })

        Artist.get_create_Records(artists)
        Album.get_create_Records(albums)
        Folder.get_create_Records(folders)

        for song in songs:
            artistDict = {'name': song['artist']}
            albumDict = {'name': song['album']}
            folderDict = {'name': song['folder']}
            fartist = Artist.getRecord(**artistDict)
            falbum = Album.getRecord(**albumDict)
            ffolder = Folder.getRecord(**folderDict)
            song['fartist'] = fartist
            song['falbum'] = falbum
            song['ffolder'] = ffolder
        Song.get_create_Records(songs)


dbWorker = DBWorker()
