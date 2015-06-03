#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import json
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from config.constants import ProjectPath, DownloadSongPath
from dwidgets import ModelMetaclass


class ConfigWorker(object):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('lastPlaylistName', 'QString', 'temporary'),
        ('lastPlayedIndex', int, 0),
        ('playbackMode', int, 4),
        ('volume', int, 50),
        # settings

        ('isAutoPlay', bool, False),
        ('isDesktopLrcShow', bool, False),
        ('isContinueLastPlayProgress', bool, False),
        ('isCoverBackground', bool, False),
        ('isFade', bool, False),
        ('isExitedWhenClosed', int, 0), # 0: minimized 1:exited
        ('addSongToPlaylistPlayMode', int, 0), # 0: play first; 1:play next ; 2:play last:

        ('isShortcutEnable', bool, True),
        ('shortcut_preivous', 'QString', 'Left'),
        ('shortcut_next', 'QString', 'Right'),
        ('shortcut_volumnIncrease', 'QString', 'Up'),
        ('shortcut_volumeDecrease', 'QString', 'Down'),
        ('shortcut_playPause', 'QString', 'Space'),
        ('shortcut_simpleFullMode', 'QString', 'F11'),
        ('shortcut_miniFullMode', 'QString', 'F10'),
        ('shortcut_hideShowWindow', 'QString', 'F5'),
        ('shortcut_hideShowDesktopLRC', 'QString', 'F6'),

        ('desktopLRC_fontType', int, 0),
        ('fontType', list, ['A', 'B']),

        ('desktopLRC_fontSize', int, 30),
        ('desktopLRC_fontSize_minValue', int, 10),
        ('desktopLRC_fontSize_maxValue', int, 100),

        ('desktopLRC_fontItalic', int, 0),
        ('fontItalic', list, ['C', 'D']),

        ('desktopLRC_lineNumber', int, 1),
        ('lineNumber', list, ['1', '2']),

        ('desktopLRC_fontAlignment', int, 0),
        ('fontAlignment', list, ['F', 'G']),

        ('desktopLRC_background1', int, 0),
        ('background1', list, ['green', 'red']),

        ('desktopLRC_background2', int, 0),
        ('background2', list, ['yellow', 'gray']),

        ('desktopLRC_backgroundSize', int, 30),
        ('desktopLRC_backgroundSize_minValue', int, 10),
        ('desktopLRC_backgroundSize_maxValue', int, 100),

        ('DownloadSongPath', 'QString', DownloadSongPath)
    )

    __contextName__ = "ConfigWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        self.load()

    def save(self):
        mediaPlayer = contexts['MediaPlayer']
        if mediaPlayer._playlist:
            self.lastPlaylistName = mediaPlayer._playlist.name
            self.lastPlayedIndex = mediaPlayer.currentIndex
        self.volume = mediaPlayer.volume
        self.playbackMode = mediaPlayer.playbackMode

        ret = self.getDict()
        with open(os.path.join(ProjectPath, 'config.json'), 'wb') as f:
            json.dump(ret, f, indent=4)

    def load(self):
        filePath = os.path.join(ProjectPath, 'config.json')
        if os.path.exists(filePath):
            with open(os.path.join(ProjectPath, 'config.json'), 'r') as f:
                ret = json.load(f)
            self.setDict(ret)


configWorker = ConfigWorker()
