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

from widget.playlist import PlaylistUI
from widget.webcast_list import WebcastList
from widget.radio_list import RadioList

from helper import Dispatcher
from nls import _

from widget.tab_box import TabManager
from constant import TAB_LOCAL, TAB_WEBCAST, TAB_RADIO


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
                (_("Library"), self.playlist_ui, TAB_LOCAL),
                (_("Radio"), WebcastList(), TAB_WEBCAST),

                (_("MusicFM"), RadioList(), TAB_RADIO)
                ])
        
        self.tab_box.connect("switch-tab", self.on_tab_box_switch_tab)
        main_align.add(self.tab_box)
        self.add(main_align)
        
        Dispatcher.connect_after("play-song", self.manual_active_tab, TAB_LOCAL)
        Dispatcher.connect_after("play-webcast", self.manual_active_tab, TAB_WEBCAST)
        Dispatcher.connect_after("play-radio", self.manual_active_tab, TAB_RADIO)
        
    def on_tab_box_switch_tab(self, widget, item):    
        Dispatcher.emit("switch-browser", item.tab_type)
        
    def manual_active_tab(self, widget, songs, tab_type):    
        self.tab_box.active_tab(tab_type)
