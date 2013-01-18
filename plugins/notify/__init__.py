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

from helper import SignalCollector
from player import Player
from cover_manager import CoverManager

from dbus_notify import DbusNotify
from nls import _
from utils import threaded

class TrackNotify(object):
    
    def __init__(self):
        self.dbus_notify = DbusNotify("deepin-music-player", "deepin-music-player")
        
    @threaded    
    def on_new_song(self, player, song):    
        icon = CoverManager.get_path_from_song(song)
        self.dbus_notify.set_icon_from_path(icon)
        self.dbus_notify.set_summary(song.get_str("title"))
        body = ""
        if song.get_str("artist"):
            body = "%s: %s\n" % (_("Artist"), song.get_str("artist"))
        if song.get_str("album"):    
            body += "%s: %s\n" % (_("Album"), song.get_str("album"))
        self.dbus_notify.set_body(body)    
        self.dbus_notify.notify()

track_notify = TrackNotify()        

def enable(dmusic):
    SignalCollector.connect("notify", Player, "instant-new-song", track_notify.on_new_song)
    
def disable(dmusic):    
    SignalCollector.disconnect_all("notify")
    