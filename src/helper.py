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

NEED_RESTORE = True

class EventDispatcher(gobject.GObject):
    SIGNAL = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT))
    SIGNAL_SIMPLE = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))
    SIGNAL_COMP = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT,gobject.TYPE_INT))
    SIGNAL_COMP2 = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT,gobject.TYPE_PYOBJECT))
    SIGNAL_CONFIG = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,gobject.TYPE_STRING,gobject.TYPE_STRING))
    SIGNAL_BASE = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())
    SIGNAL_TUPLE = (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_INT, gobject.TYPE_INT))
    __gsignals__ = {
        "config-changed" : SIGNAL_CONFIG,
        "source-updated" : SIGNAL_SIMPLE,
        "close-lyrics" :  SIGNAL_BASE,
        "show-lyrics" : SIGNAL_BASE,
        "show-playlist" :SIGNAL_SIMPLE,
        "select-source-id" :SIGNAL_SIMPLE,
        "volume" : (gobject.SIGNAL_RUN_LAST,gobject.TYPE_NONE, (gobject.TYPE_FLOAT,)),
        "play-device" :  (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,)),    
        "play-song" : SIGNAL_SIMPLE,
        "add-songs" : SIGNAL_SIMPLE,
        "quit": SIGNAL_BASE,
        "reload-lrc" : SIGNAL_SIMPLE,
        "reload-browser" : SIGNAL_SIMPLE,
        "unlock-lyrics" : SIGNAL_BASE,
        "lock-lyrics" : SIGNAL_BASE,
        "show-desktop-page": SIGNAL_BASE,
        "show-scroll-page" : SIGNAL_BASE,
        "show-setting" : SIGNAL_BASE,
        "show-main-menu" : SIGNAL_TUPLE,
        "window-mode" :  (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,)),    
        "show-job" : SIGNAL_BASE,
        "hide-job" : SIGNAL_BASE,
        "transfor-job" : SIGNAL_SIMPLE,
        "webcast-info" : SIGNAL,
        "play-webcast" : SIGNAL_SIMPLE,
        "add-webcasts" : SIGNAL_SIMPLE,
        "clear-sourcebar-status" : SIGNAL_BASE,
        "new-cd-playlist" : SIGNAL_COMP2,
        "del-cd-playlist" : SIGNAL_SIMPLE,
        "switch-browser" : SIGNAL,
        "play-radio" : SIGNAL_SIMPLE,
        "being-quit" : SIGNAL_BASE,
        "album-changed" : SIGNAL_SIMPLE,
        "change-webcast" : SIGNAL_SIMPLE,
        "add-source" : SIGNAL_SIMPLE,
        "remove-source" : SIGNAL_SIMPLE,
        "switch-source" : SIGNAL_SIMPLE,        
        "add-search-view" : SIGNAL_SIMPLE, 
        "remove-search-view" : SIGNAL_SIMPLE,
        "save-current-list" : SIGNAL_BASE,
        "search-lyrics" : SIGNAL_BASE,
        "dialog-run" : SIGNAL_BASE,
        "dialog-close" : SIGNAL_BASE,
        "download-songs" : SIGNAL_SIMPLE,
        "ready" : SIGNAL_BASE,
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
        
    def lock_lyrics(self):
        self.emit("lock-lyrics")
        
    def show_lyrics(self):    
        self.emit("show-lyrics")
        
    def show_setting(self):    
        self.emit("show-setting")
        
    def show_desktop_page(self):    
        self.emit("show-desktop-page")
        
    def show_scroll_page(self):    
        self.emit("show-scroll-page")
        
    def show_main_menu(self, x, y):    
        self.emit("show-main-menu", x, y)
        
    def change_window_mode(self, status):    
        self.emit("window-mode", status)
        
    def transfor_job(self, job):
        self.emit("transfor-job", job)
        
    def reload_browser(self, infos):    
        self.emit("reload-browser", infos)
        
    def show_jobs(self):    
        self.emit("show-job")
        
    def hide_jobs(self):    
        self.emit("hide_job")
        
    def play_webcast(self, webcast):    
        self.emit("play-webcast", webcast)
        
    def emit_webcast_info(self, parent, key):    
        self.emit("webcast-info", parent, key)
        
    def clear_sourcebar_status(self):    
        self.emit("clear-sourcebar-status")
        
    def new_audiocd_playlist(self, name, songs, udi):    
        self.emit("new-cd-playlist", name,  songs, udi)
        
    def delete_audiocd_playlist(self, udi):    
        self.emit("del-cd-playlist", udi)
        
    def download_songs(self, songs):    
        self.emit("download-songs", songs)
        
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
