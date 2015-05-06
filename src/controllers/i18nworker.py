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
        ('setting_basicSetting', 'QString', u'基础设置'),
        ('setting_start', 'QString', u'启动'),
        ('setting_autoPlay', 'QString', u'自动播放'),
        ('setting_showDesktopLRC', 'QString', u'显示桌面歌词'),
        ('setting_continueLastPlayProgress', 'QString', u'继续上次播放的进度'),
        ('setting_coverSkin', 'QString', u'启用唱片皮肤'),
        ('setting_fade', 'QString', u'启用淡出淡入'),
        ('setting_close', 'QString', u'关闭'),
        ('setting_minimized', 'QString', u'最小化到托盘'),
        ('setting_quit', 'QString', u'立即退出'),
        ('setting_addtoPlaylist', 'QString', u'添加音乐到播放列表'),
        ('setting_firstPlay', 'QString', u'优先播放'),
        ('setting_nextPaly', 'QString', u'下一首播放'),
        ('setting_lastPlay', 'QString', u'最后播放'),
        ('setting_keyBindings', 'QString', u'快捷键'),
        ('setting_enableKeyBindings', 'QString', u'启用快捷键'),
        ('setting_last', 'QString', u'上一首'),
        ('setting_next', 'QString', u'下一首'),
        ('setting_volumnIncrease', 'QString', u'音量增大'),
        ('setting_volumeDecrease', 'QString', u'音量减小'),
        ('setting_playPause', 'QString', u'暂停/继续'),
        ('setting_simpleFullMode', 'QString', u'简洁/完整模式'),
        ('setting_miniFullMode', 'QString', u'迷你/完整模式'),
        ('setting_hideShowWindow', 'QString', u'隐藏/显示窗口'),
        ('setting_hideShowdesktopLRC', 'QString', u'隐藏/显示桌面歌词'),
        ('setting_desktopLRC', 'QString', u'桌面歌词'),
        ('setting_fontType', 'QString', u'字体'),
        ('setting_fontSize', 'QString', u'字号'),
        ('setting_fontItalic', 'QString', u'字型'),
        ('setting_lineNumber', 'QString', u'行数'),
        ('setting_fontAlignment', 'QString', u'对齐'),
        ('setting_background1', 'QString', u'颜色阴影'),
        ('setting_background2', 'QString', u'颜色阴影'),
        ('setting_backgroundSize', 'QString', u'阴影大小'),
        ('setting_download', 'QString', u'下载'),
        ('setting_downloadFolder', 'QString', u'下载目录'),
        ('setting_about', 'QString', u'关于'),
    )

    __contextName__ = "I18nWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        pass
