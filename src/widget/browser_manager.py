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

from dtk.ui.utils import (alpha_color_hex_to_cairo, cairo_disable_antialias,
                          color_hex_to_cairo, propagate_expose)
from widget.local_browser import SimpleBrowser
from widget.webcasts_browser import WebcastsBrowser
from widget.skin import app_theme
from widget.ui import SearchBox, CustomEntry
from widget.ui_utils import switch_tab
from widget.slide_switcher import SlideSwitcher
from helper import Dispatcher

class BrowserMananger(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Search Widgets at top.
        self.search_entry = CustomEntry()
        search_button = SearchBox()
        search_button.set_size_request(85, 32)
        top_hbox = gtk.HBox()        
        top_hbox.pack_start(self.search_entry, False, True)
        top_hbox.pack_start(search_button, False, False)
        top_hbox.connect("realize", self.on_top_hbox_realize, 85)
        top_hbox.connect("size-allocate", self.on_top_hbox_size_allocate, 85)

        
        top_hbox_align = gtk.Alignment()
        top_hbox_align.connect("expose-event", self.on_top_hbox_expose)        
        top_hbox_align.set_padding(0, 1, 1, 0)
        top_hbox_align.set(0, 0, 1, 1)
        top_hbox_align.add(top_hbox)
        
        # Bottom widgets and is switchable.
        self.local_browser = SimpleBrowser()
        self.webcasts_browser = WebcastsBrowser()
        self.bottom_box = gtk.VBox()
        self.bottom_box.add(self.local_browser)
        
        self.pack_start(top_hbox_align, False, True)
        self.pack_start(self.bottom_box, True, True)
        
        Dispatcher.connect("switch-browser", self.on_dispatcher_switch_browser)
        
    def on_dispatcher_switch_browser(self, obj, index):    
        if index == 0:
            switch_tab(self.bottom_box, self.local_browser)
        elif index == 1:    
            switch_tab(self.bottom_box, self.get_slide_box())
            
            
    def get_slide_box(self):        
        box = gtk.VBox()
        box.pack_start(SlideSwitcher(), True, True)
        return box
            
    def save(self):        
        self.webcasts_browser.save()
        
    def on_top_hbox_realize(self, widget, size):    
        rect = widget.allocation
        self.search_entry.set_size(rect.width - size, 32)
        widget.show_all()
        
    def on_top_hbox_size_allocate(self, widget, rect, size):
        self.search_entry.set_size(rect.width - size, 32)
        widget.show_all()
        
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
