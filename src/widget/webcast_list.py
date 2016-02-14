#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

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
