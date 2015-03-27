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


class I18nWorker(object):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('song', 'QString', '歌曲'),
        ('album', 'QString', '专辑'),
        ('artist', 'QString', '艺术家'),
        ('folder', 'QString', '文件夹'),
        ('duration', 'QString', '曲长'),
        ('temporary', 'QString', '试听歌单'),
        ('favorite', 'QString', '我的收藏'),
        ('myPlaylist', 'QString', '我创建的歌单'),
        ('newPlaylist', 'QString', '新建歌单'),
    )

    __contextName__ = "I18nWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        pass
