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

import dbus #@UnusedImport
import dbus.glib #@UnusedImport

from helper import SignalCollector
from player import Player

from logger import Logger


GAJIM_OBJ_PATH = '/org/gajim/dbus/RemoteObject'
GAJIM_INTERFACE = 'org.gajim.dbus.RemoteInterface'
GAJIM_SERVICE = 'org.gajim.dbus'


class GajimStatusPlugin(Logger):
    
    def __init__(self):
        self.__lastsong = None
        self.sbus = dbus.SessionBus()
        
        self.__connected_to_gajim = False
        self.__check_gajim_presence()

        gobject.timeout_add(10000, self.__check_gajim_presence)

        gtk.quit_add(0,self.reset_status)

    def reset_status(self):
        if not self.__connected_to_gajim: return
        try: self.change_meth("")
        except: pass

    def __check_gajim_presence(self):
        try: 
            obj = self.sbus.get_object(GAJIM_SERVICE, GAJIM_OBJ_PATH)
        except:
            if self.__connected_to_gajim:
                self.__connected_to_gajim = False
                self.loginfo("disconnect from gajim")
        else:
            interface = dbus.Interface(obj, GAJIM_INTERFACE)
            self.__change_meth = interface.__getattr__("change_status")
            self.get_meth = interface.__getattr__("get_status")
            self.list_accounts = interface.__getattr__("list_accounts")
            if not self.__connected_to_gajim:
                self.__connected_to_gajim = True
                self.loginfo("connect to gajim")
                if self.__lastsong:
                    self.on_new_song(None, self.__lastsong)
        return False

    # Changes the status message for each account separately, avoiding changing
    # status of accounts. Therefore, it avoids Gajim of trying to connect not
    # connected accounts.
    # This should probably be fixed in Gajim's code by adding something like
    # change_status_message() rather than here. See the following bug report for
    # followups: http://trac.gajim.org/ticket/5039
    def change_meth(self, status_msg):
        for account in self.list_accounts():
            self.__change_meth(self.get_meth(account), status_msg, account)

    def on_new_song(self, player, song):
        self.__lastsong = song
        if not song or not self.__connected_to_gajim: return

        title = ""
        artist = ""
        album = ""
        
        if song.get_str("title"):
            title = song.get_str("title")
        if song.get_str("artist"):
            artist = song.get_str("artist")
        if song.get_str("album"):
            album = song.get_str("album")

        status_msg = "\xe2\x99\xaa %s: %s (%s) \xe2\x99\xaa" %(artist, title, album)
        self.loginfo("Change gajim status to \"%s\"",status_msg)
        try:
            self.change_meth(status_msg)
        except Exception, ex:
            self.loginfo("Status update failed: %s", str(ex))
            self.__check_gajim_presence()
            self.on_new_song(player,song)
            
gajim_status_notification = GajimStatusPlugin()            
            
def enable(dmusic):
    SignalCollector.connect("gajim", Player, "instant-new-song", gajim_status_notification.on_new_song)

def disable(dmusic):
    gajim_status_notification.reset_status()
    SignalCollector.disconnect_all("gajim")
