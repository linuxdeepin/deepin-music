#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import json
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from config.constants import ProjectPath
from dwidgets import ModelMetaclass


class ConfigWorker(object):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('lastPlaylistName', 'QString', 'temporary'),
        ('lastPlayedIndex', int, 0),
        ('playbackMode', int, 4),
        ('volume', int, 50),
        ('isCoverBackground', bool, True),

        ('createItems', list),
        ('destoryItems', list)
    )

    __contextName__ = "ConfigWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        self.load()

    def save(self):
        mediaPlayer = contexts['MediaPlayer']
        if mediaPlayer._playlist:
            self.lastPlaylistName = mediaPlayer._playlist.name
            self.lastPlayedIndex = mediaPlayer._playlist.currentIndex()
        self.volume = mediaPlayer.volume
        self.playbackMode = mediaPlayer.playbackMode

        ret = self.getDict()
        with open(os.path.join(ProjectPath, 'config.json'), 'wb') as f:
            json.dump(ret, f, indent=4)

    def load(self):
        with open(os.path.join(ProjectPath, 'config.json'), 'r') as f:
            ret = json.load(f)
        self.setDict(ret)
