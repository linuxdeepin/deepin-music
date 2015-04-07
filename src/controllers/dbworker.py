#!/usr/bin/python
# -*- coding: utf-8 -*-
# from __future__ import unicode_literals

import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QDate, QDir, QTimer)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from models import *
from log import logger
from dwidgets import dthread
import threading
import copy


class DBWorker(QObject):

    def __init__(self):
        super(DBWorker, self).__init__()
        db.connect()
        db.create_tables([Song, Artist, Album, Folder, Playlist, SongPlaylist], safe=True)
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

    @dthread
    def addSongs(self, songs):
        songs = copy.deepcopy(songs)
        artists = []
        albums = []
        folders = []
        for song in songs:
            artists.append({'name': song['artist']})
            albums.append({'name': song['album'], 'artist': song['artist']})
            folders.append({'name': song['folder']})

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
