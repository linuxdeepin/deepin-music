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

from dtk.ui.tab_window import TabBox
from dtk.ui.new_treeview import TreeView
from webcast_view import WebcastItem

from playlist import PlaylistUI
from helper import Dispatcher
from player import Player

class ListManager(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # playlist
        self.playlist_ui = PlaylistUI()
        
        # webcastlist
        self.webcast_view = self.get_webcast_view()
        
        self.tab_box = TabBox()
        self.tab_box.add_items(
            [("本地音乐", self.playlist_ui),
             ("网络广播", self.webcast_view)
             ]
            )
        
        self.add(self.tab_box)
        
        Dispatcher.connect("play-webcast", self.on_dispatcher_play_webcast)
        
    def get_webcast_view(self):    
        view = TreeView()
        view.set_size_request(324, -1)
        return view
    
    def on_dispatcher_play_webcast(self, obj, webcast):
        self.webcast_view.add_items([WebcastItem(webcast)])
        Player.play_new(webcast)
