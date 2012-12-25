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

from helper import Dispatcher
from widget.tab_box import TabManager
from widget.local_tab import local_search_tab

class ListManager(gtk.VBox):
    def __init__(self):
        gtk.VBox.__init__(self)
        
        main_align = gtk.Alignment()
        main_align.set_padding(0, 0, 1, 0)
        main_align.set(1, 1, 1, 1)
        
        # playlist
        self.tab_box = TabManager([local_search_tab])
        self.tab_box.connect("switch-tab", self.on_tab_box_switch_tab)
        main_align.add(self.tab_box)
        self.add(main_align)
        
        Dispatcher.connect("add-source", self.on_dispatcher_add_source)
        Dispatcher.connect("remove-source", self.on_dispatcher_remove_source)
        Dispatcher.connect("switch-source", self.on_dispatcher_switch_source)
        
    def on_tab_box_switch_tab(self, widget, item):    
        Dispatcher.emit("switch-browser", item, True)
        
    def manual_active_tab(self, widget, songs, tab_type):    
        self.tab_box.active_tab(tab_type)
        
    def on_dispatcher_add_source(self, widget, data):    
        self.tab_box.add_items([data], False)

    def on_dispatcher_remove_source(self, widget, data):    
        self.tab_box.remove_items([data])
        
    def on_dispatcher_switch_source(self, widget, data):    
        self.tab_box.active_item(data)
        Dispatcher.emit("switch-browser", data, False)
