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

import urllib2
import socket    
socket.setdefaulttimeout(30) # 30s

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
            
class MSCDDB(object):            
    QUERY_API = "http://toc.music.metaservices.microsoft.com/cdinfo/GetMDRCD.aspx?CD=%s"
    @classmethod
    def parse_disc_id(cls, disc_id):
        if isinstance(disc_id, list):
            return "+".join(["%X" % key for key in disc_id])
        return disc_id
    
    @classmethod
    def query(cls, disc_id):
        ms_disc = cls.parse_disc_id(disc_id)
        url = cls.QUERY_API % ms_disc
        try:
            fp = urllib2.urlopen(url)
        except:    
            return []
        else:
            xml_info = fp.read()
            try:
                handler = MSCDDBParser()
                parseString(xml_info, handler)
            except:    
                return []
            else:
                return handler.track_infos
        finally:    
            fp.close()
