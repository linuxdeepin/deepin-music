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
from dtk.ui.utils import get_content_size
from dtk.ui.listview import render_text
from dtk.ui.constant import ALIGN_END
from dtk.ui.scrolled_window import ScrolledWindow


from constant import DEFAULT_FONT_SIZE
from widget.ui import SongView, app_theme

class PlaylistItem(gobject.GObject):

    __gsignals__ = {"active" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
                    "right-press-item" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (int, int,)),}
    
    def __init__(self, playlist, editable=True):
        '''Init song item.'''
        gobject.GObject.__init__(self)
        self.editable = editable
        self.update(playlist)
        
    def set_text(self, text):    
        self.text = text
        
    def get_text(self):    
        return self.text
    
    def get_editable(self):
        return self.editable
    
    def set_editable(self, value):
        self.editable = value
        
    def update(self, playlist):
        '''update'''
        self.playlist = playlist
        songs = self.playlist.get_songs()
        self.song_view = SongView()
        self.song_view.add_songs(songs)
        self.scrolled_window = ScrolledWindow(app_theme.get_pixbuf("skin/main.png"))
        self.scrolled_window.add_child(self.song_view)
        self.scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        self.text = playlist.get_name()
    
    def get_list_widget(self):
        return self.scrolled_window
    
    def get_songs(self):
        if self.song_view:
            return self.song_view.get_songs()
        
    def get_name(self):
        return self.text
