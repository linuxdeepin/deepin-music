#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty

from .utils import registerContext
from dwidgets.tornadotemplate import template


class MusicManageWorker(QObject):

    playUrl = pyqtSignal(unicode)

    __contextName__ = 'MusicManageWorker'

    @registerContext
    def __init__(self, parent=None):
        super(MusicManageWorker, self).__init__(parent)
        self.loader = template.Loader(os.sep.join([os.getcwd(), 'views', 'html']))
        self.getArtistHtml()
        self.getAlbumHtml()
        self.getMusicHtml()
        self.getfolderHtml()

    @pyqtProperty('QString')
    def artistHtml(self):
        albums = []
        for i in range(1, 101):
            if i % 10 == 0:
                index = 1
            else:
                index = i % 10
            albums.append({
                'cover': os.sep.join([os.getcwd(), 'views', 'html', 'img', '%d.jpg' % (index, )]),
                'artist': i,
                'number': i
            })
        ouput = self.loader.load('template_musicmanage_artist.html').generate(albums=albums)

        with open(self.artistUrl, 'w') as f:
            f.write(ouput)
        return ouput

    @pyqtProperty('QString')
    def albumHtml(self):
        albums = []
        for i in range(1, 51):
            if i % 10 == 0:
                index = 1
            else:
                index = i % 10
            albums.append({
                'cover': os.sep.join([os.getcwd(), 'views', 'html', 'img', '%d.jpg' % (index, )]),
                'artist': i,
                'number': i
            })
        ouput = self.loader.load('template_musicmanage_album.html').generate(albums=albums)

        with open(self.albumUrl, 'w') as f:
            f.write(ouput)
        return ouput

    @pyqtProperty('QString')
    def musicHtml(self):
        albums = []
        for i in range(1, 31):
            if i % 10 == 0:
                index = 1
            else:
                index = i % 10
            albums.append({
                'cover': os.sep.join([os.getcwd(), 'views', 'html', 'img', '%d.jpg' % (index, )]),
                'artist': i,
                'number': i
            })
        ouput = self.loader.load('template_musicmanage_music.html').generate(albums=albums)

        with open(self.musicUrl, 'w') as f:
            f.write(ouput)
        return ouput

    @pyqtProperty('QString')
    def folderHtml(self):
        albums = []
        for i in range(1, 11):
            if i % 10 == 0:
                index = 1
            else:
                index = i % 10
            albums.append({
                'cover': os.sep.join([os.getcwd(), 'views', 'html', 'img', '%d.jpg' % (index, )]),
                'artist': i,
                'number': i
            })
        ouput = self.loader.load('template_musicmanage_folder.html').generate(albums=albums)

        with open(self.folderUrl, 'w') as f:
            f.write(ouput)
        return ouput

    def getArtistHtml(self):
        return self.artistHtml

    def getAlbumHtml(self):
        return self.albumHtml

    def getMusicHtml(self):
        return self.musicHtml

    def getfolderHtml(self):
        return self.folderHtml

    @pyqtProperty('QString')
    def baseURL(self):
        return os.sep.join([os.getcwd(), 'views', 'html'])

    @pyqtProperty('QString')
    def artistUrl(self):
        return os.sep.join([os.getcwd(), 'views', 'html', 'musicmanage_artist.html'])

    @pyqtProperty('QString')
    def albumUrl(self):
        return os.sep.join([os.getcwd(), 'views', 'html', 'musicmanage_album.html'])

    @pyqtProperty('QString')
    def musicUrl(self):
        return os.sep.join([os.getcwd(), 'views', 'html', 'musicmanage_music.html'])

    @pyqtProperty('QString')
    def folderUrl(self):
        return os.sep.join([os.getcwd(), 'views', 'html', 'musicmanage_folder.html'])
