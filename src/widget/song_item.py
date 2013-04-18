#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2013 Deepin, Inc.
#               2011 ~ 2013 Hou ShaoHui
# 
# Author:     Hou ShaoHui <houshao55@gmail.com>
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


from dtk.ui.utils import get_content_size
from dtk.ui.constant import ALIGN_START
from dtk.ui.treeview import TreeItem

from widget.ui_utils import render_item_text, draw_single_mask

DEFAULT_FONT_SIZE = 8


class SongItem(TreeItem):
    
    def __init__(self, song, extend=False):
        
        TreeItem.__init__(self)
        
        self.song_error = False
        self.update(song)
        self.extend = extend
        self.height = 26
        
        self.is_highlight = False
        self.column_index = 0
        
        self.default_height = 26
    
    def emit_redraw_request(self):
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
        
    def update(self, song, redraw=False):
        '''update'''
        self.song = song
        self.title = song.get_str("title")
        self.artist = song.get_str("artist")
        self.length = song.get_str("#duration")
        self.add_time = song.get_str("#added")
        self.album = song.get_str("album")
        
        # Calculate item size.
        self.title_padding_x = 15
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
        if not self.song_error:
            self.song_error = True
            self.emit_redraw_request()
        
    def clear_error(self):    
        if self.song_error:
            self.song_error = False
            self.emit_redraw_request()
        
    def exists(self):    
        return self.song.exists()
        
    def is_error(self):    
        return self.song_error == True
        
    def render_title(self, cr, rect):
        '''Render title.'''
        if self.is_highlight:    
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_select:    
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "globalItemSelect")
        elif self.is_hover:
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "globalItemHover")
        
        # if self.is_highlight:
        #     text_color = "#ffffff"
        # else:    
        #     text_color = app_theme.get_color("labelText").get_color()
            
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_item_text(cr, self.title, rect, self.is_select, self.is_highlight, error=self.song_error)
    
    def render_artist(self, cr, rect):
        '''Render artist.'''
        if self.is_highlight:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_select:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemSelect")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
            
        rect.x += self.artist_padding_x
        rect.width -= self.artist_padding_x * 2
        render_item_text(cr, self.artist, rect, self.is_select, self.is_highlight, error=self.song_error)
    
    def render_length(self, cr, rect):
        '''Render length.'''
        if self.is_highlight:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_select:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemSelect")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
            
        rect.width -= self.length_padding_x * 2
        rect.x += self.length_padding_x * 2
        render_item_text(cr, self.length, rect, self.is_select, self.is_highlight, error=self.song_error, font_size=8)
        
    def render_add_time(self, cr, rect):
        '''Render add_time.'''
        if self.is_highlight:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_select:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemSelect")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
        rect.width -= self.add_time_padding_x * 2
        render_item_text(cr, self.add_time, rect, self.is_select, self.is_highlight, align=ALIGN_START,
                         error=self.song_error, font_size=8)
        
    def render_album(self, cr, rect):
        '''Render album.'''
        if self.is_highlight:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_select:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemSelect")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
        rect.width -= self.album_padding_x * 2 
        render_item_text(cr, self.album, rect, self.is_select, self.is_highlight, error=self.song_error)
        
    def get_height(self):    
        # if self.is_highlight:
        #     return 32
        return self.default_height
        
    def get_column_widths(self):
        '''Get sizes.'''
        if self.extend:
            return (100, 100, 100, 90)
        else:
            return (156, 102, 51) 
        
    
    def get_column_renders(self):
        '''Get render callbacks.'''
        
        if self.extend:
            return (self.render_title, self.render_artist, self.render_album, self.render_add_time)
        else:
            return (self.render_title, self.render_artist, self.render_length)
        
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
    
    def select(self):    
        self.is_select = True
        self.emit_redraw_request()
        
    def highlight(self):    
        self.is_highlight = True
        self.is_select = False
        self.emit_redraw_request()
        
    def unhighlight(self):    
        self.is_highlight = False
        self.is_select = False
        self.emit_redraw_request()
        
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
            
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
