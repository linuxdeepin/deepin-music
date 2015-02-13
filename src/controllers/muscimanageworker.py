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

    @pyqtProperty('QString')
    def musicManageHtml(self):
        loader = template.Loader(os.sep.join([os.getcwd(), 'views', 'html']))
        albums = []
        for i in range(1, 11):
            albums.append({
                'cover': 'img/%d.jpg' % i,
                'artist': i,
                'number': i
            })
        ouput = loader.load('musicmanage.html').generate(albums=albums)
        return ouput
