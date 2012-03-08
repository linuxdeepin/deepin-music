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
import utils

gobject.threads_init()
AUTOSAVE_TIMEOUT = 1000 * 60 * 5 # 5min
SIGNAL_DB_QUERY_FIRED = 50


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
        '''Init.'''
        gobject.GObject.__init__(self)
        
        # set condition lock.
        self.__condition = Condition()
        
        # set datebase operation lock.
        self.__db_operation_lock = Condition()
        
        # songs
        self.__songs = {}
        self.__song_types = []
        self.__songs_by_type = {}
        
        # playlist
        self.__playlists = {}
        self.__playlist_types = []
        
        # init constant
        self.__is_loaded = False
        self.__force_check = False
        self.__save_song_type = ["local", "xiami"]
        self.is_busy = False
        
        # Init queued signal.
        self.__reset_queued_signal()
        
    def set_busy(self):    
        self.is_busy = True
        
    def set_force_check(self):    
        self.__force_check = True
        
    def __reset_queued_signal(self):
        '''Reset queued signal'''
        self.__queued_signal = {
            "added" : {},
            "removed" : {},
            "changed" : {},
            "quick-changed" : {},
            "playlist-added" : {},
            "playlist-removed" : {}
            }
        
    def __fire_queued_signal(self):    
        '''Fire queued signal'''
        self.__condition.acquire()
        try:
            for song_type, songs in self.__queued_signal["removed"].iteritems():
                self.emit("removed", song_type, songs)
            for song_type, songs in self.__queued_signal["added"].iteritems():
                self.emit("added", song_type, songs)
            for song_type, infos in self.__queued_signal["changed"].iteritems():   
                self.emit("changed", song_type, infos) #todo
                self.emit("simple-changed", [ i[0] for i in infos ]) # get song
            for song_type, infos in self.__queued_signal["quick-changed"].iteritems():    
                self.emit("quick-changed", song_type, infos)
            for pl_type, playlists in self.__queued_signal["playlist-added"].iteritems():    
                self.emit("playlist-added", pl_type, playlists)
            for pl_type, playlists in self.__queued_signal["playlist-removed"].iteritems():    
                self.emit("playlist-removed", pl_type, playlists)
        except:        
            self.logexception("Failed fire queued signal")
        self.__reset_queued_signal()    
        self.__condition.release()
        
    def create_song(self, tags, song_type, read_from_file=False):    
        '''Create song'''
        self.is_busy = True
        try:
            uri = tags["uri"]
        except KeyError:    
            raise MissingUriTag()
        tags["uri"] = utils.realuri(uri)
        return self.__create_song(tags, song_type, read_from_file=read_from_file)
    
    def __create_song(self, tags, song_type, read_from_file=False):
        '''create song'''
        s = Song()
        s.init_from_dict(tags)
        s.set_type(song_type)
        if read_from_file:
            s.read_from_file()
        self.add(s)    
        return s
    
    def add(self, songs):
        '''Add song'''
        if not isinstance(songs, (tuple, list, set)):
            songs = [ songs ]
        for song in songs:    
            self.__add_cb(song)
            
    def __add_cb(self, song):        
        '''add song cb'''
        self.is_busy = True
        song_type = song.get_type()
        uri = song.get("uri")
        self.__db_operation_lock.acquire()
        self.__songs[uri] = song
        self.__songs_by_type.setdefault(song_type, set())
        self.__songs_by_type[song_type].add(song)
        self.loginfo("add %s by %s", uri, song_type)
        
        # add type, song to added signal.
        self.__condition.acquire()
        self.__queued_signal["added"].setdefault(song_type, set())
        self.__queued_signal["added"][song_type].add(song)
        self.__condition.release()
        self.__db_operation_lock.release()
        
    def remove(self, songs):    
        '''Remove song'''
        self.is_busy = True
        if not isinstance(songs, (tuple, list, set)):
            songs = [ songs ]
        for song in songs:    
            self.__remove_cb(song)
            
    def __remove_cb(self, song):        
        song_type = song.get_type()
        self.__db_operation_lock.acquire()
        try:
            del self.__songs[song.get("uri")]
        except KeyError:    
            pass
        self.__songs_by_type[song_type].remove(song)
        self.loginfo("remove the %s success", song)
        
        # add signal.
        self.__condition.acquire()
        self.__queued_signal["removed"].setdefault(song_type, set())
        self.__queued_signal["removed"][song_type].add(song)
        self.__condition.release()
        self.__db_operation_lock.release()
        
    def set_property(self, song, keys_values, write_to_file=False, use_quick_update=False):    
        '''Set song property'''
        if not song: return False
        ret = True
        self.is_busy = True
        song_type = song.get_type()
        old_keys_values = {}
        mod_keys = keys_values.keys()
        
        [ old_keys_values.update({key:song.get(key)}) for key in song.keys() if key in mod_keys ]
        [ song.update({key:value}) for key, value in keys_values.items() if value is not None ]
        for key in [ key for key, value in keys_values.items() if value is None ]:
            del song[key]
        if write_to_file:    
            ret = song.write_to_file()
            
        # update signal.
        self.__condition.acquire()    
        if use_quick_update:    
            self.__queued_signal["quick-changed"].setdefault(song_type, [])
            self.__queued_signal["quick-changed"][song_type].append((song, old_keys_values))
        else:    
            self.__queued_signal["changed"].setdefault(song_type, [])
            self.__queued_signal["changed"][song_type].append((song, old_keys_values))
        self.__condition.release()    
        return ret
    
    def del_property(self, song, keys):        
        '''delete song property'''
        if not song: return False
        self.is_busy = True
        
        if not isinstance(keys, (list, tuple)):
            keys = [ keys ]
        song_type = song.get_type()
        old_keys_values = {}
        [ old_keys_values.update({key:song.get_sortable(key)}) for key in song.keys() if key in keys]
        for key in keys: del song[key]
        self.loginfo("from %s delete property %s", song, keys)
        self.__condition.acquire()
        self.__queued_signal["changed"].setdefault(song_type, [])
        self.__queued_signal["changed"][song_type].append((song, old_keys_values))
        self.__condition.release()
        
    def reload_song_from_file(self, song):    
        s = Song()
        s.init_from_dict({"uri":song.get("uri")})
        s["uri"] = utils.realuri(s.get("uri"))
        s.read_from_file()
        new_tags = {}
        for key in TAG_KEYS.values() + ["#size", "#mtime", "ctime"]:
            if s.has_key(key) and not song.has_key(key):
                new_tags[key] = s.get(key)
            elif not s.has_key(key) and song.has_key(key):    
                new_tags[key] = song.get(key)
            elif s.get(key) != song.get(key):
                new_tags[key] = s.get(key)
        self.set_property(song, new_tags)        
        
    def get_or_create_song(self, tags, song_type, read_from_file=False):    
        self.is_busy = True
        try:
            uri = utils.realuri(tags["uri"])
        except KeyError:    
            raise MissingUriTag()
        try:
            s = self.__songs[uri]
            if s.get_type() != song_type:
                raise KeyError
        except KeyError:    
            s = self.__create_song(tags, song_type, read_from_file)
        else:    
            if read_from_file:
                s.read_from_file()
            self.set_property(s, tags)    
        return s    
    
    def has_uri(self, uri):
        uri = utils.realuri(uri)
        return self.__songs.has_key(uri)
    
    def full_erase(self, song_type):
        for song in list(self.__songs_by_type[song_type]):
            self.remove(song)
            
    def get_all_uris(self):        
        return self.__songs.keys()
    
    def get_song(self, uri):
        uri = utils.realuri(uri)
        try:
            return self.__songs[uri]
        except KeyError:
            if not uri:
                return None
            if uri.startswith("file://"):
                return self.get_or_create_song({"uri":uri}, "unknown_local", read_from_file=True)
            else:
                return self.get_or_create_song({"uri":uri}, "unknown", read_from_file=True)
            
    def get_songs(self, song_type):        
        try:
            return list(set(self.__songs_by_type[song_type]))
        except KeyError:
            self.logwarn("get_songs: type %s unknown return empty set()", song_type)
            return []
        
    def get_songs_from_uris(self, uris):    
        return [ self.__songs[utils.realuri(uri)] for uri in uris ]
    
    def create_playlist(self, pl_type, name, items=[]):
        '''create playlist with name '''
        if items and isinstance(items[0], Song):
            items = [ song.get("uri") for song in items ]
        pl = Playlist(pl_type, name, items)    
        self.add_playlist(pl_type, pl)
        return pl
    
    def add_playlist(self, pl_type, pl):
        pl_name = pl.get_name()
        self.__playlists[pl_type].add(pl)
        self.__condition.acquire()
        self.__queued_signal["playlist-added"].setdefault(pl_type, [])
        self.__queued_signal["playlist-added"][pl_type].append(pl)
        self.is_busy = True
        self.__condition.release()
        
    def del_playlist(self, pl_type, pl):    
        self.__playlists[pl_type].discard(pl)
        self.__condition.acquire()
        self.__queued_signal["playlist-removed"].setdefault(pl_type, [])
        self.__queued_signal["playlist-removed"][pl_name].append(pl)
        self.is_busy = True
        self.__condition.release()
        
    def get_random_song(self, song_type="local"):        
        songs = list(self.__songs_by_type[song_type])
        shuffle(songs)
        if songs:
            return songs[0]
        else:
            return None
        
    def register_type(self, name):    
        if name not in self.__song_types:
            self.__song_types.append(name)
            self.__songs_by_type[name] = set()
            
    def unregister_type(self, name):        
        if name in self.__song_types:
            self.__db_operation_lock.acquire()
            [ self.__songs.remove(song.get("uri"))  for song in self.__songs_by_type[name] ]
            self.__db_operation_lock.release()
            self.__song_types.remove(name)
            del self.__songs_by_type[name]
        else:    
            self.logwarn("Unregister a unknown type %s", name)
            
    def register_playlist_type(self, name):        
        if name not in self.__playlist_types:
            self.__playlist_types.append(name)
            self.__playlists[name] = set()
            
    def unregister_playlist_type(self, name):        
        if name in self.__playlist_types:
            self.__playlist_types.remove(name)
            del self.__playlists[name]
        else:    
            self.logwarn("W:MediaDB:Unregister a unknown playlist type %s", name)
            
            
    def load(self):        
        '''load songs and playlists from db file'''
        self.loginfo("Loading library...")
        
        # Load songs
        try:
            db_objs = utils.load_db(get_config_file("songs.db"))
        except:    
            self.logexception("Failed to load library")
            db_objs = []
            
        # Load playlists    
        try:    
            pls_objs = utils.load_db(get_config_file("playlists.db"))
        except:    
            self.logexception("Failed load playlists")
            pls_objs = []
            
        if db_objs:    
            for obj in db_objs:
                try:
                    song_type = obj["song_type"]
                except KeyError:    
                    self.logerror("Song with no type found, %s", obj.get("uri"))
                    
                if song_type not in self.__song_types:    
                    self.logwarn("Song type %s not exist, for registration", song_type)
                    self.register_type(song_type)
                    
                if self.__force_check:    
                    s = Song()
                    s.init_from_dict(obj)
                    s["uri"] = utils.realuri(s.get("uri"))
                else:    
                    s = Song(obj)
                s.set_type(song_type)    
                if not self.__force_check or not self.__songs.has_key(s.get("uri")):
                    self.add(s)
                    
        if pls_objs:            
            for pl_obj in pls_objs:            
                name, infos = pl_obj
                if self.__force_check:
                    infos = map(utils.realuri, infos)
                self.create_playlist("local", name, infos)    
        if self.__force_check:        
            self.save()
        self.is_busy = False    
        
        # fire signal
        self.__reset_queued_signal()
        gobject.timeout_add(AUTOSAVE_TIMEOUT, self.async_save)
        gobject.timeout_add(SIGNAL_DB_QUERY_FIRED * 20, self.__fire_queued_signal)
        
        self.loginfo("%s songs loaded in %s types", len(self.__songs), len(self.__song_types))
        self.loginfo("Finish loading library")
        gobject.idle_add(self.__delay_post_load)
        
        
    def __delay_post_load(self):    
        self.__is_loaded = True
        self.emit("loaded")
    
    def save(self):    
        if not self.is_busy:
            return True
        
        # Quickly copy obj before pickle it
        self.__db_operation_lock.acquire()
        songs = self.__songs.values()
        playlists = self.__playlists["local"].copy()
        self.__db_operation_lock.release()
        objs = [ song.get_dict() for song in songs if song.get_type() in self.__save_song_type ]
        
        # save
        utils.save_db(objs, get_config_file("songs.db"))
        utils.save_db([pl.get_pickle_obj() for pl in playlists], get_config_file("playlists.db"))
        self.loginfo("%d songs saved and %d playlists saved", len(objs), len(playlists))
        self.is_busy = False
        
    @utils.threaded
    def async_save(self):
        self.save()
        
        
