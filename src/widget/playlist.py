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
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.paned import HPaned
from dtk.ui.listview import ListView

from library import MediaDB
from widget.ui import SongView
from widget.song_item import SongItem
from widget.list_item import PlaylistItem
from config import config
from player import Player
import utils


class PlaylistUI(gtk.VBox):
    '''Playlist UI.'''
	
    def __init__(self):
        '''Init.'''
        super(PlaylistUI, self).__init__()
        self.list_paned = HPaned(100)
        self.category_list = ListView()
        self.category_list.connect("button-press-item", self.list_button_press)
        
        category_scrolled_window = ScrolledWindow()
        category_scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        category_scrolled_window.add_child(self.category_list)
        
        self.right_box = gtk.HBox()
        self.list_paned.pack1(category_scrolled_window)
        self.list_paned.pack2(self.right_box)
        self.pack_start(self.list_paned, True, True)            
        
        self.current_playlist = None
        self.current_item = None
        
        
        if MediaDB.isloaded():
            self.__on_db_loaded(MediaDB)
        else:    
            MediaDB.connect("loaded", self.__on_db_loaded)
            

        
    def __on_db_loaded(self, db):        
        if not MediaDB.get_playlists():
            MediaDB.create_playlist("local", "[最近播放]")
            MediaDB.create_playlist("local", "[默认列表]")
            
        self.items_dict = {name : PlaylistItem(pl) for name,  pl in MediaDB.get_playlists().iteritems()} 
        init_items = [ item for __, item in  self.items_dict.iteritems()]
        init_items.reverse()
        
        
        self.category_list.add_items(init_items)
        self.current_item = self.items_dict[self.get_current_pname()]
        if self.current_item in self.category_list.items:
            index = self.category_list.items.index(self.current_item)
        else:    
            index = 0
        self.category_list.select_rows.append(index)    
        Player.set_source(self.current_item.song_view)
        self.right_box.pack_start(self.current_item.get_list_widget(), True, True)
        self.list_paned.show_all()
        
        
    def get_current_pname(self):    
        return config.get("playlist", "current_name")
    
    def __add_playlist(self, db, p_type, pls):    
        pass
            
    def __removed_playlist(self, db, p_type, pls):
        pass
        
    def list_button_press(self, widget, item, column, x, y):        
        utils.container_remove_all(self.right_box)
        self.right_box.pack_start(item.get_list_widget(), True, True)
        self.list_paned.show_all()
        
    def save_to_library(self):    
        MediaDB.full_erase_playlists()
        for item in self.category_list.items:
            songs = item.get_songs()
            name = item.title
            MediaDB.create_playlist("local", name, songs)
                    
playlist_ui = PlaylistUI()        
