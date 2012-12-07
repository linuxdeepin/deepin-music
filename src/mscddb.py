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

from xml.sax import parseString, handler

import os
import socket    
socket.setdefaulttimeout(30) # 30s

from logger import Logger
from xdg_support import get_cache_file
import utils

class MSCDDBParser(handler.ContentHandler):
    CD_KEKS = {
        "albumTitle"  : "album",
        # "albumArtist" : "artist",
        "genre"       : "genre",
        "releaseDate" : "date",
        }
    TRACK_KEYS = {
        "trackNumber" : "#track",        
        "trackTitle"  : "title",
        "trackPerformer" : "artist",
        }
    def __init__(self):
        self.cd_infos = {}
        self.track_infos = []
        self.__temp_info = {}
        self.cd_label = ""
        
    def startElement(self, name, attrs):
        self.content = ""
        
    def characters(self, content):    
        self.content = content.encode("utf-8", "ingnore")
        
    def endElement(self, name):    
        if name in self.CD_KEKS:
            self.cd_infos[self.CD_KEKS[name]] = self.content
        elif name in self.TRACK_KEYS:
            if name == "trackNumber":
                self.__temp_info[self.TRACK_KEYS[name]] = int(self.content)
            else:    
                self.__temp_info[self.TRACK_KEYS[name]] = self.content
        elif name == "track":        
            self.__temp_info.update(self.cd_infos)
            self.track_infos.append(self.__temp_info)
            self.__temp_info = {}
        elif name == "label":    
            self.cd_label = self.content
            
class MSCDDB(Logger):            
    QUERY_API = "http://toc.music.metaservices.microsoft.com/cdinfo/GetMDRCD.aspx?CD=%s"
    
    def parse_disc_id(self, disc_id):
        if isinstance(disc_id, list):
            return "+".join(["%X" % key for key in disc_id])
        return disc_id
    
    def query(self, disc_id):
        local_xml_file = self.get_save_file(disc_id)
        if os.path.exists(local_xml_file):
            self.loginfo("Read cd info from local..")
            result = self.read_from_local(local_xml_file)
            if result is not None: 
                return result
            else:
                os.unlink(local_xml_file)
                return self.query(disc_id)
        else:    
            self.loginfo("Read cd info fromo remote..")
            ms_disc = self.parse_disc_id(disc_id)
            url = self.QUERY_API % ms_disc
            ret = utils.download(url, local_xml_file)
            if ret:
                return self.read_from_local(local_xml_file)
            else:
                return None
        
    def read_from_local(self, local_file):        
        xml_fp = open(local_file, "r")
        xml_info = xml_fp.read()
        xml_fp.close()
        return self.handle_xml_info(xml_info)
        
    def handle_xml_info(self, xml_info):       
        handler = MSCDDBParser()            
        try:
            parseString(xml_info, handler)
        except Exception, e:    
            print e
            return None
        else:
            return handler.track_infos, handler.cd_label
            
    def get_save_file(self, disc_id):        
        disc_hash = "".join(["%d" % key for key in disc_id])                
        return get_cache_file("cddb/%s" % disc_hash)

mscddb = MSCDDB()
