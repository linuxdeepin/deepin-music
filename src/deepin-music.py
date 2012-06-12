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

import sys

if "--help" in sys.argv or "-h" in sys.argv:
    from option_parser import DMusicOptionParser
    DMusicOptionParser()
    sys.exit(0)


import gobject
gobject.threads_init()

import pygtk
pygtk.require("2.0")

import gtk
if gtk.pygtk_version < (2,6) or gtk.gtk_version < (2, 6):
    raise ImportError,"Need GTK > 2.6.0"
gtk.gdk.threads_init()

import pygst
pygst.require("0.10")

import gst
if gst.pygst_version < (0, 10, 1):
    raise ImportError,"Need Gstreamer >= 0.10.1"

import mutagen 
if mutagen.version < (1, 8):
    raise ImportError,"Need mutagen >= 1.8"

from findfile import get_cache_file
PIDFILE = get_cache_file("dmusic.pid")

from widget.skin import app_theme
from config import config
from library import MediaDB
from player import Player
from pinyin import TransforDB

from widget.main import DeepinMusic
from option_parser import DMusicOptionParser



class DeepinMusicApp(object):
    app_instance = None
    app_ready = False
    db_ready = False
    
    def __init__(self):
        self.option = DMusicOptionParser()
        self.option.run_preload()
        config.load()
        MediaDB.connect("loaded", self.on_db_loaded)
        MediaDB.load()
        TransforDB.load()
        self.app_instance = DeepinMusic()
        self.app_instance.connect("ready", self.on_ready_cb)
        gtk.main()
    
    def on_ready_cb(self, app):
        self.app_ready = True
        self.post_start()
        
    def on_db_loaded(self, *args, **kwargs):    
        self.db_ready = True
        self.post_start()
        
    def post_start(self):    
        if self.db_ready and self.app_ready:
            Player.load()

if __name__ == "__main__":
    DeepinMusicApp()
