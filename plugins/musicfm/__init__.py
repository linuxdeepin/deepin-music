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

from radio_browser import RadioBrowser
from radio_list import RadioList
from radio_view import RadioSearchView
from nls import _
from helper import Dispatcher, SignalCollector
from widget.tab_box import  ListTab
from widget.tab_switcher import TabItem

radio_browser = RadioBrowser()
radio_list = RadioList()
radio_list_tab = ListTab(_("MusicFM"), radio_list, radio_browser)
radio_search_view = RadioSearchView(radio_list_tab)
radio_search_tab = TabItem(_("MusicFM"), radio_search_view)

def enable(dmusic):
    SignalCollector.connect("musicfm", Dispatcher, "being-quit", lambda w: radio_list.save())
    radio_browser.start_fetch_channels()    
    Dispatcher.emit("add-source", radio_list_tab)
    Dispatcher.emit("add-search-view", radio_search_tab)
    
def disable(dmusic):    
    SignalCollector.disconnect_all("musicfm")
    Dispatcher.emit("remove-source", radio_list_tab)
    Dispatcher.emit("remove-search-view", radio_search_tab)

