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

from dtk.ui.utils import (cairo_disable_antialias,
                          color_hex_to_cairo, propagate_expose)
from widget.local_browser import SimpleBrowser
from widget.webcasts_browser import WebcastsBrowser
from widget.radio_browser import RadioBrowser
from widget.global_search import GlobalSearch
from widget.ui import SearchBox
from widget.ui_utils import switch_tab, draw_line, draw_alpha_mask
from helper import Dispatcher
from constant import TAB_LOCAL, TAB_WEBCAST, TAB_RADIO


class BrowserMananger(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Search Widgets at top.
        self.search_box = SearchBox()
        self.search_box.connect("search", self.on_searchbox_search)
        search_box_align = gtk.Alignment()
        search_box_align.connect("expose-event", self.on_top_hbox_expose)        
        search_box_align.set_padding(0, 1, 1, 0)
        search_box_align.set(0, 0, 1, 1)
        search_box_align.add(self.search_box)
        
        # Bottom widgets and is switchable.
        self.local_browser = SimpleBrowser()
        self.webcasts_browser = WebcastsBrowser()
        self.radio_browser = RadioBrowser()
        self.bottom_box = gtk.VBox()
        self.global_search = GlobalSearch()
        
        self.bottom_box.add(self.local_browser)
        # self.bottom_box.add(self.global_search)
        self.bottom_box_align = gtk.Alignment()
        self.bottom_box_align.set_padding(0, 0, 1, 2)
        self.bottom_box_align.set(1, 1, 1, 1)
        self.bottom_box_align.add(self.bottom_box)
        
        self.pack_start(search_box_align, False, True)
        self.pack_start(self.bottom_box_align, True, True)
        
        self.connect("expose-event", self.on_expose_event)
        Dispatcher.connect("switch-browser", self.on_dispatcher_switch_browser)
        
        
    def on_searchbox_search(self, widget, keyword):    
        self.global_search.begin_search(keyword)    
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x, rect.y, rect.width - 2, rect.height ,"layoutRight")
        
        draw_line(cr, (rect.x + 1, rect.y), 
                  (rect.x + 1, rect.y + rect.height), "#b0b0b0")
        return False
        
    def on_dispatcher_switch_browser(self, obj, tab_type):    
        if tab_type == TAB_LOCAL:
            switch_tab(self.bottom_box, self.local_browser)
        elif tab_type == TAB_WEBCAST:    
            switch_tab(self.bottom_box, self.webcasts_browser)
        elif tab_type == TAB_RADIO:    
            switch_tab(self.bottom_box, self.radio_browser)
            
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
