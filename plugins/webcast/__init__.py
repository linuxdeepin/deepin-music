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


from nls import _
from helper import Dispatcher, SignalCollector
from widget.tab_box import  ListTab
from widget.webcast_list import WebcastList
from widget.webcasts_browser import WebcastsBrowser

webcast_list = WebcastList()
webcast_browser = WebcastsBrowser()
webcast_list_tab = ListTab(_("Radio"), webcast_list, webcast_browser)

def _save_db():
    webcast_list.save()
    webcast_browser.save()

def enable(dmusic):
    SignalCollector.connect("webcast", Dispatcher, "being-quit", lambda w: _save_db())
    Dispatcher.emit("add-source", webcast_list_tab)
    
def disable(dmusic):    
    SignalCollector.disconnect_all("webcast")
    Dispatcher.emit("remove-source", webcast_list_tab)
