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
from dtk.ui.constant import ALIGN_START, ALIGN_END

from widget.ui_utils import render_item_text
import utils

DEFAULT_FONT_SIZE = 8

import utils

class SongItem(gobject.GObject):
    '''song items for deepin-ui listview'''
    __gsignals__ = {"redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }
    def __init__(self, song, extend=False):
        '''Init song item.'''
        gobject.GObject.__init__(self)
        self.song_error = False
        self.update(song)
        self.extend = extend
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
        
    def update(self, song, redraw=False):
        '''update'''
        self.song = song
        
        self.title = utils.xmlescape(song.get_str("title"))
        self.artist = utils.xmlescape(song.get_str("artist"))
        self.length = utils.xmlescape(song.get_str("#duration"))
        self.add_time = utils.xmlescape(song.get_str("#added"))
        self.album = utils.xmlescape(song.get_str("album"))
        
        # Calculate item size.
        self.title_padding_x = 10
        self.title_padding_y = 5
        (self.title_width, self.title_height) = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
        self.artist_padding_x = 10
        self.artist_padding_y = 5
        (self.artist_width, self.artist_height) = get_content_size(self.artist, DEFAULT_FONT_SIZE)

        self.length_padding_x = 2
        self.length_padding_y = 5
        (self.length_width, self.length_height) = get_content_size(self.length, DEFAULT_FONT_SIZE)
        
        self.add_time_padding_x = 10
        self.add_time_padding_y = 5
        (self.add_time_width, self.add_time_height) = get_content_size(self.add_time, DEFAULT_FONT_SIZE)
        
        self.album_padding_x = 10
        self.album_padding_y = 5
        (self.album_width, self.album_height) = get_content_size(self.album, DEFAULT_FONT_SIZE)
        
        if redraw:
            self.emit_redraw_request()
            
    def set_error(self):        
        self.song_error = True
        self.emit_redraw_request()
        
    def clear_error(self):    
        self.song_error = False
        self.emit_redraw_request()
        
    def exists(self):    
        return self.song.exists()
        
    def is_error(self):    
        return self.song_error == True
        
    def render_title(self, cr, rect, in_select, in_highlight):
        '''Render title.'''
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_item_text(cr, self.title, rect, in_select, in_highlight, error=self.song_error)
    
    def render_artist(self, cr, rect, in_select, in_highlight):
        '''Render artist.'''
        rect.x += self.artist_padding_x
        rect.width -= self.artist_padding_x * 2
        render_item_text(cr, self.artist, rect, in_select, in_highlight, error=self.song_error)
    
    def render_length(self, cr, rect, in_select, in_highlight):
        '''Render length.'''
        rect.width -= self.length_padding_x * 2
        rect.x += self.length_padding_x * 2
        # render_item_text(cr, self.length, rect, in_select, in_highlight, align=ALIGN_END, error=self.song_error)
        render_item_text(cr, self.length, rect, in_select, in_highlight, error=self.song_error)
        
    def render_add_time(self, cr, rect, in_select, in_highlight):
        '''Render add_time.'''
        rect.width -= self.add_time_padding_x * 2
        render_item_text(cr, self.add_time, rect, in_select, in_highlight, align=ALIGN_END, error=self.song_error)
        
    def render_album(self, cr, rect, in_select, in_highlight):
        '''Render album.'''
        rect.width -= self.album_padding_x * 2 
        render_item_text(cr, self.album, rect, in_select, in_highlight, error=self.song_error)
        
    def get_column_sizes(self):
        '''Get sizes.'''
        if self.extend:
            return [
                (80, self.title_height + self.title_padding_y * 2),
                (70, self.artist_height + self.artist_padding_y * 2),
                (70, self.album_height + self.album_padding_y * 2),
                (80, self.add_time_height + self.add_time_padding_y * 2)
                ]
        else:
            return [
                (150, 26),
                (100, 26), 
                (self.length_width + self.length_padding_x * 2 + 20, 26)]    
    
    def get_renders(self):
        '''Get render callbacks.'''
        
        if self.extend:
            return [ self.render_title, self.render_artist, self.render_album, self.render_add_time]
        else:
            return [self.render_title, self.render_artist, self.render_length]
        
            
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
