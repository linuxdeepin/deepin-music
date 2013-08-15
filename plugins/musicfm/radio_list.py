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

from radio_view import RadioView
from widget.ui import EmptyListItem
from widget.ui_utils import switch_tab
from constant import EMPTY_RADIO_ITEM

class RadioList(gtk.VBox):
    def __init__(self):
        gtk.VBox.__init__(self)
        self.radio_view = RadioView()
        self.empty_radio_box = EmptyListItem(self.radio_view.on_drag_data_received,
                                               EMPTY_RADIO_ITEM)
        self.add(self.empty_radio_box)
        
        self.radio_view.connect("empty-items", self.on_radio_view_empty_items)
        self.radio_view.connect("begin-add-items", self.on_radio_view_begin_add)
        
        self.radio_view.load()
        
    def on_radio_view_empty_items(self, widget):    
        switch_tab(self, self.empty_radio_box)
        
    def on_radio_view_begin_add(self, widget):    
        switch_tab(self, self.radio_view)
        
    def save(self):    
        self.radio_view.save()

    restore_status = property(lambda self: self.radio_view.restore_status)    
