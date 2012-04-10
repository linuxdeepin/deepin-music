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
from widget.ui import SongView

class PlaylistItem(gobject.GObject):
    '''song items for deepin-ui listview'''
    __gsignals__ = {"redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }
    
    def __init__(self, playlist):
        '''Init song item.'''
        gobject.GObject.__init__(self)
        self.update(playlist)
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
        
    def update(self, playlist):
        '''update'''
        self.playlist = playlist
        songs = self.playlist.get_songs()
        self.song_view = SongView()
        self.song_view.add_songs(songs)
        self.scrolled_window = ScrolledWindow()
        self.scrolled_window.add_child(self.song_view)
        self.scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        self.title = playlist.get_name()
        
        # Calculate item size.
        self.title_padding_x = 10
        self.title_padding_y = 5
        (self.title_width, self.title_height) = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
    def render_title(self, cr, rect):
        '''Render title.'''
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_text(cr, rect, self.title, font_size=DEFAULT_FONT_SIZE)
    
        
    def get_column_sizes(self):
        '''Get sizes.'''
        return [(min(self.title_width + self.title_padding_x * 2, 120),
                 self.title_height + self.title_padding_y * 2),
                ]    
    
    def get_renders(self):
        '''Get render callbacks.'''
        return [self.render_title]
    
    def get_list_widget(self):
        return self.scrolled_window
    
    def get_songs(self):
        if self.song_view:
            return self.song_view.get_songs()
