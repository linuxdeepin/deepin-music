#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Wang Yong
# 
# Author:     Wang Yong <lazycat.manatee@gmail.com>
# Maintainer: Wang Yong <lazycat.manatee@gmail.com>
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


import pango
import gobject
import gtk

from dtk.ui.timeline import Timeline, CURVE_SINE
from dtk.ui.utils import get_content_size, color_hex_to_cairo
from dtk.ui.draw import draw_text

from widget.skin import app_theme

class TabItem(object):
    
    def __init__(self, title, search_view):
        self.title = title
        self.search_view = search_view

class TabSwitcher(gtk.EventBox):
    '''
    class docs
    '''
	
    __gsignals__ = {
        "tab-switch-start" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "tab-switch-complete" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (int,)),
        "click-current-tab" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (int,)),
    }
    
    def __init__(self, items):
        '''
        init docs
        '''
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.items = items
        self.tab_name_size = 10
        self.tab_number = len(self.items)
        tab_sizes = map(lambda item: get_content_size(item.title, self.tab_name_size), self.items)
        self.tab_name_padding_x = 10
        self.tab_name_padding_y = 2
        self.tab_width = max(map(lambda (width, height): width, tab_sizes)) + self.tab_name_padding_x * 2
        self.tab_height = tab_sizes[0][1] + self.tab_name_padding_y * 2
        self.tab_line_height = 1
        self.tab_index = 0
        
        self.tab_animation_x = 0
        self.tab_animation_time = 200 # milliseconds
        
        self.padding_x = 0
        self.padding_y = 15        
        self.in_animiation = False
        
        self.set_size_request(-1, self.tab_height + self.tab_line_height)
        self.line_dcolor = app_theme.get_color("globalItemHighlight")

        self.connect("expose-event", self.expose_tab_switcher)
        self.connect("button-press-event", self.button_press_tab_switcher)
        
    def add_item(self, item):    
        if item not in self.items:
            self.items.append(item)
        self.repack_items()    
        
    def remove_item(self, item):    
        try:
            self.items.remove(item)
        except:    
            pass
        
        self.repack_items()
        
        self.tab_index = 0
        self.emit("tab_switch_start", self.items[0])
        self.queue_draw()
            
    def repack_items(self):        
        self.tab_number = len(self.items)
        tab_sizes = map(lambda item: get_content_size(item.title, self.tab_name_size), self.items)
        self.tab_width = max(map(lambda (width, height): width, tab_sizes)) + self.tab_name_padding_x * 2
        self.tab_height = tab_sizes[0][1] + self.tab_name_padding_y * 2
        self.set_size_request(-1, self.tab_height + self.tab_line_height)
        
    def active_item_by_index(self, index):    
        self.emit("tab_switch_start", self.items[index])
        self.tab_index = index
        self.queue_draw()

    def expose_tab_switcher(self, widget, event):
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation

        # Draw tab line.
        cr.set_source_rgb(*color_hex_to_cairo(self.line_dcolor.get_color()))
        cr.rectangle(rect.x + self.padding_x, 
                     rect.y + self.tab_height,
                     rect.width - self.padding_x * 2, 
                     self.tab_line_height)
        cr.fill()
        
        # Draw tab.
        draw_start_x = rect.x + (rect.width - self.tab_width * self.tab_number) / 2
        if self.in_animiation:
            cr.rectangle(self.tab_animation_x,
                         rect.y,
                         self.tab_width,
                         self.tab_height)
        else:
            cr.rectangle(draw_start_x + self.tab_index * self.tab_width,
                         rect.y,
                         self.tab_width,
                         self.tab_height)
        cr.fill()
        
        # Draw tab name.
        for (tab_index, tab_item) in enumerate(self.items):
            if self.in_animiation:
                tab_name_color = app_theme.get_color("labelText").get_color()
            elif tab_index == self.tab_index:
                tab_name_color = "#FFFFFF"
            else:
                tab_name_color = app_theme.get_color("labelText").get_color()
            draw_text(cr,
                      tab_item.title,
                      draw_start_x + tab_index * self.tab_width,
                      rect.y,
                      self.tab_width,
                      self.tab_height,
                      text_size=self.tab_name_size,
                      text_color=tab_name_color,
                      alignment=pango.ALIGN_CENTER,
                      )
            
    def button_press_tab_switcher(self, widget, event):
        # Init.
        rect = widget.allocation
        # tab_start_x = rect.x + (rect.width - self.tab_width * self.tab_number) / 2
        tab_start_x = (rect.width - self.tab_width * self.tab_number) / 2
        for tab_index in range(0, self.tab_number):
            if tab_start_x + tab_index * self.tab_width < event.x < tab_start_x + (tab_index + 1) * self.tab_width:
                if self.tab_index != tab_index:
                    self.start_animation(tab_index, tab_start_x + rect.x)
                else:
                    self.emit("click_current_tab", self.tab_index)
                break
            
    def start_animation(self, index, tab_start_x):
        if not self.in_animiation:
            self.in_animiation = True
            
            source_tab_x = tab_start_x + self.tab_index * self.tab_width
            target_tab_x = tab_start_x + index * self.tab_width
            
            timeline = Timeline(self.tab_animation_time, CURVE_SINE)
            timeline.connect('update', lambda source, status: self.update_animation(source, status, source_tab_x, (target_tab_x - source_tab_x)))
            timeline.connect("completed", lambda source: self.completed_animation(source, index))
            timeline.run()
            
            self.emit("tab_switch_start", self.items[index])
    
    def update_animation(self, source, status, animation_start_x, animation_move_offset):
        self.tab_animation_x = animation_start_x + animation_move_offset * status
        
        self.queue_draw()
        
    def completed_animation(self, source, index):
        self.tab_index = index
        self.in_animiation = False
        
        self.emit("tab_switch_complete", self.tab_index)
        
        self.queue_draw()
        
gobject.type_register(TabSwitcher)
        
