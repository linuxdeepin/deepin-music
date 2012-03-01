#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 Deepin, Inc.
#               2011 Hou Shaohui
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
from time import time, sleep
import gst
import gst.interfaces
import sys

from config import config
from logger import Logger
from player.fadebin import PlayerBin

DEBUG = False

class DeepinPlayer(gobject.GObject, Logger):
    __gsignals__ {
        "play-end" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "new-song" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "instant-new-song" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "paused"   : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "played"   : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "stopped"  : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "seeked"   : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())
}
    
    def __init__(self):
        gobject.GObject.__init__(self):
        self.song = None        # current song
        self.__playlist = None  # playlist
        self.__need_load_prefs = True # TODO
        self.__current_stream_seeked = False # TODO
        self.__next_alreay_called = False
        self.__emit_signal_new_song_id = None
        
        self.stop_after_this_track = False
        self.__current_song_reported = False
        self.__current_duration = None
        
        self.bin = PlayerBin()
        self.bin.connect("eos",   self.__on_eos)
        self.bin.connect("error", self.__on_error)
        self.bin.connect("tags-found", self.__on_tag)
        self.bin.connect("tick", self.__on_tick)
        self.bin.connect("playing-stream", self.__on_playing)
        
        
        
        



