#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
#
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou ShaoHui <houshao55@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import os
import gtk
import gobject
from dtk.ui.label import Label
from dtk.ui.draw import draw_hlinear
from dtk.ui.navigatebar import Navigatebar
from widget.skin import NormalWindow, app_theme

class SongInfo(gtk.VBox):
    def __init__(self, *args):
        super(SongInfo, self).__init__(*args)
        self.cover_image = gtk.Image()
        self.cover_image.set_size_request(80, 80)
        
        title_box, self.display_title = self.__create_simple_label("歌名:")
        artist_box, self.display_artist = self.__create_simple_label("艺术家:")
        album_box, self.display_album = self.__create_simple_label("专辑:")
        
        simple_info_box = gtk.VBox(spacing=5)
        simple_info_box.pack_start(title_box, False, False)
        simple_info_box.pack_start(artist_box, False, False)
        simple_info_box.pack_start(album_box, False, False)
        base_info_box = gtk.HBox(spacing=10)
        base_info_box.pack_start(self.cover_image, False, False)
        base_info_box.pack_start(simple_info_box, False, False)
        
        filetype_box, self.display_filetype = self.__create_simple_label("文件类型:")
        filesize_box, self.display_filesize = self.__create_simple_label("文件大小:")
        bitrate_box,  self.display_bitrate  = self.__create_simple_label("比特率:")
        duration_box, self.display_duration = self.__create_simple_label("歌曲时长:")
        
        addtime_box,  self.display_addtime  = self.__create_simple_label("添加时间:")
        lastplay_box, self.display_lastplay = self.__create_simple_label("最后播放时间:")
        playcount_box, self.display_playcount = self.__create_simple_label("播放次数:")
        
        fixed_box = gtk.VBox(spacing=5)
        fixed_box.pack_start(filetype_box, False, False)
        fixed_box.pack_start(filesize_box, False, False)
        fixed_box.pack_start(bitrate_box, False, False)
        fixed_box.pack_start(duration_box, False, False)
        
        user_box = gtk.VBox(spacing=5)
        user_box.pack_start(addtime_box, False, False)
        user_box.pack_start(lastplay_box, False, False)
        user_box.pack_start(playcount_box, False, False)
        
        detail_box = gtk.HBox(spacing=5)
        detail_box.pack_start(fixed_box, False, False)
        detail_box.pack_start(user_box, False, False)
        self.pack_start(base_info_box, False, False)
        self.pack_start(detail_box, False, False)

    def __create_simple_label(self, text):    
        label = gtk.Label(text)
        display_label = gtk.Label("")
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(display_label, False, False)
        return (hbox, display_label)
        

