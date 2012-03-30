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

import gobject
from dtk.ui.utils import get_content_size
from dtk.ui.listview import render_text
from dtk.ui.constant import ALIGN_END
from constant import DEFAULT_FONT_SIZE


class SongItem(gobject.GObject):
    '''song items for deepin-ui listview'''
    __gsignals__ = {"redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }
    def __init__(self, song):
        '''Init song item.'''
        gobject.GObject.__init__(self)
        self.update(song)
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
        
    def update(self, song):
        '''update'''
        self.song = song
        
        self.title = song.get_str("title")
        self.artist = song.get_str("artist")
        self.length = song.get_str("#duration")
        
        # Calculate item size.
        self.title_padding_x = 10
        self.title_padding_y = 5
        (self.title_width, self.title_height) = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
        self.artist_padding_x = 10
        self.artist_padding_y = 5
        (self.artist_width, self.artist_height) = get_content_size(self.artist, DEFAULT_FONT_SIZE)

        self.length_padding_x = 10
        self.length_padding_y = 5
        (self.length_width, self.length_height) = get_content_size(self.length, DEFAULT_FONT_SIZE)
        
    def render_title(self, cr, rect):
        '''Render title.'''
        rect.x += self.title_padding_x
        render_text(cr, rect, self.title, font_size=DEFAULT_FONT_SIZE)
    
    def render_artist(self, cr, rect):
        '''Render artist.'''
        rect.x += self.artist_padding_x
        render_text(cr, rect, self.artist, font_size=DEFAULT_FONT_SIZE)
    
    def render_length(self, cr, rect):
        '''Render length.'''
        rect.width -= self.length_padding_x
        render_text(cr, rect, self.length, ALIGN_END, font_size=DEFAULT_FONT_SIZE)
        
    def get_column_sizes(self):
        '''Get sizes.'''
        return [(min(self.title_width + self.title_padding_x * 2, 120),
                 self.title_height + self.title_padding_y * 2),
                (min(self.artist_width + self.artist_padding_x * 2, 100)
                 self.artist_height + self.artist_padding_y * 2),
                (self.length_width + self.length_padding_x * 2, 
                 self.length_height + self.length_padding_y * 2),
                ]    
    
    def get_renders(self):
        '''Get render callbacks.'''
        return [self.render_title,
                self.render_artist,
                self.render_length]
        
    def get_song(self):
        return self.song
    
    def __hash__(self):
        return hash(self.song.get("uri"))
    
    def __repr__(self):
        return "<SongItem %s>" % self.song.get("uri")
    
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
