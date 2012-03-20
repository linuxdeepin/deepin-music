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

import gtk
import gobject
import pango
import random
from dtk.ui.listview import ListView

import utils
from config import config
from widget.song_item import SongItem
from player import Player
from findfile import get_config_file
from library import MediaDB
from logger import Logger

class SongView(ListView):
    ''' song view. '''
    def __init__(self, *args):
        super(SongView, self).__init__(*args)
        self.current_item = -1
        
    def get_songs(self):        
        songs = []
        for song_item in self.items:
            songs.append(song_item.get_song())
        return songs    
    
    def is_empty(self):
        return len(self.items) == 0
    
    def get_loop_mode(self):
        config.get("setting", "loop_mode")
        
    def set_loop_mode(self, value):    
        confif.set("setting", "loop_mode", value)
        
    def get_previous_song(self):
        if self.is_empty():
            if config.get("setting", "empty_random") == "true":
                return MediaDB.get_random_song("local")
        else:    
            if config.get("setting", "loop_mode") == "random_mode":
                return self.get_random_song()
            if self.highlight_item != None:
                if self.highlight_item in self.items:
                    current_index = self.items.index(self.highlight_item)
                    prev_index = current_index - 1
                    if prev_index < 0:
                        prev_index = len(self.items) - 1
                    self.highlight_item = self.items[prev_index]    
            else:        
                self.highlight_item = self.items[0]
            return self.highlight_item.get_song()
    
    def get_next_song(self, manual=False):
        
        if self.is_empty():
            if config.getboolean("setting", "empty_random"):
                return MediaDB.get_random_song("local")
        else:    
            if manual:
                if config.get("setting", "loop_mode") != "random_mode":
                    return self.get_manual_song()
                else:
                    return self.get_random_song()
            
            elif config.get("setting", "loop_mode") == "list_mode":
                return self.get_manual_song()
            
            elif config.get("setting", "loop_mode") == "order_mode":            
                if self.highlight_item != None:
                    if self.highlight_item in self.items:
                        current_index = self.items.index(self.highlight_item)
                        next_index = current_index + 1
                        if next_index <= len(self.items) -1:
                            self.highlight_item = self.items[next_index]    
                            return self.highlight_item.get_song()
                return None        
            elif config.get("setting", "loop_mode") == "single_mode":
                if self.highlight_item != None:
                    return self.highlight_item.get_song()
                
            elif config.get("setting", "loop_mode") == "random_mode":    
                self.get_random_song()
                        
    def get_manual_song(self):                    
        if self.highlight_item != None:
            if self.highlight_item in self.items:
                current_index = self.items.index(self.highlight_item)
                next_index = current_index + 1
                if next_index > len(self.items) - 1:
                    next_index = 0
                self.highlight_item = self.items[next_index]    
        else:        
            self.highlight_item = self.items[0]
        return self.highlight_item.get_song()
    
    def get_random_song(self):
        if self.highlight_item in self.items:
            current_index = [self.items.index(self.highlight_item)]
        else:    
            current_index = [-1]
        items_index = set(range(len(self.items)))
        remaining = items_index.difference(current_index)
        self.highlight_item = self.items[random.choice(list(remaining))]
        return self.highlight_item.get_song()

    def add_songs(self, songs, pos=None, sort=False, play=False):    
        '''Add song to songlist.'''
        if songs == None:
            return
        if not isinstance(songs, (list, tuple)):
            songs = [ songs ]
        song_items = [ SongItem(song) for song in songs]
        self.add_items(song_items, pos, sort)
        
        if len(songs) == 1 and play:
            self.highlight_item = SongItems[0]
            gobject.idle_add(Player.play_new, self.highlight_item.get_song())
            
    def add_uris(self, uris, pos=None, sort=True):
        if uris == None:
            return
        if not isinstance(uris, (tuple, list)):
            uris = [ uris ]
        songs = []
        for uri in uris:    
            song = MediaDB.get_song(uri)
            if song:
                songs.append(song)
        self.add_songs(songs, pos, sort)        
        
    def get_current_song(self):        
        return self.highlight_item.get_song()
    
    def random_reorder(self, *args):
        songs = self.get_songs()
        new_songs = []
        self.clear()
        list_index = range(0, len(songs))
        random.shuffle(list_index)
        
        for index in list_index:
            new_songs.append(songs[index])
        self.add_songs(new_songs)    
