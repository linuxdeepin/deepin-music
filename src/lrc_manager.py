#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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
import re
from deepin_utils.net import is_network_connected

from config import config
from lrc_download import TTPlayer, DUOMI, SOSO, TTPod
from cover_query import poster
from helper import Dispatcher
import utils

class LrcManager(object):
    
    def __init__(self):
        pass
        
    def get_lrc_search_str(self, song):    
        artist = song.get_str("artist")
        title = song.get_str("title")
        if artist:
            result = "%s-%s.lrc" % (artist, title)
        else:
            result = "%s.lrc" % title
        return result.replace("/", "")    
        
    def vaild_lrc(self, filepath):    
        try:
            fp = open(filepath, "r")
            lrc_content = fp.read()
            fp.close()
        except:    
            return False
        else:
            partial="".join( (i for i in lrc_content if (ord(i) < 128 and ord(i) != 0) ) )
            return bool(re.search('\[\d{1,}:\d{1,}.*?\]',partial))
            
    def multiple_engine(self, song, lrc_path, artist, title):    
        try:
            ret = False
            
            ting_result = poster.query_lrc_info(artist, title)
            if ting_result:
                urls = [item[2] for item in ting_result]
                for url in urls:
                    ret = utils.download(url, lrc_path)
                    if ret:
                        return lrc_path
            
            result = TTPlayer().request(artist, title)

            if result:
                urls = [item[2] for item in result]                
                for url in urls:
                    ret = utils.download(url, lrc_path)
                    if ret and self.vaild_lrc(lrc_path):
                        return lrc_path
                    
            ttpod_result = TTPod().request_data(artist, title)        
            if ttpod_result:
                with open(lrc_path, 'wb') as fp:
                    fp.write(ttpod_result)
                    return lrc_path
                        
            duomi_result = DUOMI().request(artist, title)
            if duomi_result:
                urls = [item[2] for item in duomi_result]                
                for url in urls:
                    ret = utils.download(url, lrc_path, "gbk")
                    if ret and self.vaild_lrc(lrc_path):
                        return lrc_path
                        
            soso_result =  SOSO().request(artist, title)
            if soso_result:
                urls = [item[2] for item in soso_result]                
                for url in urls:
                    ret = utils.download(url, lrc_path, "gb18030")
                    if ret and self.vaild_lrc(lrc_path):
                        return lrc_path
            try:    
                os.unlink(lrc_path)
            except:    
                pass
                
            return None
                    
        except Exception, e:
            print e
            return None
        
    def allocation_lrc_file(self, song, lrc_path):    
        if os.path.exists(lrc_path):
            if self.vaild_lrc(lrc_path):
                song["location_lrc"] = lrc_path
                # save_lrc_path = self.get_lrc_filepath(song)
                # if os.path.exists(save_lrc_path): os.unlink(save_lrc_path)
                # utils.run_command("cp %s %s" % (lrc_path, save_lrc_path))
                Dispatcher.reload_lrc(song)
                
    def unallocation_lrc_file(self, song):            
        try:
            del song["location_lrc"]
        except KeyError:    
            pass
        else:
            Dispatcher.reload_lrc(song)
        
    def get_lrc_filepath(self, song):    
        save_path = os.path.expanduser(config.get("lyrics", "save_lrc_path"))
        if not os.path.exists(save_path):
            utils.makedirs(save_path)
        fillpath = os.path.join(save_path, self.get_lrc_search_str(song))
        return fillpath
        
    def get_lrc(self, song, try_web=True):    
        
        lrc_path = self.get_lrc_filepath(song)
        
        
        # user allocation lrc
        location_lrc = song.get("location_lrc", "")
        if location_lrc and os.path.exists(location_lrc):
            return location_lrc
        
        # lrc already exist
        if os.path.exists(lrc_path):
            if self.vaild_lrc(lrc_path):
                return lrc_path
            else:
                try:
                    os.unlink(lrc_path)
                except: pass    
                
        # search in current directory and same name file    
        current_lrc_path = os.path.join(song.get_dir(), song.get_filename() + ".lrc")
        if os.path.exists(current_lrc_path) and self.vaild_lrc(current_lrc_path):
            return current_lrc_path
        
        # Search in local directory of the file
        if song.get("uri") != None and song.get_scheme() == "file":
            local_lrc = os.path.join(song.get_dir(), self.get_lrc_search_str(song))
            if os.path.exists(local_lrc):
                return local_lrc
            
        
        if try_web and is_network_connected():
            if song.get("lyric_url", None):
                ret = utils.download(song.get("lyric_url"), lrc_path)
                if ret:                
                    return lrc_path
            
            trust_a = song.get_str("artist")
            trust_t = song.get_str("title")
            filename = song.get_filename()
            if "-" in filename:
                untrust_a = filename.split("-")[0].strip()
                untrust_t = filename.split("-")[1].strip()
            else:    
                untrust_a = song.get_str("artist")
                untrust_t = song.get_filename()
            trust_result = self.multiple_engine(song, lrc_path, trust_a, trust_t)
            if trust_result:
                return trust_result
            else:
                return self.multiple_engine(song, lrc_path, untrust_a, untrust_t)
        return None
