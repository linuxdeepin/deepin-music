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

try:
    import indicate
except ImportError:
    pass
import dbus

from mpris_adapter import Mpris2Adapter
from mpris_adapter import ORG_MPRIS_MEDIAPLAYER2
from mpris_adapter import ORG_MPRIS_MEDIAPLAYER2_PLAYER
from mpris_adapter import MICROSECOND

from player import Player
from helper import SignalCollector, Dispatcher
from library import MediaDB
from config import config


MPRIS2 = None

def enable(dmusic):
    global MPRIS2
    MPRIS2 = Mpris2Manager(dmusic)
    MPRIS2.acquire()
    MPRIS2.register_events()
    init_indicate()

def disable(dmusic):
    global MPRIS2
    MPRIS2.unregister_events()
    MPRIS2.release()
    

def init_indicate():
    ## for Maverick registration
    try:
        SERVER_INDICATOR = indicate.indicate_server_ref_default()
        SERVER_INDICATOR.set_type('music.dmusic')
        SERVER_INDICATOR.set_desktop_file('/usr/share/applications/deepin-music-player.desktop')
        SERVER_INDICATOR.show()
    except:
        pass

DBUS_OBJECT_NAME = 'org.mpris.MediaPlayer2.dmusic'

class Mpris2Manager(object):
    def __init__(self, dmusic):
        self.dmusic = dmusic
        self.bus = None

    def acquire(self):
        if self.bus:
            self.bus.get_bus().request_name(DBUS_OBJECT_NAME)
        else:
            self.bus = dbus.service.BusName(DBUS_OBJECT_NAME, bus=dbus.SessionBus())
        self.adapter = Mpris2Adapter(self.dmusic, self.bus)
        ### for Natty registration
        self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2, 'DesktopEntry')
        self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2_PLAYER,
                'PlaybackStatus', 'Metadata', 'CanGoNext', 'CanGoPrevious',
                'CanPause', 'CanPlay')

    def register_events(self):
        SignalCollector.connect("mpris2", Player, "instant-new-song", self.on_player_new_song)
        # SignalCollector.connect("mpris2", Player, "new-song", self.on_player_new_song)
        SignalCollector.connect("mpris2", Player, "played", self.on_player_played)
        SignalCollector.connect("mpris2", Player, "paused", self.on_player_paused)
        SignalCollector.connect("mpris2", Player, "stopped", self.on_player_stopped)
        SignalCollector.connect("mpris2", Player, "seeked", self.on_player_seeked)
        SignalCollector.connect("mpris2", MediaDB, "simple-changed", self.on_songs_changed)
        SignalCollector.connect("mpris2", config, "config-changed", self.on_config_changed)
        SignalCollector.connect("mpris2", Dispatcher, "being-quit", self.on_being_quit)
        
        
    def on_being_quit(self, *args):    
        self.release()
        self.unregister_events()

    def release(self):
        if self.adapter is not None:
            self.adapter.remove_from_connection()
        if self.bus is not None:
            self.bus.get_bus().release_name(self.bus.get_name())

    def unregister_events(self):
        SignalCollector.disconnect_all("mpris2")

    def on_player_new_song(self, *args):
        self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2_PLAYER,
                ('PlaybackStatus', 'Playing'), 'Metadata', 'CanGoNext', 'CanGoPrevious',
                'CanPause', 'CanPlay')

    def on_player_stopped(self, *args):
        self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2_PLAYER,
                'PlaybackStatus', 'Metadata', 'CanGoNext', 'CanGoPrevious',
                'CanPause', 'CanPlay')

    def on_player_paused(self, *args):
        self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2_PLAYER,
                'PlaybackStatus', 'CanPause', 'CanPlay')
        
    def on_player_played(self, *args):    
        self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2_PLAYER,
                'PlaybackStatus', 'CanPause', 'CanPlay')

    def on_songs_changed(self, db, songs):
        if Player.song:
            if Player.song in songs:
                self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2_PLAYER, 'Metadata')

    def on_config_changed(self, obj, section, option, value):
        if section == "player" and option == "volume":
            self.adapter.populate(ORG_MPRIS_MEDIAPLAYER2_PLAYER, 'Volume')

    def on_player_seeked(self, player, pos):
        self.adapter.Seeked(pos * MICROSECOND)
