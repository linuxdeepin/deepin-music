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

import gtk
from dtk.ui.listview import ListView
from dtk.ui.new_treeview import TreeView, TreeItem
from dtk.ui.draw import draw_pixbuf, draw_text

from widget.ui_utils import draw_single_mask
from widget.skin import app_theme
import pango

    
class WebcastItem(TreeItem):    
    def __init__(self, webcast):
        TreeItem.__init__(self)
        self.column_index = 0
        self.side_padding = 5
        self.item_height = 37
        self.webcast = webcast
        self.item_width = 121
        
    def get_height(self):    
        return self.item_height
    
    def get_column_widths(self):
        return (50, -1)
    
    def get_column_renders(self):
        return (self.render_icon, self.render_title)
    
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
        
    def emit_redraw_request(self):    
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def select(self):        
        self.is_select = True
        self.emit_redraw_request()
        
    def render_icon(self, cr, rect):        
        # Draw select background.
        if self.is_select:
            icon_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_press.png".get_pixbuf())
        else:    
            icon_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_normal.png".get_pixbuf())
            
        icon_y = (rect.y - icon_pixbuf.get_height())  / 2 
        padding_x = 10
        draw_pixbuf(cr, icon_pixbuf, rect.x + padding_x, icon_y)    
        
    def render_title(self, cr, rect):    
        if self.is_select:
            text_color = app_theme.get_color("simpleItemSelect").get_color()
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        draw_text(cr, self.webcast.get_str("title"), rect.x, rect.y, rect.width, rect.height, 
                  text_color = text_color,
                  alignment=pango.ALIGN_CENTER)    
        
    def expand(self):
        pass
    
    def unexpand(self):
        pass
    
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
        
    def button_press(self, column, offset_x, offset_y):
        pass
    
    def single_click(self, column, offset_x, offset_y):
        pass        

    def double_click(self, column, offset_x, offset_y):
        pass        
    
    def draw_drag_line(self, drag_line, drag_line_at_bottom=False):
        pass

    
class WebcastView(TreeView):
    def __init__(self):
        TreeView.__init__(*)
