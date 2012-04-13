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
    
    __gsignals__ = {
        "config-changed" : SIGNAL_CONFIG,
        "source-updated" : SIGNAL_SIMPLE,
        "show-lyrics" :  SIGNAL_SIMPLE,
        "show-playlist" :SIGNAL_SIMPLE,
        "select-source-id" :SIGNAL_SIMPLE,
        "volume" : (gobject.SIGNAL_RUN_LAST,gobject.TYPE_NONE, (gobject.TYPE_FLOAT,)),
        "play-device" :  (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,)),    
        "quit": SIGNAL_SIMPLE,
        }
    
    def __init__(self):
        super(EventDispatcher, self).__init__()
        self.__update_source_id = None
        
    def config_change(self, section, option, value):    
        self.emit("config-changed", selction, option, value)
        
    def show_playlists(self, value):    
        self.emit("show-playlist", vaule)
        
    def show_lyrics(self, value):    
        self.emit("show-lyrics", value)

    def quit(self):    
        self.emit("quit", None)
        
Dispatcher = EventDispatcher()

    