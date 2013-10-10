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
import helper

from helper import Dispatcher
from widget.tab_box import TabManager
from widget.local_tab import local_search_tab
from config import config

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
        self.current_title = config.get("listmanager", "source", "")
        self.current_source = None
        
        Dispatcher.connect("add-source", self.on_dispatcher_add_source)
        Dispatcher.connect("remove-source", self.on_dispatcher_remove_source)
        Dispatcher.connect("switch-source", self.on_dispatcher_switch_source)
        if helper.NEED_RESTORE:
            Dispatcher.connect("ready", self.on_dispatcher_ready)
        
    def on_dispatcher_ready(self, widget):    
        if self.current_source:
            self.switch_source(self.current_source, True)
            restore_status = getattr(self.current_source.list_widget, "restore_status", None)
            if restore_status and callable(restore_status):
                # gobject.idle_add(restore_status)
                restore_status()
                
        else:    
            try:
                gobject.idle_add(local_search_tab.list_widget.restore_status)
            except Exception, e:
                print e
        
    def on_tab_box_switch_tab(self, widget, item):    
        try:
            config.set("listmanager", "source", item.title)
            config.write()
        except: pass    
        
        Dispatcher.emit("switch-browser", item, True)
        
    def manual_active_tab(self, widget, songs, tab_type):    
        self.tab_box.active_tab(tab_type)
        
    def on_dispatcher_add_source(self, widget, data):    
        if data.title == self.current_title:
            self.current_source = data
        self.tab_box.add_items([data], False)

    def on_dispatcher_remove_source(self, widget, data):    
        self.tab_box.remove_items([data])
        
    def on_dispatcher_switch_source(self, widget, data):    
        self.switch_source(data)

    def switch_source(self, data, switched=False):    
        self.tab_box.active_item(data)
        Dispatcher.emit("switch-browser", data, switched)
        
    def switch_to_local(self):    
        self.switch_source(local_search_tab, True)
