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
import traceback
import time
import gobject

import utils
import common
from widget.dialog import WinDir, WinFile
from library import MediaDB
from widget.jobs_manager import Job
from nls import _

class ImportFolderJob(Job):
    def __init__(self, dirs=None):
        if not dirs:
            dirs = WinDir().run()
            if dirs:
                dirs = [ dirs ]
                
        if dirs:        
            self.message = _("Reading directories...")
            self.dirs = dirs
            super(ImportFolderJob, self).__init__()
            
    def add_to_library(self, uri):        
        try:
            MediaDB.get_songs_by_uri(uri)
        except:    
            self.logerror("Failed load %s", uri)
            
    def job(self):        
        dirs = self.dirs
        added = []
        db_uris = set(MediaDB.get_all_uris())
        alldirs = [ utils.get_path_from_uri(each_dir) for each_dir in dirs ]
        
        for mdir in alldirs:
            for dirpath, dirs, names in os.walk(mdir):
                [ dirs.remove(each_dir) for each_dir in dirs if each_dir[0] == "." ]
                for each_dir in dirs:
                    full_path_dir  = os.path.join(dirpath, each_dir)
                    if os.path.islink(full_path_dir):
                        alldirs.append(os.path.realpath(full_path_dir))

                valid_files = []    
                for name in names:    
                    full_path_file = os.path.join(dirpath, name)
                    if name[0] != "." and common.file_is_supported(full_path_file):
                        valid_files.append(full_path_file)
                    yield full_path_file    
                        
                valid_files = set(valid_files)    
                for each_file in valid_files:
                    real_file = os.path.realpath(each_file)
                    uri = utils.get_uri_from_path(real_file)
                    if real_file not in db_uris:
                        added.append(uri)
                    elif os.path.getctime(real_file) > MediaDB.get_song(uri).get("#ctime"):
                        added.append(uri)

        added = set(added)
        for uri in added:
            self.add_to_library(uri)
            yield utils.get_path_from_uri(uri)
                    
class ImportPlaylistJob(Job):
    def __init__(self, dirs=None, callback=None, pos=None, sort=False):
        if not dirs:
            dirs = WinDir().run()
            if dirs:
                dirs = [ dirs ]
                
        if dirs:        
            self.message = "Reading directories..."
            if not isinstance(dirs, list):
                dirs = [ dirs ]
            self.dirs = dirs
            super(ImportPlaylistJob, self).__init__()
            
        self.add_song_cache = []    
        self.pos = pos
        self.sort = sort
        self.callback = callback
            
    def __get_or_create_song(self, uri):        
        try:
            songs = MediaDB.get_songs_by_uri(uri)
            self.add_song_cache.extend(songs)
        except:    
            self.logerror("Failed load %s", uri)
            
    def job(self):        
        '''job'''
        dirs = self.dirs
        added = []
        db_uris = set(MediaDB.get_all_uris())
        alldirs = [ utils.get_path_from_uri(each_dir) for each_dir in dirs ]
        
        for mdir in alldirs:
            for dirpath, dirs, names in os.walk(mdir):
                [ dirs.remove(each_dir) for each_dir in dirs if each_dir[0] == "." ]
                for each_dir in dirs:
                    full_path_dir  = os.path.join(dirpath, each_dir)
                    if os.path.islink(full_path_dir):
                        alldirs.append(os.path.realpath(full_path_dir))

                valid_files = []    
                for name in names:    
                    full_path_file = os.path.join(dirpath, name)
                    if name[0] != "." and common.file_is_supported(full_path_file):
                        valid_files.append(full_path_file)
                    yield full_path_file    
                        
                valid_files = set(valid_files)    
                for each_file in valid_files:
                    real_file = os.path.realpath(each_file)
                    uri = utils.get_uri_from_path(real_file)
                    if real_file not in db_uris:
                        added.append(uri)
                    elif os.path.getctime(real_file) > MediaDB.get_song(uri).get("#ctime"):
                        added.append(uri)
        
        added = set(added)
        # for uri in added:
            # self.__get_or_create_song(uri)
            # end = time.time()
            # if (end - start) * 1000 > 1000:
            #     self.callback(self.add_song_cache, self.pos, self.sort)
            #     self.pos += len(self.add_song_cache)
            #     del self.add_song_cache[:]
            #     start = time.time()
            # else:    
            #     end = time.time()
               
            # yield utils.get_path_from_uri(uri)
 
        # if self.add_song_cache:            
        if added:
            gobject.idle_add(self.callback, added, self.pos, self.sort)
            # self.callback(self.add_song_cache, self.pos, self.sort)
        
class ImportFileJob(object):        
    '''import file to db'''
    def __init__(self):
        uri =WinFile().run()
        if uri and common.file_is_supported(utils.get_path_from_uri(uri)):
            try:
                MediaDB.get_songs_by_uri(uri)
            except:    
                traceback.print_exc()
              
class CheckDeletedDBJob(Job):                
    __message = _("Check deleted files")
    def job(self):
        songs = MediaDB.get_songs("local")
        total = len(songs)
        i = 0
        for song in songs:
            i += 1
            if not song.exists():
                MediaDB.remove(song)
            yield "Check deleted files %d/%s" % (i,total), float(i)/float(total), False
            
             
class ReloadDBJob(Job):
    __message = "Reload database"
    def job(self):
        songs = MediaDB.get_songs_by_type("local")
        total = len(songs)
        i = 0
        for song in songs:
            i += 1
            if not song.exists():
                MediaDB.remove(song)
            else:
                try: MediaDB.reload_song_from_file(song)
                except:
                    traceback.print_exc()
                    
            yield "%s %d/%s" % (_("Reload database"),i,total)
