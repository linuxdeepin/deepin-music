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
from dtk.ui.draw import draw_text
from dtk.ui.utils import get_content_size

from widget.skin import app_theme
from library import MediaDB
from player import Player
from nls import _
import utils


class PlayInfo(gtk.VBox):
    def __init__(self, default_width=110):
        super(PlayInfo, self).__init__()

        self.default_height = 20
        self.set_size_request(default_width, self.default_height)
        self.default_width = default_width
        self.artist_label = _("Deepin Music") + " for Linux Deepin"

        Player.connect("instant-new-song",self.__new_song)
        MediaDB.connect("simple-changed",self.__on_change)
        Player.bin.connect("buffering", self.__on_buffering)
        Player.connect("init-status", self.__on_player_init_status)
        
        self.connect("expose-event", self.on_expose_event)
        self.padding_x = 0
        self.set_size_request(self.default_width, self.default_height)
        self.song = None
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        self.draw_content(cr, rect)
        
    def draw_content(self, cr, rect):    
        draw_text(cr, self.artist_label, rect.x + self.padding_x, rect.y, rect.width - self.padding_x * 2,
                  rect.height, text_size=9.5,
                  gaussian_radious=2,
                  gaussian_color="#000000",
                  border_radious=1,
                  border_color="#000000",
                  text_color = "#FFFFFF"
                  ) 
        
    def __on_player_init_status(self, player):    
        self.set_text(_("Deepin Music") + " for Linux Deepin")
        
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
            title = "%s - %s" % (title, song.get_str("artist"))
        title = utils.xmlescape(title)    
            
        if buffering is not None:    
            if buffering >= 0 and buffering <= 98:
                buffering_title = "%s(%d%%) - %s" % (_("buffering"), buffering, title)
            else:    
                buffering_title = title
            self.set_text(buffering_title)    
            
        else:    
            self.set_text(title)
        
    def set_text(self, title):        
        self.artist_label = title
        self.queue_draw()
