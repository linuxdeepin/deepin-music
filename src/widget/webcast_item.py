#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Hou Shaohui
# 
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou Shaohui <houshao55@gmail.com>
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
import pango
from dtk.ui.draw import draw_pixbuf, draw_text
from widget.skin import app_theme
from webcasts import WebcastsDB

import utils

class WebcastItem(gobject.GObject):    
    
    __gsignals__ = {"redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }
    
    def __init__(self, webcast):
        gobject.GObject.__init__(self)
        self.index = 0
        self.side_padding = 5
        self.item_height = 31
        self.webcast = webcast
        self.is_collected = WebcastsDB.is_collected(webcast.get("uri"))         
        self.item_width = 121
        self.star_normal_pixbuf = app_theme.get_pixbuf("webcast/collect_normal.png").get_pixbuf()
        self.star_press_pixbuf = app_theme.get_pixbuf("webcast/collect_press.png").get_pixbuf()
        self.update_sizes()
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
        
    def emit_redraw_request(self):
        self.emit("redraw-request")
    
    def get_column_sizes(self):
        return [
            (100, 32),
            (self.star_icon_w + self.star_icon_padding_x * 2, 32),
            (20, 32)
            ]
    def update_sizes(self):
        self.star_icon_padding_x = 2
        self.star_icon_padding_y = 5
        self.star_icon_w = self.star_normal_pixbuf.get_width()
        self.star_icon_h = self.star_normal_pixbuf.get_height()
    
    def render_content(self, cr, rect, in_select, in_highlight):    
        if in_select or in_highlight:
            text_color = "#ffffff"
            icon_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_press.png").get_pixbuf()            
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            icon_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_normal.png").get_pixbuf()            
            
        icon_y = rect.y + (rect.height - icon_pixbuf.get_height())  / 2 
        padding_x = 10
        draw_pixbuf(cr, icon_pixbuf, rect.x + padding_x, icon_y)    
            
        draw_text(cr, utils.xmlescape(self.webcast.get_str("title")), rect.x + icon_pixbuf.get_width() + padding_x * 2, 
                  rect.y, rect.width - icon_pixbuf.get_width() - padding_x * 2, rect.height, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT, text_size=9)    
        
    def render_star(self, cr, rect, in_select, in_highlight):
        icon_y = rect.y + (rect.height - self.star_icon_h) / 2
        rect.x += self.star_icon_padding_x
        
        if self.is_collected or self.webcast.get("collect"):
            pixbuf = self.star_press_pixbuf
        else:    
            pixbuf = self.star_normal_pixbuf
        draw_pixbuf(cr, pixbuf, rect.x , icon_y)
        
    def get_renders(self):    
        return [self.render_content, self.render_star, self.render_block]
    
    def render_block(self, cr, rect, in_select, in_highlight):
        pass
        
    def get_webcast(self):
        return self.webcast
    
    def __hash__(self):
        return hash(self.webcast.get("uri"))
    
    def __repr__(self):
        return "<SongItem %s>" % self.webcast.get("uri")
    
    def __cmp__(self, other_item):
        if not other_item:
            return -1
        try:
            return cmp(self.webcast, other_item.get_webcast())
        except AttributeError: return -1
        
    def __eq__(self, other_item):    
        try:
            return self.webcast == other_item.get_webcast()
        except:
            return False
