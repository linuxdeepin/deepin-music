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

import os
import gtk
import gobject

from dtk.ui.thread_pool import MissionThreadPool, MissionThread
from cover_query import multi_query_artist_engine, multi_query_album_engine
from library import DBQuery, MediaDB
from helper import SignalContainer
from xdg_support import get_cache_file
import utils


def get_cover_save_path(name):
    return get_cache_file("cover/%s.jpg" % name)

def cleanup_cover(old_path):    
    if not os.path.exists(old_path):    
        return False
    try:
        pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(old_path, 300, 300)
    except gobject.GError:    
        return False
    else:
        # Check cover is not a big black image
        str_pixbuf = pixbuf.get_pixels()
        if str_pixbuf.count("\x00") > len(str_pixbuf)/2 or str_pixbuf.count("\xff") > len(str_pixbuf)/2 : 
            return False
        else:
            if os.path.exists(old_path): os.unlink(old_path)
            pixbuf.save(old_path, "jpeg", {"quality":"85"})
            del pixbuf  
            
            # Change property album to update UI
            # MediaDB.set_property(song, {"album" : song.get("album")})
            return True

class FetchArtistCover(MissionThread):
    def __init__(self, artist_name):
        MissionThread.__init__(self)
        self.artist_name = artist_name
        
    def start_mission(self):    
        if self.artist_name:
            query_result = multi_query_artist_engine(self.artist_name)
            if query_result:
                if utils.direct_download(query_result, get_cover_save_path(self.artist_name)):
                    cleanup_cover(get_cover_save_path(self.artist_name))
                
    def get_mission_result(self):    
        return None
    
class FetchAlbumCover(MissionThread):    
    
    def __init__(self, infos):
        MissionThread.__init__(self)
        self.artist_name, self.album_name = infos
        
    def start_mission(self):    
        if self.artist_name or self.album_name:
            query_result = multi_query_album_engine(self.artist_name, self.album_name)
            if query_result:
                if utils.direct_download(query_result, self.get_save_path()):
                    cleanup_cover(self.get_save_path())
                    
    def get_save_path(self):                
        return get_cover_save_path("%s-%s" % (self.artist_name, self.album_name))
    
    def get_mission_result(self):
        return None

class FetchManager(SignalContainer):    
    
    def __init__(self, db_query):
        SignalContainer.__init__(self)
        self.__db_query = db_query
        
    def connect_to_db(self):    
        self.autoconnect(self.__db_query, "full-update", self.__full_update)
        self.__db_query.set_query("")
    
    def __full_update(self, db_query):    
        self.start_artist_missions()
        self.start_album_missions()
    
    def get_infos_from_db(self, tag, values=None):
        genres = []
        artists = []
        extened = False
        infos_dict =  self.__db_query.get_info(tag, genres, artists, values, extened)
        
        if tag == "artist":
            all_artist_keys = infos_dict.keys()
            if all_artist_keys:
                return [artist for artist in all_artist_keys if not os.path.exists(get_cover_save_path(artist))]
            return []
            
        elif tag == "album":
            results = []
            for key, values in infos_dict.iteritems():
                artist_name = values[0].replace("/", "")
                album_name = key.replace("/", "")
                if not os.path.exists(get_cover_save_path("%s-%s" % (artist_name, album_name))):
                    results.append((artist_name, album_name))
            return results        
        else:
            return []
        
    def start_artist_missions(self):    
        artists = self.get_infos_from_db("artist")
        if artists:
            artist_missions = []
            for artist_name in artists:
                artist_missions.append(FetchArtistCover(artist_name.replace("/", "")))
            MissionThreadPool(artist_missions, 5).start()    
            
    def start_album_missions(self):        
        albums = self.get_infos_from_db("album")
        if albums:
            album_missions = []
            for album_info in albums:
                album_missions.append(FetchAlbumCover(album_info))
            MissionThreadPool(album_missions, 5).start()    
                
class SimpleFetchManager(FetchManager):            
    def __init__(self):
        FetchManager.__init__(self, DBQuery("local"))
        
        if MediaDB.isloaded():
            self.__on_db_loaded(MediaDB)
        else:    
            self.autoconnect(MediaDB, "loaded", self.__on_db_loaded)
            
    def __on_db_loaded(self, db):        
        self.connect_to_db()
