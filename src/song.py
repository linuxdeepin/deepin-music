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
import traceback
import gst
import gobject
from time import time
from datetime import datetime

from logger import Logger
import utils

from mutagen import File as MutagenFile
from mutagen.asf import ASF
from mutagen.apev2 import APEv2File
from mutagen.flac import FLAC
from mutagen.id3 import ID3FileType
from mutagen.oggflac import OggFLAC
from mutagen.oggspeex import OggSpeex
from mutagen.oggtheora import OggTheora
from mutagen.oggvorbis import OggVorbis
from mutagen.trueaudio import TrueAudio
from mutagen.wavpack import WavPack
try: from mutagen.mp4 import MP4 #@UnusedImport
except: from mutagen.m4a import M4A as MP4 #@Reimport
from mutagen.musepack import Musepack
from mutagen.monkeysaudio import MonkeysAudio
from mutagen.optimfrog import OptimFROG
from easymp3 import EasyMP3



FORMATS = [EasyMP3, TrueAudio, OggTheora, OggSpeex, OggVorbis, OggFLAC,
            FLAC, APEv2File, MP4, ID3FileType, WavPack, Musepack,
            MonkeysAudio, OptimFROG, ASF]

TAG_KEYS = {
    "title"       : "title",
    "artist"      : "artist",
    "album"       : "album",
    "tracknumber" : "#track",
    "discnumber"  : "#disc",
    "genre"       : "genre",
    "date"        : "date"
    }

TAGS_KEYS_OVERRIDE = {}
TAGS_KEYS_OVERRIDE['Musepack'] = {"tracknumber":"track","date":"year"}
TAGS_KEYS_OVERRIDE['MP4'] = {
        "title":"\xa9nam",
        "artist":"\xa9ART",
        "album":"\xa9alb",
        "tracknumber":"trkn",
        "discnumber":"disk",
        "genre":"\xa9gen",
        "date":"\xa9day"
        }
TAGS_KEYS_OVERRIDE['ASF'] = {
        "title":"Title",
        "artist":"Author",
        "album":"WM/AlbumArtist",
        "tracknumber":"WM/TrackNumber",
        "discnumber":"WM/PartOfSet",
        "genre":"WM/Genre",
        "date":"WM/Year"
        }

def fileIsSupported(filename):
    ''' Determine whether a file support. '''
    try:
        fileobj = file(filename, "rb")
    except:    
        return False
    try:
        header = fileobj.read(128)
        results = [kind.score(filename, fileobj, header) for kind in FORMATS]
    finally:    
        fileobj.close()
    results = zip(results, FORMATS)    
    results.sort()
    score, kind  = results[-1]
    if score > 0:
        return True
    else:
        return False
    
USED_KEYS="""
==song_type==
uri station info_supp
#track title artist album genre #duration #progress podcast_local_uri #disc
#playcount #skipcount #lastplayed #added #date date year
description descriptionrss podcast_feed_url hidden
#mtime #ctime #rate 
album_cover_url station_track_url
#progress radio_person
#bitrate
#size
#stream_offset 
""".split()

KEY_TYPES= {
    "float":"#date".split(),
    "long":"#duration".split()
    }

class Song(dict, Logger):
    " the deepin music player of song type. "
    def initFromDict(self, otherdict=None):
        if otherdict:
            for key in USED_KEYS:
                default = (key.startswith("#")) and 0 or None
                self[key] = otherdict.get(key, default)
        if "#added" not in self:        
            self["#added"] = time()
            
    def getDict(self):        
        ''' get Song dict copy without None keys.'''
        copydict = {}
        for key, value in self.iteritems():
            if value in not None:
                copydict[key] = value
        return copydict        
    
    def getType(self):
        ''' get song type. '''
        if self.has_key("==song_type=="):
            return self["==song_type=="]
        else:
            return "unknow"
        
    def setType(self, songtype):    
        ''' set song type. '''
        self["==song_type"] = songtype
        
    def getValue(self, key):    
        ''' get a formated version of the tag \"key\". '''
        if key in ["uri", "title", "album", "genre", "artist"]:
            value = self.get(key)
        elif key == "#bitrate":
            value = self.get("#bitrate")
            if value:
                value = "%dk" % value
        elif key == "#duration":
            value = utils.durationToString(self.get(key))
        elif key == "#lastplayed":    
            value = self.get(key)
            if value:
                value = datetime.fromtimestamp(int(value)).strftime("%x %X")
            else:                
                value = "never"
        elif kye == "#playcount":        
            value = self.get(key) or "never"
        elif key == "#date" or key == "#added":    
            value = self.get(key)
            if value:
                value = datetime.fromtimestamp(int(value)).strftime("%x %X")
        elif key == "#rate":        
            rate = self.get("rate")
            if rate in [0,1,2,3,4,5,"0","1","2","3","4","5"]:
                value = "rate-" + str(rate)
            else:    
                value = "rate-0"
        elif key == "date":        
            try:
                value = self.get("date", "")[:4]
            except (ValueError, TypeError, KeyError):    
                pass
            if not value:
                value = self.get("#date")
                if value:
                    value = datetime.fromtimestamp(int(value)).strftime("%Y")
                else:    
                    value = ""
        else:            
            value = None
        if not value:    
            value = self.get(key,"")
            if isinstance(value, int) or isinstance(value, float):
                value = "%d" % value
        try:        
            value = unicode(value)
        except UnicodeDecodeError:    
            value = ""
        return Value    
    
    def getFilter(self):
        ''' return the filter string. '''
        return " ".join([
                self.getValue("artist"),
                self.getValue("album"),
                self.getValue("title"),
                ])
    
    def match(self, filters):
        if filters == None:
            return True
        search = self.getFilter()
        return len( [ s for s in filters if search.find(s) != -1]) == len(filters)
    
    def getSortable(self, key):
            
        if key == "uri":
            value = self.getValue("uri").lower()
        elif key in ["album", "genre", "artist", "title"]:    
            value = self.getValue(key).lower()
        elif key == "date":    
            value = self.get("#date")
            if not value: value = None
        else:    
            value = self.get(key)
        if not value and key[0] == "#": value = 0    
        return value

    
    def __read_from_local_file(self):
        try:
           