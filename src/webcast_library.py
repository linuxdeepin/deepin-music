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
import gobject
from threading import Condition
from contextlib import contextmanager 

from logger import Logger
from song import Song
from xdg_support import get_config_file

import utils


AUTOSAVE_TIMEOUT = 1000 * 60 # 1min
SIGNAL_DB_QUERY_FIRED = 50
DEFAULT_DB = os.path.join((os.path.dirname(os.path.realpath(__file__))), "data", "all_webcasts.db")

class WebcastDatebase(gobject.GObject, Logger):
    
    __gsignals__ = {
        "loaded" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "quick-changed":(gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "added": (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "removed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        }
        
    def __init__(self):
        gobject.GObject.__init__(self)
            
        # set condition lock.
        self.__condition = Condition()
        
        # set datebase operation lock.
        self.__db_operation_lock = Condition()
        
        # Manager webcasts.
        self.__hiddens = set()
        self.__songs = {}
        self.__type = "webcast"
        
        # init params.
        self.__is_loaded = False
        self.__dirty = False
        self.__reset_queued_signal()
        
        self.__user_save_db = get_config_file("all_webcasts.db")
        self.__default_db = DEFAULT_DB
        
    @contextmanager
    def keep_operation(self):
        self.__db_operation_lock.acquire()
        try:
            yield
        except Exception, e:    
            self.logwarn("keep operation lock failed %s", e)
            
        else:    
            self.__db_operation_lock.release()
            
    @contextmanager
    def keep_signal(self):
        self.__condition.acquire()
        try:
            yield
        except Exception, e:    
            self.logwarn("keep stream lock failed %s", e)
            
        else:    
            self.__condition.release()
            
        
    def set_dirty(self):    
        self.__dirty = True
        
    def __reset_queued_signal(self):    
        self.__queued_signal = {
            "added" : set(),
            "removed" : set(),
            "changed" : list(),
            "quick-changed" : list(),
            }
        
    def __fire_queued_signal(self):    
        
        with self.keep_signal():
            try:
                if self.__queued_signal["added"]:
                    self.emit("added", self.__queued_signal["added"])
                elif self.__queued_signal["removed"]:    
                    self.emit("removed", self.__queued_signal["removed"])
                elif self.__queued_signal["changed"]:
                    self.emit("changed", self.__queued_signal["changed"])
                elif self.__queued_signal["quick-changed"]:    
                    self.emit("quick-changed", self.__queued_signal["quick-changed"])
            except:        
                self.logexception("Failed fire queued signal.")
                
            self.__reset_queued_signal()    
        return True    
                    
    def create_song(self, tags):    
        
        # Set dirty flag.
        self.set_dirty()
        
        s = Song()
        s.init_from_dict(tags)
        s.set_type(self.__type)
        self.add(s)
        
        return s
    
    def add(self, songs):
        if not isinstance(songs, (tuple, list, set)):
            songs = [ songs ]
            
        for song in songs:    
            self.__add_cb(song)
            
    def __add_cb(self, song):        
        
        # Set dirty flag
        self.set_dirty()
            
        uri = song.get("uri")
        
        with self.keep_operation():
            self.__songs[uri] = song
            # self.logdebug("add webcast %s ", uri)
            
            with self.keep_signal():
                self.__queued_signal["added"].add(song)
                
    def remove(self, songs):            
        
        # Set dirty flag.
        self.set_dirty()
                
        if not isinstance(songs, (tuple, list, set)):
            songs = [ songs ]
            
        for song in songs:    
            self.__remove_cb(song)
            
            
    def __remove_cb(self, song):        
        
        with self.keep_operation():
            try:
                del self.__songs[song.get("uri")]
            except KeyError:    
                pass
            
            self.logdebug("remove the %s success", song)
            
            with self.keep_signal():
                self.__queued_signal["removed"].add(song)
                
    def set_property(self, song, key_values, use_quick_update=False, emit_update=True):            
        
        ret = True
        
        # set dirty flag
        self.set_dirty()
        
        old_keys_values = {}
        mod_keys = key_values.keys()
        
        # Get will modify key's values.
        [ old_keys_values.update({key:song.get(key)}) for key in song.keys() if key in mod_keys ]
        
        # update modify key-valus.
        [ song.update({key:value}) for key, value in key_values.items() if value is not None ]
        
        for key in [ key for key, value in key_values.items() if value is None ]:
            try:
                del song[key]
            except KeyError:    
                pass
            
        if emit_update:    
            with self.keep_signal():    
                if use_quick_update:
                    self.__queued_signal["quick-changed"].append((song, old_keys_values, key_values))
                else:    
                    self.__queued_signal["changed"].append((song, old_keys_values, key_values))
                
        return ret        
    
    
    def del_property(self, song, keys):
        
        if not song: return False
        
        # set dirty flag
        self.set_dirty()
        
        if not isinstance(keys, (list, tuple, set)):
            keys = [ keys ]
            
        old_keys_values = {}    
        [ old_keys_values.update({key:song.get(key)}) for key in song.keys() if key in keys ]
        
        for key in keys:
            try:
                del song[key]
            except KeyError:    
                continue
            
        self.logdebug("from %s delete property %s", song, keys)
        
        with self.keep_signal():
            self.__queued_signal["changed"].append((song, old_keys_values, {}))
                
            
    def get_or_create_song(self, tags):        
        
        # set dirty flags
        try:
            uri = tags["uri"]
        except KeyError:    
            return None
        
        try:
            s = self.__songs[uri]
        except KeyError:    
            s = self.create_song(tags)
        else:    
            self.set_property(s, tags)
        return s    
    
    def get_all_uris(self):
        return self.__songs.keys()
    
    def get_all_songs(self):
        return self.__songs.values()
        
    def get_song(self, uri):
        try:
            return self.__songs[uri]
        except KeyError:
            return self.get_or_create_song({"uri":uri})
        
    def save(self):    
        if not self.__dirty:
            return True
        
        songs = []
        with self.keep_operation():
            songs = self.__songs.values()
            
        objs = [ song.get_dict() for song in songs if song not in self.__hiddens ]    
        utils.save_db(objs, get_config_file(self.__user_save_db))
        self.__dirty = False
        
    @utils.threaded    
    def asyc_save(self):
        self.save()
        
    def load(self):    
        
        save_flag = False
        
        try:
            db_objs = utils.load_db(self.__user_save_db)
        except:    
            db_objs = None

            
        if db_objs is None:    
            save_flag = True
            self.logexception("Faild load user db, will to load default db")
            try:
                db_objs = utils.load_db(self.__default_db)
            except:    
                db_objs = []
                
        if db_objs:        
            for obj in db_objs:
                s = Song(obj)
                s.set_type(self.__type)
                if not self.__songs.has_key(s.get("uri")):
                    # self.logdebug("load webcast %s" % s)
                    self.add(s)
                    
        if save_flag:            
            self.set_dirty()
            self.asyc_save()
            
        self.__dirty = False            

        # fire signal
        self.__reset_queued_signal()
        gobject.timeout_add(AUTOSAVE_TIMEOUT, self.asyc_save)
        gobject.timeout_add(SIGNAL_DB_QUERY_FIRED * 20, self.__fire_queued_signal)
        
        gobject.idle_add(self.__delay_post_load)
        
        
    def __delay_post_load(self):    
        self.__is_loaded = True
        self.emit("loaded")
        
    def isloaded(self):    
        return self.__is_loaded

WebcastDB = WebcastDatebase()
    
class WebcastQuery(gobject.GObject, Logger):    
    
    
    __gsignals__ = {
        "full-update"  : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "added" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "removed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "update-songs" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        }
    
    def __init__(self):
        gobject.GObject.__init__(self)
        
        self.tree = {}
        
        self.multiple_category = {"region" : "location", "region_en" : "country",
                                  "genre" : "genres", "genre_en" : "genres", "foreign" : "country"}
        
        
        WebcastDB.connect("added", self.on_db_added)
        WebcastDB.connect("removed", self.on_db_removed)
        WebcastDB.connect("changed", self.on_db_changed)
        WebcastDB.connect("quick-changed", self.on_db_changed, True)
        
        self.__condition_signal = Condition()
        self.__condition_query = Condition()
        
        self.__query_id = 0
        self.__reset_signal_queue()
        gobject.timeout_add(SIGNAL_DB_QUERY_FIRED * 20 * 2, self.__fire_queued_signal)
        
        
    @contextmanager    
    def keep_query(self):
        self.__condition_query.acquire()
        try:
            yield
        except Exception, e:    
            self.logwarn("keep operation lock failed %s", e)
            
        else:    
            self.__condition_query.release()
            
    @contextmanager    
    def keep_signal(self):
        self.__condition_signal.acquire()
        try:
            yield
        except Exception, e:    
            self.logwarn("keep operation lock failed %s", e)
            
        else:    
            self.__condition_signal.release()
            
    def __reset_signal_queue(self):
        self.__signal_to_fire = {
            "added" : set(),
            "removed" : set(),
            "update-songs" : set(),
            }
        
    def __fire_queued_signal(self):    
        with self.keep_signal():
            try:
                if self.__signal_to_fire["added"]:
                    self.emit("added", self.__signal_to_fire["added"])
                if self.__signal_to_fire["removed"]:
                    self.emit("removed", self.__signal_to_fire["removed"])
                if self.__signal_to_fire["update-songs"]:
                    self.emit("update-songs", self.__signal_to_fire["update-songs"])
            except:        
                self.logexception("Failed fire queued signal")
                
            self.__reset_signal_queue()    
            
        return True    
            
    def get_all_songs(self):    
        return WebcastDB.get_all_songs()
    
    def __get_info(self, song):
        categorys = song.get("categorys", set())
        genres = song.get("genres", set())
        country = song.get("country", "")
        location = song.get("location", "")
        return (categorys, genres, country, location)
    
    def empty(self):    
        self.__tree = {}
    
        
    @utils.threaded
    def set_query(self):
        myid = 0
        with self.keep_query():
            self.__query_id += 1
            myid = self.__query_id
            
        self.empty()    
        [ self.__add_cache(song) for song in self.get_all_songs() ]
        
        with self.keep_query():
            if myid == self.__query_id:
                def fire():
                    self.emit("full-update")
                gobject.idle_add(fire)    

    
    def __add_cache(self, song):
        
        categorys, genres, country, location = self.__get_info(song)
        
        for category in categorys:
            self.__tree.setdefault(category, ({}, set()))
            self.__tree[category][1].add(song)
            
            if category in self.multiple_category.keys():
                child_key = self.multiple_category[category]
                
                if child_key == "location":
                    if location:
                        self.__tree[category][0].setdefault(location, set())
                        self.__tree[category][0][location].add(song)
                    
                elif child_key == "country":    
                    if country:
                        self.__tree[category][0].setdefault(country, set())
                        self.__tree[category][0][country].add(song)
    
                elif child_key == "genres":    
                    for genre in genres:
                        if not genre:
                            continue
                        self.__tree[category][0].setdefault(genre, set())
                        self.__tree[category][0][genre].add(song)
                    
    def __delete_cache(self, song, old_values=False):                    
        if old_values:
            categorys, genres, country, location = old_values
        else:    
            categorys, genres, country, location = self.__get_info(song)
        
        for category in categorys:
            try:
                self.__tree[category][1].remove(song)
            except (KeyError, ValueError):
                pass
            
            if category in self.multiple_category.keys():
                child_key = self.multiple_category[category]
                
                if child_key == "location":
                    self.__tree[category][0][location].remove(song)
                    if len(self.__tree[category][0][location]) == 0:
                        del self.__tree[category][0][location]
                    
                elif child_key == "country":    
                    self.__tree[category][0][country].remove(song)
                    if len(self.__tree[category][0][country]) == 0:
                        del self.__tree[category][0][country]
                    
    
                elif child_key == "genres":    
                    for genre in genres:
                        self.__tree[category][0][genre].remove(song)
                        if len(self.__tree[category][0][genre]) == 0:
                            del self.__tree[category][0][genre]
                        
    def on_db_added(self, db, songs):                    
        
        for song in songs:
            self.__add_cache(song)
            
            with self.keep_signal():
                self.__signal_to_fire["added-songs"].add(song)
                
    def on_db_removed(self, db, songs):            
        
        for song in songs:
            self.__delete_cache(song)
            
            with self.keep_signal():
                self.__signal_to_fire["removed-songs"].add(song)
            
    def on_db_changed(self, db, infos, use_quick_update=False):
        
        for song, old_keys_values, new_key_values in infos:
            if old_keys_values.has_key("genres") \
                    or old_keys_values.has_key("location") \
                    or old_keys_values.has_key("country"):
                
                use_quick_update = False
                
            if not use_quick_update:    
                use_quick_update = True
                for tag, old_value in old_keys_values.iteritems():
                    new_value = song.get(tag)
                    if old_value != new_value:    
                        use_quick_update = False
                        break
                    
                    
            will_removed = False        
            if not old_keys_values.has_key("categorys"): categorys = song.get("categorys")
            else: categorys = song.get("categorys")
            
            if not old_keys_values.has_key("genres"): 
                genres = song.get("genres")
            else:
                genres = old_keys_values.get("genres")
                will_removed = True
            
            if not old_keys_values.has_key("country"): 
                country = song.get("country")
            else:
                country = old_keys_values.get("country")
                will_removed = True
            
            if not old_keys_values.has_key("location"): 
                location = song.get("location")
            else:
                location = old_keys_values.get("location")
                will_removed = True
            
                
            for category in categorys:
                if not category: continue
                if category in self.multiple_category.keys():
                    if will_removed:
                        if song in self.__tree[category][1]:
                            try:            
                                self.__delete_cache(song, (categorys, genres, country, location))
                            except:    
                                pass
                            
                            self.__add_cache(song)                            
                            
            
            with self.keep_signal():
                self.__signal_to_fire["update-songs"].add(song)
                
                 
    def get_info(self, category, child_category=None):                    
        my_data = self.__tree[category]
        if child_category is None:
            return my_data[0].keys(), len(my_data[1])
        else:
            return ([], len(my_data[0][child_category]))
        
    def get_songs(self, category, child_category=None):    
        my_data = self.__tree[category]
        if child_category is None:
            return my_data[1]
        else:
            if child_category not in my_data[0].keys():
                return my_data[1]
            else:
                return my_data[0][child_category]
        
    def get_categorys(self):    
        return self.__tree.keys()
    
    def get_composite_categorys(self):
        filter_categorys = ["foreign", "region_en", "region",
                            "genre", "genre_en"]
        return filter(lambda key: key not in filter_categorys, self.get_categorys())
