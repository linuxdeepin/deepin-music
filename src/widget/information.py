#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 Deepin, Inc.
#               2011 Hou Shaohui
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


import gtk
from dtk.ui.label import Label
from dtk.ui.utils import get_content_size

from widget.skin import app_theme
from library import MediaDB
from player import Player


class PlayInfo(gtk.VBox):
    def __init__(self, default_width=127):
        super(PlayInfo, self).__init__()

        self.set_size_request(default_width, 44)
        self.default_width = default_width
        self.artist_label = self.create_simple_label("深度音乐", 9.5)
        self.title_label = self.create_simple_label("Linux Deepin ", 9.5)

        Player.connect("instant-new-song",self.__new_song)
        MediaDB.connect("simple-changed",self.__on_change)
        Player.bin.connect("buffering", self.__on_buffering)
        
        self.set_spacing(3)
        artist_label_align = gtk.Alignment()
        artist_label_align.set_padding(1, 0, 0, 0)
        artist_label_align.set(0, 0, 1, 1)
        artist_label_align.add(self.artist_label)
        self.pack_start(artist_label_align, False, False)
        self.pack_start(self.title_label, False, False)
        self.song = None
        
    def __on_buffering(self, playbin, progress):
        if self.song:
            self.update(self.song, progress)

    def __on_change(self,db,songs):
        if self.song in songs:
            self.update(self.song)
    
    def __new_song(self, player, song):
        self.update(song)

    def update(self, song , buffering = None):
        if not song: return
        self.song = song
        self.title_label.set_text(song.get_str("title"))
        self.artist_label.set_text(song.get_str("artist"))
        
    def create_simple_label(self, content, text_size):    
        label = Label(content, app_theme.get_color("labelText"), text_size=text_size, enable_gaussian=True, label_width=self.default_width)
        width, height = get_content_size(content, text_size)
        label.set_size_request(110, height)
        return label
