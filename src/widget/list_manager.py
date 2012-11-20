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
from widget.playlist import PlaylistUI
from widget.castlist import WebcastList
from widget.radio_view import RadioView

from helper import Dispatcher
from nls import _

from widget.tab_box import TabManager, Tab

class ListManager(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        main_align = gtk.Alignment()
        main_align.set_padding(0, 0, 1, 0)
        main_align.set(1, 1, 1, 1)
        # playlist
        self.playlist_ui = PlaylistUI()
        
        # webcastlist
        self.tab_box = TabManager([
                Tab(_("本地音乐"), self.playlist_ui, 0),
                Tab(_("网络广播"), WebcastList(), 1),
                Tab(_("豆瓣电台"), self.get_radio_box(), 2, True)])
        
        self.tab_box.connect("switch-tab", self.on_tab_box_switch_tab)
        main_align.add(self.tab_box)
        self.add(main_align)
        
    def on_tab_box_switch_tab(self, widget, item):    
        Dispatcher.emit("switch-browser", item.index)
        
    def get_radio_box(self):   
        view = RadioView()
        scrolled_window = ScrolledWindow(0, 0) 
        scrolled_window.add_child(view)
        scrolled_window.set_size_request(303, -1)
        return scrolled_window
