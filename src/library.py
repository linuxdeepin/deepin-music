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
import time
from threading import Condition
from random import shuffle

from song import Song, TAG_KEYS
from findfile import get_config_file
from logger import Logger

class MissingUriTag(Exception):
    pass

class MediaDatebase(gobject.GObject, Logger):
    '''The Music player media datebase.'''
    __gsignals__ = {
        "loaded" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, gobject.TYPE_PYOBJECT)),
        "quick-changed":(gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, gobject.TYPE_PYOBJECT)),
        "simple-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "added": (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, gobject.TYPE_PYOBJECT)),
        "removed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, gobject.TYPE_PYOBJECT)),
        "playlist-added" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, gobject.TYPE_PYOBJECT)),
        "playlist-removed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, gobject.TYPE_PYOBJECT))}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        
        # reset queued signal.
        self.__reset_queued_signal()
        
        # set condition lock.
        self.__condition = Condition()
        
        # set datebase operation lock.
        self.__db_operation_lock = Condition()
        
        self.__songs = {}
        self.__playlists = {}
        self.__song_types = []
        
        self.__is_loaded = False
        self.__force_check = False
        
    def __reset_queued_signal(self):    
        ''' reset queued signal. '''
        self.__queued_signal = {
            "added" : {},
            "removed" : {},
            "changed" : {},
            "quick-changed" : {},
            "playlist-added" : {},
            "playlist-removed" : {}}
        
    def isloaded(self):    
        return self.__is_loaded
    
    def load(self):
        '''load songs and playlists information for db file.'''
        self.loginfo("Loading library...")
        
        # load the songs.db
        try:
            db_objs = utils.load_db(get_config_file("songs.db"))
        except:    
            self.logexception("Failed to load library")
            db_objs = []
            
        # load the playlist.db    
        try:    
            pls_objs = utils.load_db(get_config_file("playlists.db"))
        except:    
            self.logexception("Failed load playlists")
            pls_objs = []
            
        if db_objs:    
            for obj in db_objs:
                try:
                    stype = obj["song_type"]
                except KeyError:
                    self.logerror("Song with no type found, %s", obj.get("uri"))
                    continue
                
                if stype not in self.__song_types:
                    self.logwarn("Song type %s not exist, for registration", stype)
                    self.register_type(stype)
                    
                if self.__force_check:    
                    s = Song()
                    s.init_from_dict(obj)
                else:    
                    s = Song(obj)
                s.set_type(stype)
                if self.__force_check:
                    s["uri"] = utils.realuri(s.get("uri"))
                if s in self.__hiddens:    
                    self.__hiddens.remove(s)
                    del self.__songs[s.get("uri")]
                if not self.__force_sanity_check or not self.__songs.has_key(s.get("uri")):
                    self.add([s])
                    
                    
                    
                    
                    
            
            
            
            
            
            
    
        
        
        
        
