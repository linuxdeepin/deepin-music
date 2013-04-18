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
import pango

from dtk.ui.entry_treeview import TreeItem
from dtk.ui.draw import draw_text

from widget.song_view import SongView
from widget.ui_utils import switch_tab as switch_box
from widget.ui import LocalEmpty
from constant import PLAYLIST_WIDTH, CATEGROYLIST_WIDTH
from widget.skin import app_theme
from widget.ui_utils import (draw_alpha_mask, create_upper_align, create_bottom_align,
                             create_left_align, create_right_align, draw_single_mask)
from nls import _



DEFAULT_FONT_SIZE = 8


class ListTreeItem(TreeItem):        
    
    def __init__(self, playlist, udi=None):
        TreeItem.__init__(self)
        
        # Init playlist.
        self.playlist = playlist
        self.udi = udi
        self.title = self.playlist.get_name()
        self.song_view = SongView()
        self.song_view.add_songs(self.playlist.get_songs())
        self.song_view.connect("begin-add-items", lambda w: self.switch_it())
        self.song_view.connect("empty-items", lambda w: self.switch_it(False))
        self.song_view.set_size_request(PLAYLIST_WIDTH, -1)
        
        # Init params.
        self.entry = None
        self.item_height = 26
        self.item_width = CATEGROYLIST_WIDTH
        self.ENTRY_COLUMN = 0
        self.is_double_click = False
        self.is_highlight = False
        self.text_padding = 15
        
        # create jobs box.
        self.main_box = gtk.VBox()
        
        self.jobs_main_box = LocalEmpty(self.song_view.on_drag_data_received,
                                        self.song_view.recursion_add_dir)
        self.jobs_main_box.set_size_request(PLAYLIST_WIDTH, -1)
        
    def get_list_widget(self):
        if self.get_songs():
            switch_box(self.main_box, self.song_view)
        else:    
            switch_box(self.main_box, self.jobs_main_box)
        return self.main_box    
        
    def switch_it(self, scrolled_window=True):
        if scrolled_window:
            switch_box(self.main_box, self.song_view)
        else:    
            switch_box(self.main_box, self.jobs_main_box)
            
    def get_songs(self):
        if self.song_view:
            return self.song_view.get_songs()
        
    def get_title(self):    
        return self.title
        
    def set_title(self, value):    
        self.title = value
        
    def emit_redraw_request(self):
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
        
    def get_height(self):    
        return self.item_height
    
    def get_column_widths(self):
        return (self.item_width, )
    
    def get_column_renders(self):
        return (self.render_title,)
    
    def render_title(self, cr, rect):
        if self.is_highlight:    
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width - 3, rect.height, "globalItemHighlight")
        elif self.is_hover:
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width - 3, rect.height, "globalItemHover")
        
        if self.is_highlight:
            text_color = "#FFFFFF"
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        rect.x += self.text_padding    
        rect.width -= self.text_padding * 2
        
            
        draw_text(cr, self.title, rect.x, rect.y, rect.width, rect.height, text_size=9, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT)    
        
    def render_content(self, cr, rect):
        if self.is_highlight:    
            if not self.is_double_click:
                draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "globalItemHighlight")
                text_color = "#FFFFFF"
            else:    
                text_color = app_theme.get_color("labelText").get_color()
                
        elif self.is_hover:
            text_color = app_theme.get_color("labelText").get_color()
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "globalItemHover")
        else:    
            text_color = app_theme.get_color("labelText").get_color()

            
        if not self.is_highlight:    
            self.entry_buffer.move_to_start()
            
        self.entry_buffer.set_text_color(text_color)
        width, height = self.entry_buffer.get_content_size()
        offset_y = (self.item_height - height) / 2
        # offset_x = (self.item_height - width) / 2
        rect.y += offset_y
        rect.x += 10
        # rect.x += offset_x
        
        if self.entry and self.entry.allocation.width == self.get_column_widths()[0]-4:
            self.entry.calculate()
            self.entry_buffer.set_text_color("#000000")
            self.entry_buffer.render(cr, rect, self.entry.im)
        else:
            self.entry_buffer.render(cr, rect)
        
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
        
    def select(self):        
        self.is_select = True
        self.emit_redraw_request()
        
    def highlight(self):    
        self.is_highlight = True
        self.emit_redraw_request()
        
    def unhighlight(self):    
        self.is_highlight = False
        self.emit_redraw_request()
        
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
        
    def single_click(self, column, offset_x, offset_y):
        self.is_double_click = False
        self.emit_redraw_request()
        
    def double_click(self, column, offset_x, offset_y):
        self.is_double_click = True
