#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
#
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou ShaoHui <houshao55@gmail.com>
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

import utils
from player import Player

OBJECT_PATH = "/org/gnome/listen"
SERVICE_NAME = "org.gnome.Listen"

try: 
    import dbus #@UnusedImport
    import dbus.service
    #Try connection du message bus
    dbus_version = getattr(dbus, 'version', (0, 0, 0))
    if dbus_version >= (0, 41, 0) and dbus_version < (0, 80, 0):
        dbus.SessionBus()
        import dbus.glib  #@UnusedImport
    elif dbus_version >= (0, 80, 0):
        from dbus.mainloop.glib import DBusGMainLoop
        DBusGMainLoop(set_as_default=True)
        dbus.SessionBus()

except: dbus_imported = False
else: dbus_imported = True

if not dbus_imported:
    class DeepinMusicDBus(object):
        def __init__(self, object_path=OBJECT_PATH):
            pass
    print "No dbus support"
else:    
    class DeepinMusicDBus(dbus.service.Object):
        def __init__(self, object_path=OBJECT_PATH):
            self.bus = dbus.SessionBus()
            bus_name = dbus.service.BusName(SERVICE_NAME, bus=self.bus)
            dbus.service.Object.__init__(self, bus_name, object_path)
            
        @dbus.service.method(SERVICE_NAME)    
        def hello(self):
            win = utils.get_main_window()
            if win.get_property("visible"):
                win.present()
            return "Running"
        
        @dbus.service.method(SERVICE_NAME)
        def focus(self):
            win = utils.get_main_window()
            win.grab_focus()
            if win.get_property("visible"):
                win.present()
            return "Running"    
            
        @dbus.service.method(SERVICE_NAME)
        def play(self, uris):
            print "DBUS: play()"
            win = utils.get_main_window()
            # win.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.WATCH))
            # async_parse_uris(uris, True, True,
                    # self.win.playlist_ui.playlist.play_uris, pos=None, sort=True)
            return "Successful command "

        @dbus.service.method(SERVICE_NAME)
        def play_device(self, value):
            # Dispatcher.play_device(value)
            return "Successful command "

        @dbus.service.method(SERVICE_NAME)
        def enqueue(self, uris):
            print "DBUS: enqueue()"
            win = utils.get_main_window()
            # win.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.WATCH))    
            # async_parse_uris(uris, True, True,
                    # self.win.playlist_ui.playlist.add_uris, pos=None, sort=True)
            return "Successful command "
    
    
        @dbus.service.method(SERVICE_NAME)
        def quit(self):
            # Dispatcher.quit()
            return "Successful command "
        
        @dbus.service.method(SERVICE_NAME)
        def previous(self):
            Player.previous()
            return "Successful command "
    
        @dbus.service.method(SERVICE_NAME)
        def forward(self):
            Player.forward()
            return "Successful command "
    
        @dbus.service.method(SERVICE_NAME)
        def rewind(self):
            Player.rewind()
            return "Successful command "
    
        @dbus.service.method(SERVICE_NAME)
        def next(self):
            Player.next()
            return "Successful command "
    
        @dbus.service.method(SERVICE_NAME)
        def play_pause(self):
            Player.playpause()
            return "Successful command "

        @dbus.service.method(SERVICE_NAME)
        def stop(self):
           Player.stop()
           return "Successful command "
    
        @dbus.service.method(SERVICE_NAME)
        def volume(self, value):
            try: value = float(value)
            except:
                return _("Fail to set volume")
            else:
                # Dispatcher.volume(value)
                return "Successful command "
    
        @dbus.service.method(SERVICE_NAME)
        def current_playing(self):
            if not Player.is_paused():
                song = Player.song    
                return str(song.get_str("title") + " - (" + song.get_str("album") + " - " + song.get_str("artist") + ")")
            else:
                return ""

        @dbus.service.method(SERVICE_NAME)
        def get_title(self):
            if not Player.is_paused():
                song = Player.song
                return song.get_str("title")
            else:
                return None

        @dbus.service.method(SERVICE_NAME)
        def get_artist(self):
            if not Player.is_paused():
                song = Player.song
                return song.get_str("artist")
            else:
                return ""
    
        @dbus.service.method(SERVICE_NAME)
        def get_uri(self):
            if not Player.is_paused():
                song = Player.song
                return song.get_str("uri")
            else:
                return ""

        @dbus.service.method(SERVICE_NAME)
        def get_album(self):
            if not Player.is_paused():
                song = Player.song
                return song.get_str("album")
            else:
                return ""

        @dbus.service.method(SERVICE_NAME)
        def current_position(self):
            return Player.get_position()
            
        @dbus.service.method(SERVICE_NAME)
        def current_song_length(self):
            return Player.get_length()

        @dbus.service.method(SERVICE_NAME)
        def get_cover_path(self):
            if not Player.is_paused() and Player.song:
                return ""
                # return CoverManager.get_cover_path(Player.song)
            else:
                return ""

        @dbus.service.method(SERVICE_NAME)
        def playing(self):
            return not Player.is_paused()

        @dbus.service.method(SERVICE_NAME)
        def dump_gstplayer_state(self):
            return Player.bin.dump_state()
    