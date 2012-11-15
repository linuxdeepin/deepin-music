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
import gobject
import copy
import os 
import pango

from collections import namedtuple
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.listview import ListView
from dtk.ui.new_treeview import TreeView, TreeItem
from dtk.ui.draw import draw_pixbuf, draw_text
from dtk.ui.utils import get_content_size, get_widget_root_coordinate, get_match_parent
from dtk.ui.constant import WIDGET_POS_TOP_RIGHT
from dtk.ui.popup_grab_window import PopupGrabWindow, wrap_grab_window
from dtk.ui.window import Window
from dtk.ui.paned import HPaned

import utils
from widget.ui_utils import (draw_single_mask, draw_alpha_mask, render_item_text,
                             switch_tab, draw_range, draw_line)
from widget.skin import app_theme
from collections import OrderedDict
from constant import DEFAULT_FONT_SIZE
from webcasts import WebcastsDB
from xdg_support import get_config_file
from helper import Dispatcher
from song import Song
from nls import _

class CategroyItem(TreeItem):    
    def __init__(self, title):
        TreeItem.__init__(self)
        self.column_index = 0
        self.side_padding = 5
        self.item_height = 37
        self.title = title
        self.item_width = 121
        
    def get_height(self):    
        return self.item_height
    
    def get_column_widths(self):
        return (self.item_width,)
    
    def get_column_renders(self):
        return (self.render_title,)
    
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
        
    def emit_redraw_request(self):    
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def select(self):        
        self.is_select = True
        self.emit_redraw_request()
        
    def render_title(self, cr, rect):        
        # Draw select background.
        if self.is_select:
            draw_pixbuf(cr, self.hover_bg, rect.x, rect.y)
            text_color = app_theme.get_color("simpleItemSelect").get_color()
        elif self.is_hover:    
            text_color = app_theme.get_color("simpleItemHover").get_color()
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        draw_text(cr, self.title, rect.x, rect.y, rect.width, rect.height, text_size=11, 
                  text_color = text_color,
                  alignment=pango.ALIGN_CENTER)    
        
        if self.has_icon:
            draw_pixbuf(cr, self.selected_pixbuf, rect.x + 10,
                        rect.y + (rect.height - self.selected_pixbuf.get_height()) / 2)
        
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

