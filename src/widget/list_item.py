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

import gtk
import gobject
from dtk.ui.scrolled_window import ScrolledWindow

from widget.song_view import SongView

DEFAULT_FONT_SIZE = 8


class PlaylistItem(gobject.GObject):
    
    def __init__(self, playlist):
        '''Init song item.'''
        self.item_id = None
        self.update(playlist)
        
    def set_title(self, value):    
        self.title = value
        
    def get_title(self):    
        return self.title
    
    def get_left_image(self):
        return None
    
    def get_has_arrow(self):
        return None
    
    def set_item_id(self, index):
        self.item_id = index
        
    def get_item_id(self):    
        return self.item_id
        
    def update(self, playlist):
        '''update'''
        self.playlist = playlist
        songs = self.playlist.get_songs()
        self.song_view = SongView()
        self.song_view.add_songs(songs)
        self.scrolled_window = ScrolledWindow(0, 0)
        self.scrolled_window.add_child(self.song_view)
        self.scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        self.scrolled_window.set_size_request(220, -1)
        self.title = playlist.get_name()
        
    def get_list_widget(self):
        return self.scrolled_window
    
    def get_songs(self):
        if self.song_view:
            return self.song_view.get_songs()
