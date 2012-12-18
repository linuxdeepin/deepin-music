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

from widget.song_view import LocalSearchView
from widget.radio_view import RadioSearchView
from widget.webcast_view import WebcastSearchView
from widget.ui_utils import switch_tab, set_widget_gravity
from widget.tab_switcher import TabSwitcher
from nls import _

class GlobalSearch(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        self.set_spacing(10)
        
        self.local_view_page = LocalSearchView()
        self.radio_view_page = RadioSearchView()
        self.webcast_view_page = WebcastSearchView()
        
        self.tab_switcher = TabSwitcher(["本地资源", "电台资源", "广播资源"])
        self.tab_switcher.connect("tab-switch-start", lambda switcher, tab_index: self.switch_result_view(tab_index))
        tab_switcher_align = set_widget_gravity(self.tab_switcher, gravity=(0, 0, 1, 1),
                                                paddings=(10, 0, 0, 0))
        
        self.result_page = gtk.VBox()
        self.result_page.add(self.local_view_page)
        
        self.pack_start(tab_switcher_align, False, True)
        self.pack_start(self.result_page, True, True)
        
    def switch_result_view(self, index):    
        if index == 0:
            switch_tab(self.result_page, self.local_view_page)
        elif index == 1:    
            switch_tab(self.result_page, self.radio_view_page)
        elif index == 2:    
            switch_tab(self.result_page, self.webcast_view_page)
    
    def begin_search(self, keyword):
        self.radio_view_page.start_search_radios(keyword)
        self.local_view_page.start_search_songs(keyword)
        self.webcast_view_page.start_search_webcasts(keyword)
