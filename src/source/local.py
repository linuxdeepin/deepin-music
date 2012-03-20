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
import gtk
from time import time
import traceback

from config import config
from findfile import get_music_dir
import utils
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
            
    def job(self):        
        '''job'''
        dirs = self.dirs
        message = self.message
        start = time()
        
        added = []
        db_uris = set(MediaDB.get_all_uris())
        alldirs = [ utils.get_path_from_uri(each_dir) for each_dir in dirs ]
        last_estimated = estimated = 0
        
        total_dirs = len(alldirs)
        parsed_dirs = 0
        for mdir in alldirs:
            for dirpath, dirs, names in os.walk(mdir):
                [ dirs.remove(ignore_dir) for ignore_dir in dirs if ignore_dir[0] == "." ]
                [ alldirs.append(os.path.realpath(os.path.join(dirpath, sub_dir))) for sub_dir in dirs if os.path.islink(os.path.join(dirpath, sub_dir)) ]
                
                total_dirs += len(dirs)
                parsed_dirs += 1
                valid_files = set([ os.path.join(dirpath, name) for name in names if name[0] != "." and utils.file_is_supported(os.path.join(dirpath, name)) ])
                for each_file in valid_files:
                    real_file = os.path.realpath(each_file)
                    uri = utils.get_uri_from_path(real_file)
                    if real_file not in db_uris:
                        added.append(uri)
                    elif os.path.getctime(real_file) > MediaDB.get_song(uri).get("#ctime"):
                        added.append(uri)
                        
                estimated = float(parsed_dirs) / float(total_dirs)        
                if max(estimated, last_estimated) == estimated:
                    last_estimated = estimated
                yield message + "(%d of %d)" % (parsed_dirs, total_dirs), last_estimated, False    

                
        i = 0        
        total = len(added)
        added = set(added)
        for uri in added:
            i += 1
            tags = {"uri" : uri}
            try:
                MediaDB.get_or_create_song(tags, "local", read_from_file=True)
            except:    
                self.logerror("Failed load %s", uri)
            yield ("Reading file") + "%d/%d..." % (i, total), float(i) / float(total), False    
            # yield uri, float(i) / float(total), False  
            
        end = time()    
        self.loginfo("%d songs loaded in %d seconds", total, (end - start))
        print "%d songs loaded in %d seconds" % ( total, (end - start))

        
class ImportFileJob(object):        
    '''import file to db'''
    def __init__(self):
        uri =WinFile().run()
        if uri and utils.file_is_supported(utils.get_path_from_uri(uri)):
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
                    
            yield "Reload database %d/%s"%(i,total), float(i)/float(total), False
            
            
def ReloadDB():
    ReloadDBJob()
    search_dir = config.get("library","location", get_xdg_music_dir())
    search_dir = os.path.expanduser(search_dir)
    monitored_folder = utils.get_uri_from_path(search_dir)
    if monitored_folder:
        ImportFolderJob([monitored_folder])
            
