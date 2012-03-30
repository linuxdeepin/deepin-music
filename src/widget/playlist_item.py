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

import gobject
from dtk.ui.utils import get_content_size
from dtk.ui.listview import render_text
from dtk.ui.constant import ALIGN_END
from constant import DEFAULT_FONT_SIZE


class PlaylistItem(gobject.GObject):
    '''Playlist items for deepin-ui listview'''
    __gsignals__ = {"redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }
    def __init__(self, playlist):
        '''Init playlist item.'''
        gobject.GObject.__init__(self)
        self.playlist = playlist
        self.update(playlist)
        self.highlight_item = None
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
        
    def update(self, playlist):
        '''update'''
        self.name = playlist.get_name()
        
        # Calculate item size.
        self.name_padding_x = 10
        self.name_padding_y = 5
        (self.name_width, self.name_height) = get_content_size(self.name, DEFAULT_FONT_SIZE)
        
    def render_name(self, cr, rect):
        '''Render title.'''
        rect.x += self.name_padding_x
        render_text(cr, rect, self.name, font_size=DEFAULT_FONT_SIZE)
        
    def get_column_sizes(self):
        '''Get sizes.'''
        return [(self.name_width + self.name_padding_x * 2, 
                 self.name_height + self.name_padding_y * 2),
                ]    
    
    def get_renders(self):
        '''Get render callbacks.'''
        return [self.render_name]
        
    def get_song_items(self):
        return self.playlist.get_songs()
    
    def set_highlight_item(self, item):
        self.highlight_item = item
        
    def get_highlight_item(self):    
        return self.highlight_item
        
    
    def __hash__(self):
        return hash(self.song.get("uri"))
    
    def __repr__(self):
        return "<PlaylistItem %s>" % self.song.get("uri")
    
    def __cmp__(self, other_item):
        if not other_item:
            return -1
        try:
            return cmp(self.song, other_item.get_song())
        except AttributeError: return -1
        
    def __eq__(self, other_item):    
        try:
            return self.song == other_item.get_song()
        except:
            return False
