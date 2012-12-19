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
from dtk.ui.scrolled_window import ScrolledWindow

from widget.webcast_view import WebcastView
from widget.ui import EmptyListItem
from widget.ui_utils import switch_tab
from constant import EMPTY_WEBCAST_ITEM

class WebcastList(gtk.VBox):
    def __init__(self):
        gtk.VBox.__init__(self)
        self.webcast_sw, self.webcast_view = self.get_webcast_view()
        self.empty_webcast_box = EmptyListItem(self.webcast_view.on_drag_data_received,
                                               EMPTY_WEBCAST_ITEM)
        self.add(self.empty_webcast_box)
        
        self.webcast_view.connect("empty-items", self.on_webcast_view_empty_items)
        self.webcast_view.connect("begin-add-items", self.on_webcast_view_begin_add)
        
        self.webcast_view.load()
        
    def on_webcast_view_empty_items(self, widget):    
        switch_tab(self, self.empty_webcast_box)
        
    def on_webcast_view_begin_add(self, widget):    
        switch_tab(self, self.webcast_sw)
        
    def get_webcast_view(self):    
        view = WebcastView()
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.add_child(view)
        return scrolled_window, view
    
    def save(self):
        self.webcast_view.save()
