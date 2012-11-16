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
                             switch_tab, draw_range, draw_line, render_text)
from widget.skin import app_theme
from collections import OrderedDict
from constant import DEFAULT_FONT_SIZE
from webcasts import WebcastsDB
from xdg_support import get_config_file
from helper import Dispatcher
from song import Song
from nls import _

class CategroyRaidoItem(TreeItem):    
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
            text_color = app_theme.get_color("simpleItemSelect").get_color()
        elif self.is_hover:    
            text_color = app_theme.get_color("simpleItemSelect").get_color()
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        draw_text(cr, self.title, rect.x, rect.y, rect.width, rect.height, text_size=10, 
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

    
class RecommendItem(gobject.GObject):    
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, title, description, pixbuf):
        '''
        Initialize ItemIcon class.
        
        @param pixbuf: Icon pixbuf.
        '''
        gobject.GObject.__init__(self)
        self.pixbuf_path = None
        self.pixbuf = pixbuf
        self.padding_x = 10
        self.padding_y = 10
        self.default_width = 110
        self.default_height = 100
        self.hover_flag = False
        self.highlight_flag = False
        
        self.title = title
        self.description = description
        
    def emit_redraw_request(self):
        '''
        Emit `redraw-request` signal.
        
        This is IconView interface, you should implement it.
        '''
        self.emit("redraw-request")
        
    def get_width(self):
        '''
        Get item width.
        
        This is IconView interface, you should implement it.
        '''
        return self.default_width
        
    def get_height(self):
        '''
        Get item height.
        
        This is IconView interface, you should implement it.
        '''
        return self.default_height
    
    def render(self, cr, rect):
        '''
        Render item.
        
        This is IconView interface, you should implement it.
        '''
        # Draw cover.
        if not self.pixbuf:
            self.pixbuf = gtk.gdk.pixbuf_new_from_file(self.pixbuf_path)
        
        pixbuf_x = rect.x + (rect.width - self.pixbuf.get_width()) / 2
        draw_pixbuf(cr, self.pixbuf, pixbuf_x, rect.y)
        
        title_rect = gtk.gdk.Rectangle(rect.x + self.padding_x, 
                                       rect.y + self.pixbuf.get_height() + 5,
                                       rect.width - self.padding_x * 2, 11)
        total_rect = gtk.gdk.Rectangle(title_rect.x, title_rect.y + 16, title_rect.width, 9)
        
        render_text(cr, self.title, title_rect, 
                    app_theme.get_color("labelText").get_color(),
                    10)
        render_text(cr, self.description, total_rect,
                    app_theme.get_color("labelText").get_color(),
                    8)
        
    def icon_item_motion_notify(self, x, y):
        '''
        Handle `motion-notify-event` signal.
        
        This is IconView interface, you should implement it.
        '''
        self.hover_flag = True
        
        self.emit_redraw_request()
        
    def icon_item_lost_focus(self):
        '''
        Lost focus.
        
        This is IconView interface, you should implement it.
        '''
        self.hover_flag = False
        
        self.emit_redraw_request()
        
    def icon_item_highlight(self):
        '''
        Highlight item.
        
        This is IconView interface, you should implement it.
        '''
        self.highlight_flag = True

        self.emit_redraw_request()
        
    def icon_item_normal(self):
        '''
        Set item with normal status.
        
        This is IconView interface, you should implement it.
        '''
        self.highlight_flag = False
        
        self.emit_redraw_request()
    
    def icon_item_button_press(self, x, y):
        '''
        Handle button-press event.
        
        This is IconView interface, you should implement it.
        '''
        pass        
    
    def icon_item_button_release(self, x, y):
        '''
        Handle button-release event.
        
        This is IconView interface, you should implement it.
        '''
        pass
    
    def icon_item_single_click(self, x, y):
        '''
        Handle single click event.
        
        This is IconView interface, you should implement it.
        '''
        pass

    def icon_item_double_click(self, x, y):
        '''
        Handle double click event.
        
        This is IconView interface, you should implement it.
        '''
        pass
    
    def icon_item_release_resource(self):
        '''
        Release item resource.

        If you have pixbuf in item, you should release memory resource like below code:

        >>> if self.pixbuf:
        >>>     del self.pixbuf
        >>>     self.pixbuf = None
        >>>
        >>> return True

        This is IconView interface, you should implement it.
        
        @return: Return True if do release work, otherwise return False.
        
        When this function return True, IconView will call function gc.collect() to release object to release memory.
        '''
        return False

