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

import gobject, gtk

import dbus 
import dbus.glib

from helper import SignalCollector
from player import Player
from logger import Logger

from nls import _

PIDGIN_OBJ_PATH = '/im/pidgin/purple/PurpleObject'
PIDGIN_INTERFACE = 'im.pidgin.purple.PurpleInterface2'
PIDGIN_SERVICE = 'im.pidgin.purple.PurpleService'

class PidginStatusPlugin(Logger):
    
    def __init__(self):
        self.__lastsong = None
        self.sbus = dbus.SessionBus()
        self.__connected_to_pidgin = False
        self.__check_pidgin_presence()
        gobject.timeout_add(10000, self.__check_pidgin_presence)
        gtk.quit_add(0,self.reset_status)

    def reset_status(self):
        if not self.__connected_to_pidgin: return
        try: 
            status = self.get_meth("")
        except:
            return
        self.change_meth(status, "")
        self.set_active_meth(status)

    def __check_pidgin_presence(self):
        try: 
            obj = self.sbus.get_object(PIDGIN_SERVICE, PIDGIN_OBJ_PATH)
        except:
            if self.__connected_to_pidgin:
                self.__connected_to_pidgin = False
                self.loginfo("disconnect from pidgin")
        else:
            interface = dbus.Interface(obj, PIDGIN_INTERFACE)
            self.change_meth = interface.__getattr__("PurpleSavedstatusSetMessage")
            self.set_active_meth = interface.__getattr__("PurpleSavedstatusActivate")
            self.get_meth = interface.__getattr__("PurpleSavedstatusGetCurrent")
            if not self.__connected_to_pidgin:
                self.__connected_to_pidgin = True
                self.loginfo("connect to pidgin")
                if self.__lastsong:
                    self.on_new_song(None, self.__lastsong)
        return False

    def on_new_song(self, player, song):
        self.__lastsong = song
        if not song or not self.__connected_to_pidgin: return
        try: 
            status = self.get_meth("")
        except:
            self.__check_pidgin_presence()
            self.on_new_song(player,song)
            return

        title = ""
        artist = ""
        album = ""
        
        if song.get_str("title"):
            title = song.get_str("title")
        if song.get_str("artist"):
            artist = song.get_str("artist")
        if song.get_str("album"):
            album = song.get_str("album")
        
        status_msg = "\xe2\x99\xaa %s: %s (%s) \xe2\x99\xaa" %(artist, title, _("Deepin Music"))
        self.loginfo("Change pidgin status to \"%s\"",status_msg)
        self.change_meth(status, status_msg)
        self.set_active_meth(status)
            
pidgin_status_notification = PidginStatusPlugin()            
            
def enable(dmusic):
    SignalCollector.connect("pidgin", Player, "instant-new-song", pidgin_status_notification.on_new_song)

def disable(dmusic):
    pidgin_status_notification.reset_status()
    SignalCollector.disconnect_all("pidgin")
