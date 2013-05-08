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

import os
import gobject
from threading import Condition
from random import shuffle

from song import Song, TAG_KEYS
from xdg_support import get_config_file
from parse import Query
from logger import Logger
from nls import _
from cue_parser import read_cuesheet, CueException
import utils


AUTOSAVE_TIMEOUT = 1000 * 60 # 1min
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
        self.__hiddens = set()        
        self.__song_types_capability = {}
        self.__songs = {}
        self.__song_types = []
        self.__songs_by_type = {}
        
        # playlist
        self.__playlists = {}
        self.__playlist_types = []
        
        # init constant
        self.__is_loaded = False
        self.__force_check = False
        self.__save_song_type = ["local", "cue", "unknown"]
        self.__dirty = False
        
        # Init queued signal.
        self.__reset_queued_signal()
        
    def set_dirty(self):    
        self.__dirty = True
        
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
        return True
    
    def request(self, string, song_type="local"):
        try:
            filter_func = Query(string).search
        except Query.error:    
            self.logdebug("Request: Query error %s", string)
            return None
        else:
            if not self.__songs_by_type.has_key(song_type):
                self.logdebug("Request: type %s not exist", song_type)
                return None
            return [ song for song in self.__songs_by_type[song_type] if filter_func(song) ]
        
    def create_song(self, tags, song_type, read_from_file=False):    
        '''Create song'''
        self.set_dirty()
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
        self.set_dirty()
        song_type = song.get_type()
        uri = song.get("uri")
        self.__db_operation_lock.acquire()
        self.__songs[uri] = song
        self.__songs_by_type.setdefault(song_type, set())
        self.__songs_by_type[song_type].add(song)
        self.logdebug("add %s by %s", uri, song_type)
        
        # add type, song to added signal.
        self.__condition.acquire()
        self.__queued_signal["added"].setdefault(song_type, set())
        self.__queued_signal["added"][song_type].add(song)
        self.__condition.release()
        self.__db_operation_lock.release()
        
    def remove(self, songs):    
        '''Remove song'''
        self.set_dirty()
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
        self.logdebug("remove the %s success", song)
        
        # add signal.
        self.__condition.acquire()
        self.__queued_signal["removed"].setdefault(song_type, set())
        self.__queued_signal["removed"][song_type].add(song)
        self.__condition.release()
        self.__db_operation_lock.release()
        
    def set_property(self, song, keys_values, write_to_file=False, use_quick_update=False):    
        '''Set song property'''
        if song.get_type() in ["cue", "audiocd"]:
            return
        if not song: return False
        ret = True
        self.set_dirty()
        song_type = song.get_type()
        old_keys_values = {}
        mod_keys = keys_values.keys()
        
        [ old_keys_values.update({key:song.get_str(key)}) for key in song.keys() if key in mod_keys ]
        [ song.update({key:value}) for key, value in keys_values.items() if value is not None ]
        for key in [ key for key, value in keys_values.items() if value is None ]:
            try:
                del song[key]
            except KeyError:    
                pass
            
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
        self.set_dirty(), 
        
        if not isinstance(keys, (list, tuple)):
            keys = [ keys ]
        song_type = song.get_type()
        old_keys_values = {}
        [ old_keys_values.update({key:song.get_sortable(key)}) for key in song.keys() if key in keys]
        for key in keys: del song[key]
        self.logdebug("from %s delete property %s", song, keys)
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
        
    def get_songs_by_uri(self, uri):    
        if not uri: return
        songs = []
        uri_scheme = utils.get_scheme(uri)
        if uri_scheme == "file":
            path = utils.get_path_from_uri(uri)
            prefix = os.path.splitext(path)[0]
            cue_file = "%s.%s" % (prefix, "cue")
            if os.path.exists(cue_file):
                try:
                    cuesheet = read_cuesheet(path, cue_file)
                except CueException, e:    
                    print e
                    song = self.get_or_create_song({"uri":uri}, "local", read_from_file=True)
                    if song: return [song]
                    else:
                        return []
                else:
                    for tag in cuesheet.get_tags():
                        s = self.get_or_create_song(tag, "cue", read_from_file=False)
                        songs.append(s)
                    return songs    
                
            song = self.get_or_create_song({"uri":uri}, "local", read_from_file=True)
            if song: return [ song ]
            else:
                return []
            
        else:    
            song = self.get_or_create_song({"uri": uri}, "unknown", read_from_file=False)
            if song:
                return [ song ]
            return []
        
    def get_or_create_song(self, tags, song_type, read_from_file=False):    
        self.set_dirty()
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
                return self.get_or_create_song({"uri":uri}, "local", read_from_file=True)
            else:
                return self.get_or_create_song({"uri":uri}, "unknown", read_from_file=True)
            
    def get_songs_by_type(self, song_type):        
        if not isinstance(song_type, (tuple, list)):
            song_type = [ song_type ]
            
        songs = []    
        
        try:
            for s_type in song_type:
                songs.extend(list(set(self.__songs_by_type[s_type])))
            return songs    
        except KeyError:
            self.logwarn("get_songs: type %s unknown return empty set()", song_type)
            return songs
        
    def get_songs_from_uris(self, uris):    
        return [ self.get_song(uri) for uri in uris if uri ]
    
    def create_playlist(self, pl_type, name, items=[]):
        '''create playlist with name '''
        if items and isinstance(items[0], Song):
            items = [ song.get("uri") for song in items ]
        pl = Playlist(pl_type, name, items)    
        self.add_playlist(pl_type, pl)
        return pl
    
    def add_playlist(self, pl_type, pl):
        self.__playlists[pl_type].append(pl)
        self.__condition.acquire()
        self.__queued_signal["playlist-added"].setdefault(pl_type, [])
        self.__queued_signal["playlist-added"][pl_type].append(pl)
        self.set_dirty()
        self.__condition.release()
        
    def get_playlists(self, pl_type="local"):   
        return self.__playlists[pl_type]
    
    def full_erase_playlists(self, pl_type="local"):
        del self.__playlists[pl_type][:]
        
    def del_playlist(self, pl_type, pl):    
        self.__playlists[pl_type].remove(pl)
        self.__condition.acquire()
        self.__queued_signal["playlist-removed"].setdefault(pl_type, [])
        self.__queued_signal["playlist-removed"][pl_type].append(pl)
        self.set_dirty()
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
            self.__playlists[name] = []
            
    def unregister_playlist_type(self, name):        
        if name in self.__playlist_types:
            self.__playlist_types.remove(name)
            del self.__playlists[name]
        else:    
            self.logwarn("W:MediaDB:Unregister a unknown playlist type %s", name)
            
            
    def load(self):        
        '''load songs and playlists from db file'''
        self.logdebug("Loading library...")
        
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
        self.__dirty = False    
        
        # fire signal
        self.__reset_queued_signal()
        gobject.timeout_add(AUTOSAVE_TIMEOUT, self.interval_async_save)
        gobject.timeout_add(SIGNAL_DB_QUERY_FIRED * 20, self.__fire_queued_signal)
        
        self.logdebug("%s songs loaded in %s types", len(self.__songs), len(self.__song_types))
        self.logdebug("Finish loading library")
        gobject.idle_add(self.__delay_post_load)
        
        
    def __delay_post_load(self):    
        self.__is_loaded = True
        self.emit("loaded")
        
    def isloaded(self):    
        return self.__is_loaded
    
    def save(self):    
        if not self.__dirty:
            return True
        
        # Quickly copy obj before pickle it
        self.__db_operation_lock.acquire()
        songs = self.__songs.values()
        playlists = self.__playlists["local"][:]
        self.__db_operation_lock.release()
        objs = [ song.get_dict() for song in songs if song.get_type() in self.__save_song_type ]
        
        # save
        utils.save_db(objs, get_config_file("songs.db"))
        utils.save_db([pl.get_pickle_obj() for pl in playlists ], get_config_file("playlists.db"))
        self.logdebug("%d songs saved and %d playlists saved", len(objs), len(playlists))
        self.__dirty = False
        
    def interval_async_save(self):    
        self.async_save()
        return True
        
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
        self.__uris.extend( [ song.get("uri") for song in songs ] )
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
        
    def clear(self):    
        del self.__uris[:]
        
    def append(self, song):    
        self.extend([song])
        
        
