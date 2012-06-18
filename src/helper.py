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


import gobject

class EventDispatcher(gobject.GObject):
    SIGNAL = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT))
    SIGNAL_SIMPLE = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))
    SIGNAL_COMP = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT,gobject.TYPE_INT))
    SIGNAL_COMP2 = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT))
    SIGNAL_CONFIG = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,gobject.TYPE_STRING,gobject.TYPE_STRING))
    SIGNAL_BASE = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())
    __gsignals__ = {
        "config-changed" : SIGNAL_CONFIG,
        "source-updated" : SIGNAL_SIMPLE,
        "close-lyrics" :  SIGNAL_BASE,
        "show-playlist" :SIGNAL_SIMPLE,
        "select-source-id" :SIGNAL_SIMPLE,
        "volume" : (gobject.SIGNAL_RUN_LAST,gobject.TYPE_NONE, (gobject.TYPE_FLOAT,)),
        "play-device" :  (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,)),    
        "play-song" : SIGNAL_SIMPLE,
        "add-songs" : SIGNAL_SIMPLE,
        "quit": SIGNAL_BASE,
        "reload-lrc" : SIGNAL_SIMPLE,
        "unlock-lyrics" : SIGNAL_BASE,
        }
    
    def __init__(self):
        super(EventDispatcher, self).__init__()
        self.__update_source_id = None
        
    def config_change(self, section, option, value):    
        self.emit("config-changed", section, option, value)
        
    def show_playlists(self, value):    
        self.emit("show-playlist", value)
        
    def close_lyrics(self):    
        self.emit("close-lyrics")
        
    def reload_lrc(self, value):
        self.emit("reload-lrc", value)
        
    def play_and_add_song(self, song):
        self.emit("play-song", song)

    def volume(self, value):        
        self.emit("volume", value)
        
    def add_songs(self, songs):    
        self.emit("add-songs", songs)
        
    def quit(self):    
        self.emit("quit")
        
    def unlock_lyrics(self):    
        self.emit("unlock-lyrics")
        
        
Dispatcher = EventDispatcher()

class SignalContainer(object):    
    
    def __init__(self):
        self.signals_dict = {}
        
    def autoconnect_object(self, obj, signal, func, obj_dest, *args):
        try:
            conns = self.signals_dict[obj]
        except KeyError:    
            conns = self.signals_dict[obj] = []
        conns.append(obj.connect_after(signal, func, obj_dest, *args))    
        
    def autoconnect_after(self, obj, signal, func, *args):    
        try:
            conns = self.signals_dict[obj]
        except KeyError:    
            conns = self.signals_dict[obj] = []
        conns.append(obj.connect_after(signal, func, *args))    
        
    def autoconnect(self, obj, signal, func, *args):    
        try:
            conns = self.signals_dict[obj]
        except KeyError:    
            conns = self.signals_dict[obj] = []
        conns.append(obj.connect(signal, func, *args))    
        
    def autodisconnect_object(self, obj):    
        conns = self.signals_dict.get(obj, None)
        if conns:
            for conn in conns:
                obj.disconnect(conn)
            del self.signals_dict.dict[obj]    
            
    def autodisconnect_all(self, obj):        
        for obj, conns in self.signals_dict.iteritems():
            for conn in conns:
                obj.disconnect(conn)
        self.signals_dict = {}        
        
class _SignalCollector(object):        
    
    def __init__(self):
        self.signals_dict = {}
        
    def connect(self, sid, obj, signal, func, *args):    
        self.signals_dict.setdefault(sid, {})
        try:
            conns = self.signals_dict[sid][obj]
        except KeyError:    
            conns = self.signals_dict[sid][obj] = []
        conns.append(obj.connect(signal, func, *args))    
        
    def disconnect_object(self, sid, obj):    
        if self.signals_dict.has_key(sid):
            conns = self.signals_dict[sid].get(obj, None)
            if conns:
                for conn in conns:
                    obj.disconnect(conn)
                del self.signals_dict[sid][obj]    
                
    def disconnect_all(self, sid):            
        if self.signals_dict.has_key(sid):
            for obj, conns in self.signals_dict[sid].iteritems():
                for conn in conns:
                    obj.disconnect(conn)
            self.signals_dict[sid] = {}        
            
SignalCollector = _SignalCollector()            
                    
                
                
