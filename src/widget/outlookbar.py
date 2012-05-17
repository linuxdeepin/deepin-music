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

from dtk.ui.constant import BUTTON_PRESS, BUTTON_NORMAL, BUTTON_HOVER
from dtk.ui.draw import draw_vlinear, draw_pixbuf, draw_font
from dtk.ui.utils import widget_fix_cycle_destroy_bug, propagate_expose



ITEM_COLOR = {
    "SimpleItemPress" : [
        (0, ("#5BAED0", 0.8)),
        (1, ("#5BAED0", 0.5))],
    "SimpleItemHover" : [
        (0, ("#CCE5EC", 0.8)),
        (1, ("#CCE5EC", 0.5)),
	 ],
    "SimpleItem" : "#000000",
    "SimpleSelectItem" : "#FFFFFF"
    }

class BaseBar(gtk.VBox):
        
    def __init__(self, init_index=0):
        gtk.VBox.__init__(self)
        self.current_index = init_index
    
        
    def set_index(self, index):    
        self.queue_draw()
        self.current_index = index
        
    def get_index(self):    
        return self.current_index
    
class OptionBar(BaseBar):    
    
    def __init__(self, items, init_index=0, font_size=10, padding_left=20, padding_middle=10, padding_right=25):
        BaseBar.__init__(self, init_index)
        
        if items:
            for index, item in enumerate(items):
                simple_item = SimpleItem(
                    item, index, font_size, 25, padding_left, padding_middle, padding_right, self.set_index, self.get_index)
                self.pack_start(simple_item)
        self.show_all()        
        
class OptionTitleBar(BaseBar):
    
    def __init__(self, title, child_items=None):
        BaseBar.__init__(self, init_index=-1)

        title_item = SimpleItem(title, 0, 10, 25, 20, 10, 25, self.set_index, self.get_index)
        self.pack_start(title_item, False, True)
        if child_items:
            for index, item in enumerate(child_items):
                simple_item = SimpleItem(
                    item, index + 1 , 9 , 25, 35, 10, 25, self.set_index, self.get_index)
                self.pack_start(simple_item)
        self.show_all()        
        
    
class SimpleItem(gtk.Button):
    '''Simple item.'''
	
    def __init__(self, element, index, font_size, item_height,
                 padding_left, padding_middle, padding_right,
                 set_index, get_index):
        
        # Init.
        super(SimpleItem, self).__init__()
        self.font_size = font_size
        self.index = index
        self.set_index = set_index
        self.get_index = get_index
        self.padding_left = padding_left
        self.padding_right = padding_right
        self.font_offset = padding_middle
        self.normal_dpixbuf, self.press_dpixbuf, self.content, self.clicked_callback = element
        widget_fix_cycle_destroy_bug(self)
        
        self.set_size_request(150, item_height)
        self.connect("expose-event", self.expose_simple_item)
        self.connect("clicked", lambda w: self.wrap_item_clicked_action())
        
    def wrap_item_clicked_action(self):   
        if self.clicked_callback:
            self.clicked_callback()
        self.set_index(self.index)    
        
    def expose_simple_item(self, widget, event):    
        
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        font_color = ITEM_COLOR["SimpleItem"]
        item_pixbuf = self.normal_dpixbuf.get_pixbuf()
        select_index = self.get_index()
        
        if widget.state == gtk.STATE_NORMAL:
            if select_index == self.index:
                select_status = BUTTON_PRESS
            else:    
                select_status = BUTTON_NORMAL
                
        elif widget.state == gtk.STATE_PRELIGHT:        
            if select_index == self.index:
                select_status = BUTTON_PRESS
            else:    
                select_status = BUTTON_HOVER
                
        elif widget.state == gtk.STATE_ACTIVE:        
            select_status = BUTTON_PRESS
            
        if select_status == BUTTON_PRESS:    
            draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, ITEM_COLOR["SimpleItemPress"])
            font_color = ITEM_COLOR["SimpleSelectItem"]
            item_pixbuf = self.press_dpixbuf.get_pixbuf()
            
        elif select_status == BUTTON_HOVER:    
            draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, ITEM_COLOR["SimpleItemHover"])
            
        # Draw pixbuf.    
        draw_pixbuf(cr, item_pixbuf, rect.x + self.padding_left, rect.y + (rect.height - item_pixbuf.get_height()) / 2)    
        
        
        # Draw content.
        draw_font(cr, self.content, self.font_size, font_color,
                  rect.x + self.padding_left + self.font_offset, 
                  rect.y,
                  rect.width - self.padding_left - self.font_offset - self.padding_right,
                  rect.height)
        
        propagate_expose(widget, event)
        
        return True
    
gobject.type_register(SimpleItem)    

        
        
    
    
    