class CDPlaylist(object):        
    def __init__(self, pl_type, name, songs):
        self.name = name
        self.songs = songs
        self.__type = pl_type
        
    def get_type(self):
        return self.__type
    
    def set_name(self, name):
        self.name = name
        
    def get_name(self):    
        return self.name
    
    def get_songs(self):
        return self.songs
    
class DBQuery(gobject.GObject, Logger):        
    __gsignals__ = {
        "full-update"  : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "update-tag" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, gobject.TYPE_PYOBJECT)),
        "added" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "removed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "quick-update" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))
        }
    def __init__(self, song_type):
        gobject.GObject.__init__(self)
        self.__tree = ({}, set())
        if not isinstance(song_type, (tuple, list)):
            song_type = [ song_type ]
        self.__type = song_type
        self.__query_func = None
        self.__query_string = ""
        self.__cache_func_song_tuple = None
        self.__cache_song_tuple = {}
        self.__attr_songs = {}
        
        MediaDB.connect("added", self.db_entry_added)
        MediaDB.connect("removed", self.db_entry_removed)
        MediaDB.connect("changed", self.db_entry_changed)
        MediaDB.connect("quick-changed", self.db_entry_changed, True)
        
        self.__condition = Condition()
        self.__condition_query = Condition()
        self.__query_id = 0
        self.__reset_signal_queue()
        gobject.timeout_add(SIGNAL_DB_QUERY_FIRED * 20 * 2, self.__fire_queued_signal)
        
    def get_type(self):   
        return self.__type
    
    def __reset_signal_queue(self):
        self.__signal_to_fire = {
            "added-songs" : set(),
            "removed-songs" : set(),
            "update-genre" : set(),
            "update-artist" : set(),
            "update-album" : set(),
            "quick-update-songs" : set()
            }
        
    def __fire_queued_signal(self):    
        self.__condition.acquire()
        try:
            if self.__signal_to_fire["update-genre"]:
                self.emit("update-tag", "genre", self.__signal_to_fire["update-genre"])
            if self.__signal_to_fire["update-artist"]:    
                self.emit("update-tag", "artist", self.__signal_to_fire["update-artist"])
            if self.__signal_to_fire["update-album"]:    
                self.emit("update-tag", "album", self.__signal_to_fire["update-album"])
            if self.__signal_to_fire["removed-songs"]:    
                self.emit("removed", self.__signal_to_fire["removed-songs"])
            if self.__signal_to_fire["added-songs"]:    
                self.emit("added", self.__signal_to_fire['added-songs'])
            if self.__signal_to_fire["quick-update-songs"]:    
                self.emit("quick-update", self.__signal_to_fire["quick-update-songs"])
        except:        
            self.logexception("Failed fire queued signal")
            
        self.__reset_signal_queue()    
        self.__condition.release()
        return True
    
    def get_all_songs(self):
        return MediaDB.get_songs_by_type(self.__type)
    
    def __filter(self, song, query_func=None):
        if query_func is None:
            query_func = self.__query_func
        if query_func is None:    
            return True
        else:
            query_func(song)
            
    @utils.threaded        
    def set_query(self, string=""):
        self.__condition_query.acquire()
        self.__query_id += 1
        myid = self.__query_id
        self.__condition_query.release()
        
        if not isinstance(string, unicode):
            string = string.decode("utf-8")
        string = string.strip()    
        
        self.logdebug("Begin query %s", string)
        
        if string:
            try:
                query_func = Query(string).search
            except Query.error:    
                return
            self.empty()
            [ self.__add_cache(song) for song in self.get_all_songs() if self.__filter(song, query_func)]
        else:    
            query_func = None
            self.empty()
            [ self.__add_cache(song) for song in self.get_all_songs()]
            
        self.__condition_query.acquire()    
        if myid == self.__query_id:
            self.__query_func = query_func
            self.__query_string = string
            def fire():
                self.emit("full-update")
            gobject.idle_add(fire)    
        self.__condition_query.release()    
        
    def empty(self):    
        self.__tree = ({}, set())
        
    def db_entry_added(self, db, song_type, songs):    
        if song_type not in self.__type: return
        
        for song in songs:
            if self.__filter(song):
                self.__add_cache(song)
                
                genre, artist, album = self.__get_info(song)
                
                self.__condition.acquire()
                self.__signal_to_fire["update-genre"].add(genre)
                self.__signal_to_fire["update-artist"].add(artist)
                self.__signal_to_fire["update-album"].add(album)
                self.__signal_to_fire["added-songs"].add(song)
                
                if self.__cache_func_song_tuple:
                    self.__cache_song_tuple[song.get("uri")] = self.__cache_func_song_tuple(song)
                self.__condition.release()    
                
    def db_entry_removed(self, db, song_type, songs):            
        if song_type not in self.__type: return
        
        for song in songs:
            if self.__filter(song) and song in self.__tree[1]:
                self.__delete_cache(song)
                
                genre, artist, album = self.__get_info(song)
                
                self.__condition.acquire()
                self.__signal_to_fire["update-genre"].add(genre)
                self.__signal_to_fire["update-artist"].add(artist)
                self.__signal_to_fire["update-album"].add(album)
                self.__signal_to_fire["removed-songs"].add(song)
                self.__condition.release()                    
                if self.__cache_func_song_tuple:
                    del self.__cache_song_tuple[song.get("uri")]

                    
    def db_entry_changed(self, db, song_type, infos, use_quick_update_change=False):                
        if song_type not in self.__type: return
        
        for info in infos:
            song, old_keys_values = info
            if old_keys_values.has_key("genre") or old_keys_values.has_key("artist") or old_keys_values.has_key("album"):
                use_quick_update_change = False
                
            if not use_quick_update_change:    
                use_quick_update_change = True
                for tag, old_value in old_keys_values.iteritems():
                    new_value = song.get(tag)
                    if old_value != new_value:    
                        use_quick_update_change = False
                        break
                    
            if not old_keys_values.has_key("genre"): genre = song.get("genre")        
            else: genre = old_keys_values.get("genre")
            if not old_keys_values.has_key("artist"): artist = song.get("artist")
            else: artist = old_keys_values.get("artist")
            if not old_keys_values.has_key("album"): album = song.get("album")
            else: album = old_keys_values.get("album")
                
            if song in self.__tree[1]:
                self.__condition.acquire()
                if old_keys_values.has_key("genre"):
                    self.__signal_to_fire["update-genre"].add(old_keys_values["genre"])
                if old_keys_values.has_key("artist"):
                    self.__signal_to_fire["update-artist"].add(old_keys_values["artist"])
                if old_keys_values.has_key("album"):
                    self.__signal_to_fire["update-album"].add(old_keys_values["album"])
                if not use_quick_update_change:
                    self.__signal_to_fire["removed-songs"].add(song)
                self.__condition.release()
                
            try:    
                self.__delete_cache(song, (genre, artist, album))
            except: pass    
            
            if self.__cache_func_song_tuple:
                try: del self.__cache_song_tuple[song.get("uri")]
                except KeyError: pass 
            
            if self.__filter(song):
                self.__add_cache(song)
                if self.__cache_func_song_tuple:
                    self.__cache_song_tuple[song.get("uri")]= self.__cache_func_song_tuple(song)

                je, artist, album = self.__get_info(song)

                self.__condition.acquire()

                if old_keys_values.has_key("genre"):
                    self.__signal_to_fire["update-genre"].add(genre)
                if old_keys_values.has_key("artist"):
                    self.__signal_to_fire["update-artist"].add(artist)
                if old_keys_values.has_key("album"):
                    self.__signal_to_fire["update-album"].add(album)

                if use_quick_update_change:
                    self.__signal_to_fire["quick-update-songs"].add(song)
                else:
                    self.__signal_to_fire["added-songs"].add(song)
                self.__condition.release()
                
    def __add_cache(self, song):            
        if self.__cache_func_song_tuple:
            self.set_song_tuple_cache(song)
            
        genre2, artist2, album = self.__get_info(song)    
        sgenre, sartist, salbum = self.__get_str_info(song)
        
        self.__tree[1].add(song)
        for genre in [genre2, "###ALL###"]:
            for artist in [artist2, "###ALL###"]:
                self.__tree[0].setdefault(genre, ({}, set(), sgenre))
                self.__tree[0][genre][1].add(song)
                self.__tree[0][genre][0].setdefault(artist, ({}, set(), sartist))
                self.__tree[0][genre][0][artist][1].add(song)
                self.__tree[0][genre][0][artist][0].setdefault(album, ({}, set(), salbum))
                self.__tree[0][genre][0][artist][0][album][1].add(song)
                
        self.__add_song_attr(song, (genre2, artist2, album))        
                
    def __add_song_attr(self, song, info):            
        song_dir = song.get_dir()
        genre, artist, album = info
        if song_dir:
            self.__attr_songs.setdefault(song_dir, ({}, {}, {}, set()))
            self.__attr_songs[song_dir][3].add(song)
            self.__attr_songs[song_dir][0].setdefault(genre, set())
            self.__attr_songs[song_dir][0][genre].add(song)
            self.__attr_songs[song_dir][1].setdefault(artist, set())
            self.__attr_songs[song_dir][1][artist].add(song)
            self.__attr_songs[song_dir][2].setdefault(album, set())
            self.__attr_songs[song_dir][2][album].add(song)
            
            
    def get_attr_infos(self, info_type="###ALL###", song_dir=None):        

        if not self.__attr_songs:
            return []
        
        attr_list = []        
        
        if info_type == "###ALL###":
            for key, value in self.__attr_songs.iteritems():
                name = os.path.split(key)[1]
                num = len(value[3])
                attr_list.append((name, key, num, "folder"))
            return attr_list        

        # if info_type == "genre":
        #     if song_dir not in self.__attr_songs:
        #         return []
        #     for key, value in self.__attr_songs[song_dir][0].iteritems():
        #         num = len(value)
        #         attr_list.append((key, key, num, "genre"))
        #     return attr_list    
        
        # if info_type == "artist":
        #     if song_dir not in self.__attr_songs:
        #         return []
        #     for key, value in self.__attr_songs[song_dir][1].iteritems():
        #         num = len(value)
        #         attr_list.append((key, key, num, "artist"))
        #     return attr_list    
        
        # if info_type == "album":
        #     if song_dir not in self.__attr_songs:
        #         return []
        #     for key, value in self.__attr_songs[song_dir][2].iteritems():
        #         num = len(value)
        #         try:
        #             artist_name = list[value][0].get_str("artist")
        #         except:    
        #             artist_name = ""
        #         attr_list.append((key, artist_name,  num, "album"))
        #     return attr_list    
        
    def get_attr_songs(self,song_dir=None, info_type="###ALL###", info_key=None):
        if not self.__attr_songs:
            return []
        if info_type == "###ALL###":
            try:
                return self.__attr_songs[song_dir][3]
            except:
                return []
            
        if info_type == "genre":
            try:
                return self.__attr_songs[song_dir][0][info_key]
            except:
                return []
            
        if info_type == "artist":
            try:
                return self.__attr_songs[song_dir][1][info_key]
            except:
                return []

        if info_type == "album":
            try:
                return self.__attr_songs[song_dir][2][info_key]
            except:
                return []
        
    def __delete_song_attr(self, song, info):        
        song_dir = song.get_dir()
        genre, artist, album = info
        if song_dir:
            try:
                self.__attr_songs[song_dir][3].remove(song)
            except (KeyError, ValueError):    
                return
            
            self.__attr_songs[song_dir][0][genre].remove(song)
            self.__attr_songs[song_dir][1][artist].remove(song)
            self.__attr_songs[song_dir][2][album].remove(song)
            
            if len(self.__attr_songs[song_dir][0][genre]) == 0:
                del self.__attr_songs[song_dir][0][genre]
                
            if len(self.__attr_songs[song_dir][1][artist]) == 0:
                del self.__attr_songs[song_dir][1][artist]
            
            if len(self.__attr_songs[song_dir][2][album]) == 0:
                del self.__attr_songs[song_dir][2][album] 
                
            if len(self.__attr_songs[song_dir][3]) == 0:    
                del self.__attr_songs[song_dir]
            
    def __delete_cache(self, song, old_values=False):            
        if old_values:
            genre2, artist2, album = old_values
        else:    
            genre2, artist2, album = self.__get_info(song)
            
        try:    
            self.__tree[1].remove(song)
        except (KeyError, ValueError):    
            return
        for genre in [ genre2, "###ALL###"]:
            self.__tree[0][genre][1].remove(song)
            for artist in [artist2, "###ALL###"]:
                self.__tree[0][genre][0][artist][1].remove(song)
                self.__tree[0][genre][0][artist][0][album][1].remove(song)
                
                if len(self.__tree[0][genre][0][artist][0][album][1]) == 0:
                    del self.__tree[0][genre][0][artist][0][album]
                    if len(self.__tree[0][genre][0][artist][1]) == 0:
                        del self.__tree[0][genre][0][artist]
            if len(self.__tree[0][genre][1]) == 0:            
                del self.__tree[0][genre]
                
        self.__delete_song_attr(song, (genre2, artist2, album))        
                
    def __get_str_info(self, song):            
        genre = song.get_str("genre")
        artist = song.get_str("artist")
        album = song.get_str("album")
        if not genre.strip(): genre = _("Unknown")
        if not artist.strip(): artist = _("Unknown")
        if not album.strip(): album = _("Unknown")
        return genre, artist, album
    
    def __get_info(self, song):
        genre = song.get_str("genre")
        artist = song.get_str("artist")
        album = song.get_str("album")
        if not genre.strip(): genre = _("Unknown")
        if not artist.strip(): artist = _("Unknown")
        if not album.strip(): album = _("Unknown")
        return genre, artist, album
    
    def get_random_song(self):
        songs = list(self.__tree[1])
        shuffle(songs)
        return songs[0]
    
    def get_songs(self, genres=[], artists=[], albums=[]):
        songs = set()
        if not genres and not artists and not albums:
            songs.update(self.__tree[1].copy())
        else:    
            if not genres:
                tmp_genres = ["###ALL###"]
            else:    
                tmp_genres = genres
            for genre in tmp_genres:    
                if not artists:
                    if not albums:
                        try:
                            songs.update(self.__tree[0][genre][1].copy())
                        except KeyError:    
                            continue
                        continue
                    else:
                        tmp_artists = ["###ALL###"]
                        
                else:        
                    tmp_artists = artists
                    
                for artist in tmp_artists:    
                    if not albums:
                        try:
                            songs.update(self.__tree[0][genre][0][artist][1].copy())
                        except KeyError:    
                            continue
                    for album in albums:    
                        try:
                            songs.update(self.__tree[0][genre][0][artist][0][album][1].copy())
                        except KeyError:    
                            continue
        return songs                
    
    def get_info(self, key_request, genres=[], artists=[], key_values=None, extended_info=False):
        if key_request not in ["genre", "artist", "album"]:
            raise NotImplementedError("Only genre, artist, album are support for key_request")
        infos = {}
        if key_request == "genre":
            try:
                if key_values is None:
                    mydict = self.__tree[0].copy()
                else:    
                    mydict = {}
                    for value in key_values:
                        try:
                            mydict[value] = self.__tree[0][value]
                        except KeyError:    
                            pass
                for key, info in mydict.iteritems():        
                    if key == "###ALL###":
                        continue
                    if extended_info:
                        _title, playcount, duration, songs = infos.get(key, (info[2], 0, 0, []))
                        playcount += sum(s.get("#playcount", 0) for s in info[1])
                        duration += sum(s.get("#duration", 0) for s in info[1])
                        songs.extend(info[1])
                        infos[key] = (info[2], playcount, duration, songs)
                    else:    
                        nb = len(info[1])
                        if nb > 0:
                            infos[key] = (info[2], infos.setdefault(key, (info[2], 0))[1] + nb)
            except KeyError:                
                pass
        else:    
            tmp_genres = set(genres) & set(self.__tree[0].keys())
            if not tmp_genres:
                tmp_genres = ["###ALL###"]
            for genre in tmp_genres:    
                if key_request == "artist":
                    try:
                        if key_values is None:
                            mydict = self.__tree[0][genre][0].copy()
                        else:    
                            mydict = {}
                            for value in key_values:
                                try: mydict[value] = self.__tree[0][genre][0][value]
                                except KeyError: pass

                        for key, info in mydict.iteritems():
                            if key == "###ALL###": continue
                            if extended_info:
                                _title, playcount, duration, songs = infos.get(key,(info[2], 0, 0, []))
                                playcount += sum(s.get("#playcount", 0) for s in info[1])
                                duration += sum(s.get("#duration", 0) for s in info[1])
                                songs.extend(info[1])
                                infos[key] = (info[2], playcount, duration, songs)
                            else:
                                nb = len(info[1])
                                if nb > 0: infos[key] = (info[2], infos.setdefault(key,(info[2], 0))[1] + nb)
                    except KeyError: pass
                else:
                    try: tmp_artists = set(artists) & set(self.__tree[0][genre][0].keys())
                    except KeyError: continue
                    if not tmp_artists: tmp_artists = ["###ALL###"]
                    for artist in tmp_artists:
                        if key_request == "album":
                            try:
                                if key_values is None: mydict = self.__tree[0][genre][0][artist][0].copy()
                                else:
                                    mydict = {}
                                    for value in key_values:
                                        try: mydict[value] = self.__tree[0][genre][0][artist][0][value]
                                        except KeyError: pass

                                for key, info in mydict.iteritems():
                                    if key == "###ALL###": continue
                                    if extended_info:
                                        _title, playcount, duration, songs = infos.get(key,(info[2], 0, 0, []))
                                        playcount += sum(s.get("#playcount", 0) for s in info[1])
                                        duration += sum(s.get("#duration", 0) for s in info[1])
                                        songs.extend(info[1])
                                        infos[key] = (info[2], playcount, duration, songs)
                                    else:
                                        nb = len(info[1])
                                        if nb > 0:
                                            infos[key] = (list(info[1])[0].get_str("artist"), 
                                                          infos.setdefault(key,(info[2], 0))[1] + nb)
                            except KeyError: pass

        if key_values:
            [ infos.update({key:None}) for key in key_values if not infos.has_key(key) ]

        return infos
    

MediaDB = MediaDatebase()        
MediaDB.register_type("local")
MediaDB.register_type("webcast")
MediaDB.register_type("unknown")
MediaDB.register_type("unknown_local")
MediaDB.register_type("cue")
MediaDB.register_type("audiocd")
MediaDB.register_playlist_type("local")
