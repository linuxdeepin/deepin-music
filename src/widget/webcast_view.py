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

from widget.ui_utils import draw_single_mask

class WebcastView(ListView):
    
    def __init__(self):
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2), ("text/plain", 0, 3)]        
        ListView.__init__(self, drag_data=(targets, gtk.gdk.ACTION_COPY, 1))
        self.set_expand_column(0)
        self.connect("drag-data-get", self.__on_drag_data_get)
        self.connect("double-click-item", self.__on_double_click_item)
        self.connect("right-press-items", self.__on_right_press_items)
        
    def draw_item_hover(self, cr, x, y, w, h):
        draw_single_mask(cr, x, y, w, h, "simpleItemHover")
        
    def draw_item_select(self, cr, x, y, w, h):    
        draw_single_mask(cr, x, y, w, h, "simpleItemHighlight")
        
    def draw_item_highlight(self, cr, x, y, w, h):    
        draw_single_mask(cr, x, y, w, h, "simpleItemSelect")
        
    def __on_drag_data_get(self, widget, context, selection, info, timestamp):
        pass
    
    def __on_right_press_items(self, widget, x, y, item, select_items):
        pass
    
    def __on_double_click_item(self, widget, item, column, x, y):
        pass