class Playlist(gobject.GObject, Logger):        
    __gsignals__ = {
        "added" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "removed" :(gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "update" :(gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())
        }
    
    def __init__(self, pl_type, name, uris=[]):
        gobject.GObject.__init__(self)
        self.name = name
        self.__uris = uris
        self.__type = pl_type
        
        MediaDB.connect("removed", self.__on_db_remove)
        
    def get_pickle_obj(self):    
        return (self.name, self.__uris)
    
    def get_type(self):
        return self.__type
    
    def set_name(self, name):
        self.name = name
        
    def get_name(self):    
        return self.name
    
    def get_songs(self):
        return MediaDB.get_songs_from_uris(self.__uris)
                
    def update(self):       
        self.emit("update")

    def insert(self, song, pos):
        self.__uris.insert(pos, song.get("uri"))
        self.update()
        
    def extend(self, songs):    
        self.__uris.extend( [ song.get(uri) for song in songs ] )
        self.update()
        
    def extend_insert(self, songs, pos):    
        for song in songs:
            self.__uris.insert(pos, song.get("uri"))
            pos += 1
        self.update()    
            
    def remove(self, song):    
        while True:
            try:
                self.__uris.remove(song.get("uri"))
            except:    
                break
        self.update()    
        
    def __on_db_remove(self, db, song_type, songs):    
        [ self.remove(s) for s in songs if s.get("uri") in self.__uris ]
        
    def remove_position(self, pos):    
        del self.__uris[pos]
        self.update()
        
    def remove_positions(self, positions):    
        positions.sort(reverse=True)
        for pos in positions:
            del self.__uris[pos]
        self.update()    
        
    def append(self, song):    
        self.extend([song])
        

MediaDB = MediaDatebase()        
MediaDB.register_type("local")
MediaDB.register_type("xiami")
MediaDB.register_type("unknown")
MediaDB.register_type("unknown_local")

MediaDB.register_playlist_type("local")

