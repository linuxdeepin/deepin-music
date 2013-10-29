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
import copy

from dtk.ui.utils import (cairo_disable_antialias,
                          color_hex_to_cairo, propagate_expose)
from widget.global_search import GlobalSearch
from widget.ui_utils import switch_tab, draw_line, draw_alpha_mask
from helper import Dispatcher
from widget.completion_window import search_entry, completion_grab_window, completion_window
from widget.local_browser import local_browser




class BrowserMananger(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Search Widgets at top.
        # self.search_box = SearchBox()
        search_entry.entry_box.connect("enter-press", self.on_searchbox_search)
        search_entry.connect("search", self.on_searchbox_search)
        
        search_box_align = gtk.Alignment()
        search_box_align.connect("expose-event", self.on_top_hbox_expose)        
        search_box_align.set_padding(0, 1, 1, 0)
        search_box_align.set(0, 0, 1, 1)
        search_box_align.add(search_entry)
        
        # Bottom widgets and is switchable.
        self.bottom_box = gtk.VBox()
        self.global_search = GlobalSearch()
        self.global_search.close_button.connect("clicked", self.on_global_search_close)

        self.last_browser = local_browser
        self.bottom_box.add(local_browser)
        self.bottom_box_align = gtk.Alignment()
        self.bottom_box_align.set_padding(0, 0, 1, 1)
        self.bottom_box_align.set(1, 1, 1, 1)
        self.bottom_box_align.add(self.bottom_box)
        
        self.pack_start(search_box_align, False, True)
        self.pack_start(self.bottom_box_align, True, True)
        
        self.connect("expose-event", self.on_expose_event)
        Dispatcher.connect("switch-browser", self.on_dispatcher_switch_browser)
        
        
        # Connect entry
        # search_entry.entry.connect("changed", self.on_search_entry_changed)
        # search_entry.entry.connect_after("key-press-event", self.on_search_entry_key_press)
        # search_entry.entry.connect_after("key-release-event", self.on_search_entry_key_release)
        
        self.in_press = False
        self.press_id = 0
        self.entry_changed = False

        
    def on_search_entry_changed(self, widget, string):    
        self.press_id += 1
        self.entry_changed = True
        
    def on_search_entry_key_press(self, widget, event):    
        self.in_press = True
        self.press_id += 1
    
    def on_search_entry_key_release(self, widget, event):
        self.in_press = False
        press_id = copy.deepcopy(self.press_id)
        self.popup_completion(press_id)
        
    def popup_completion(self, press_id):    
        if (not self.in_press) and press_id == self.press_id and self.entry_changed:
            search_keyword = search_entry.get_text()
            if search_keyword:
                completion_window.show(search_keyword)
            else:    
                completion_grab_window.popup_grab_window_focus_out()
            self.entry_changed = False    
            
    def on_searchbox_search(self, widget, keyword):    
        # completion_grab_window.popup_grab_window_focus_out()
        if keyword:
            self.global_search.begin_search(keyword)    
            switch_tab(self.bottom_box, self.global_search)
            
    def on_global_search_close(self, widget):        
        search_entry.clear()
        switch_tab(self.bottom_box, self.last_browser)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x, rect.y, rect.width - 1, rect.height ,"layoutRight")
        
        draw_line(cr, (rect.x + 1, rect.y), 
                  (rect.x + 1, rect.y + rect.height), "#b0b0b0")
        return False
        
    def on_dispatcher_switch_browser(self, obj, item, is_switched):    
        self.last_browser = item.browser_widget
        if is_switched:
            search_entry.clear()            
            switch_tab(self.bottom_box, item.browser_widget)
            
    def on_top_hbox_expose(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation

        with cairo_disable_antialias(cr):
            cr.set_line_width(1)
            cr.set_source_rgba(*color_hex_to_cairo("#C3C4C5"))
            cr.move_to(rect.x + 1, rect.y + 1)
            cr.rel_line_to(0, rect.height - 1)
            cr.rel_line_to(rect.width - 2, 0)
            cr.rel_line_to(0, -rect.height + 1)
            cr.stroke()
            
        propagate_expose(widget, event)    
        
        return True
