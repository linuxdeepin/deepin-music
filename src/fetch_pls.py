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

import gobject
import threading

from dtk.ui.threads import post_gui

from utils import (get_uris_from_asx,
                   get_uris_from_pls, get_uris_from_m3u,
                   get_uris_from_xspf, get_mime_type, get_scheme)

from logger import Logger

class FetchPls(gobject.GObject, Logger):
    
    __gsignals__ = { "completed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, 
                                  (gobject.TYPE_PYOBJECT, gobject.TYPE_PYOBJECT))}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        
        
    def run(self):
        self.async_fetch()
            
    def start_fetch(self, song):        
        if get_scheme(song.get("uri")) != "file":
            t = threading.Thread(target=self.async_fetch, args=((song,)))
            t.setDaemon(True)
            t.start()
        else:    
            self.sync_fetch(song)

    def sync_fetch(self, song):    
        uri = song.get("uri")
        
        uris = None
        mime_type = get_mime_type(uri)
        if mime_type == "audio/x-scpls":
            uris = get_uris_from_pls(uri)
        elif mime_type == "audio/x-mpegurl":
            uris = get_uris_from_m3u(uri)
        elif mime_type == "video/x-ms-asf":
            uris = get_uris_from_asx(uri)
        elif mime_type == "application/xspf+xml":
            uris = get_uris_from_xspf(uri)
            
        if uris:    
            self.loginfo("%s choosen in %s", uris[0], uri)
            uri = uris[0]
        else:
            self.loginfo("no playable uri found in %s", uri)
            uri = None
            
        self.emit("completed", song, uri)    
        
    def async_fetch(self, song):    
        uri = song.get("uri")
        ntry = 2
        uris = None
        mime_type = get_mime_type(uri)
        while not uris:
            if mime_type == "audio/x-scpls":
                uris = get_uris_from_pls(uri)
            elif mime_type == "audio/x-mpegurl":
                uris = get_uris_from_m3u(uri)
            elif mime_type == "video/x-ms-asf":
                uris = get_uris_from_asx(uri)
            elif mime_type == "application/xspf+xml":
                uris = get_uris_from_xspf(uri)
            ntry += 1
            if ntry > 3: break

        # TODO: Improve multiple webradio url
        if uris:
            self.loginfo("%s choosen in %s", uris[0], uri)
            uri = uris[0]
        else:
            self.loginfo("no playable uri found in %s", uri)
            uri = None
        self.async_emit(song, uri)    
        
    @post_gui    
    def async_emit(self, song, uri):    
        self.emit("completed", song, uri)            

fetch_radio = FetchPls()        
