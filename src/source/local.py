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

import utils
import common
from widget.dialog import WinDir, WinFile
from library import MediaDB
from widget.jobs_manager import Job


class ImportFolderJob(Job):
    def __init__(self, dirs=None):
        if not dirs:
            dirs = WinDir().run()
            if dirs:
                dirs = [ dirs ]
                
        if dirs:        
            self.message = "Reading directories..."
            self.dirs = dirs
            super(ImportFolderJob, self).__init__()
            
    def add_to_library(self, uri):        
        tags = {"uri" : uri}
        try:
            MediaDB.get_or_create_song(tags, "local", read_from_file=True)
        except:    
            self.logerror("Failed load %s", uri)
            
    def job(self):        
        '''job'''
        dirs = self.dirs
        db_uris = set(MediaDB.get_all_uris())
        alldirs = [ utils.get_path_from_uri(each_dir) for each_dir in dirs ]
        
        for mdir in alldirs:
            for dirpath, dirs, names in os.walk(mdir):
                for name in names:
                    if name[0] != "." and common.file_is_supported(os.path.join(dirpath, name)):
                        valid_file = os.path.join(dirpath, name)
                        real_file = os.path.realpath(valid_file)
                        uri = utils.get_uri_from_path(real_file)
                        if uri not in db_uris:
                            self.add_to_library(uri)
                        elif os.path.getctime(real_file) > MediaDB.get_song(uri).get("#ctime"):     
                            self.add_to_library(uri)
                    yield os.path.join(dirpath, name)        
                    
class ImportPlaylistJob(Job):
    def __init__(self, dirs=None, callback=None, pos=None):
        if not dirs:
            dirs = WinDir().run()
            if dirs:
                dirs = [ dirs ]
                
        if dirs:        
            self.message = "Reading directories..."
            self.dirs = dirs
            super(ImportPlaylistJob, self).__init__()
            
        self.add_song_cache = []    
        self.pos = pos
        self.callback = callback
            
    def __get_or_create_song(self, uri):        
        tags = {"uri" : uri}
        try:
            song = MediaDB.get_or_create_song(tags, "local", read_from_file=True)
            self.add_song_cache.append(song)
        except:    
            self.logerror("Failed load %s", uri)
            
    def job(self):        
        '''job'''
        dirs = self.dirs
        db_uris = set(MediaDB.get_all_uris())
        alldirs = [ utils.get_path_from_uri(each_dir) for each_dir in dirs ]
        start = time.time()
        
        for mdir in alldirs:
            for dirpath, dirs, names in os.walk(mdir):
                for name in names:
                    if name[0] != "." and common.file_is_supported(os.path.join(dirpath, name)):
                        valid_file = os.path.join(dirpath, name)
                        real_file = os.path.realpath(valid_file)
                        uri = utils.get_uri_from_path(real_file)
                        if uri not in db_uris:
                            self.__get_or_create_song(uri)
                        elif os.path.getctime(real_file) > MediaDB.get_song(uri).get("#ctime"):     
                            self.__get_or_create_song(uri)
                        end = time.time()    
                        if end - start > 0.2:
                            self.callback(self.add_song_cache, self.pos)
                            self.pos += len(self.add_song_cache)
                            del self.add_song_cache[:]
                            start = time.time()
                        else:    
                            end = time.time()
                            
                    yield os.path.join(dirpath, name)        
                    
        if self.add_song_cache:            
            self.callback(self.add_song_cache, self.pos)
        
class ImportFileJob(object):        
    '''import file to db'''
    def __init__(self):
        uri =WinFile().run()
        if uri and common.file_is_supported(utils.get_path_from_uri(uri)):
            tags = {"uri":uri}
            try:
                MediaDB.get_or_create_song(tags, "local", read_from_file=True)
            except:    
                traceback.print_exc()
              
class CheckDeletedDBJob(Job):                
    __message = "Check deleted files"
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
        songs = MediaDB.get_songs("local")
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
                    
            yield "重新的加载数据库 %d/%s"%(i,total)
