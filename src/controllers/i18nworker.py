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
        ('song', 'QString', u'歌曲'),
        ('album', 'QString', u'专辑'),
        ('artist', 'QString', u'艺术家'),
        ('folder', 'QString', u'文件夹'),
        ('duration', 'QString', u'曲长'),
        ('size', 'QString', u'大小'),
        ('progress', 'QString', u'进度'),
        ('temporary', 'QString', u'试听歌单'),
        ('favorite', 'QString', u'我的收藏'),
        ('myPlaylist', 'QString', u'我创建的歌单'),
        ('newPlaylist', 'QString', u'新建歌单'),
    )

    __contextName__ = "I18nWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        pass
