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
from nls import _


class PlayInfo(gtk.VBox):
    def __init__(self, default_width=110):
        super(PlayInfo, self).__init__()

        self.default_height = 20
        self.set_size_request(default_width, self.default_height)
        self.default_width = default_width
        self.artist_label = self.create_simple_label(_("Deepin Music") + " for Linux Deepin", 9.5)

        Player.connect("instant-new-song",self.__new_song)
        MediaDB.connect("simple-changed",self.__on_change)
        Player.bin.connect("buffering", self.__on_buffering)
        Player.connect("init-status", self.__on_player_init_status)
        
        artist_label_align = gtk.Alignment()
        artist_label_align.set_padding(1, 0, 0, 0)
        artist_label_align.set(0, 0, 1, 1)
        artist_label_align.add(self.artist_label)
        self.pack_start(artist_label_align, False, True)
        self.song = None
        
    def __on_player_init_status(self, player):    
        self.artist_label.set_text(_("Deepin Music") + " for Linux Deepin")
        
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
        title = song.get_str("title")
        if song.get_str("artist"):
            self.artist_label.set_text("%s - %s" % (title, song.get_str("artist")))
        elif buffering is not None:    
            if buffering >= 0 and buffering <= 98:
                self.artist_label.set_text("%s(%d%%) - %s" % (_("buffering"), buffering, title))
            elif buffering >= 99:    
                if song.get_str("artist"):
                    self.artist_label.set_text("%s - %s" % (title, song.get_str("artist")))
                else:
                    self.artist_label.set_text(title)
        else:    
            self.artist_label.set_text(title)
        
    def create_simple_label(self, content, text_size):    
        label = Label(content, app_theme.get_color("labelText"), text_size=text_size, enable_gaussian=True, label_width=self.default_width)
        # width, height = get_content_size(content, text_size)
        label.set_size_request(110, self.default_height)
        return label
