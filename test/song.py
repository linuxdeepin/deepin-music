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
import gst
import gobject
from time import time
from datetime import datetime
# import traceback


import utils
import common
import locale
from logger import Logger
from pinyin import TransforDB

TAG_KEYS = {"title"      : "title",
            "artist"     : "artist",
            "album"      : "album",
            "tracknumber": "#track",
            "discnumber" : "#disc",
            "genre"      : "genre",
            "date"       : "date"}

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

USED_KEYS="""
song_type uri title artist album genre date year 
description hidden album_cover_url station info_supp station_track_url lrc_url
#track #duration #progress #disc 
#playcount #skipcount 
#lastplayed #added #date #mtime #ctime #rate #progress #bitrate #size #stream_offset seek
sort_title sort_artist sort_album sort_genre search real_uri id collected collect
ssid company rating_avg subtype aid sid like
tag categroys genres country location comment location_lrc
""".split()

class Song(dict, Logger):
    ''' The deepin music song class. '''
    def init_from_dict(self, other_dict=None, cmp_key="uri"):
        ''' init from other dict. '''
        if other_dict:
            for key in USED_KEYS:
                default = 0 if key.startswith("#") else ""
                other_dict[key] = other_dict.get(key, default)
            self.update(other_dict)    
                        
        added = self.get("#added", 0)
        if not added:
            self["#added"] = time()
            
        self.cmp_key = cmp_key    
            
    def get_dict(self):        
        ''' return valid key dict. '''
        valid_dict = {}
        for key, value in self.iteritems():
            if value is not None:
                valid_dict[key] = value
        return valid_dict        
    
    def get_type(self):
        ''' return the song type. '''
        return self.get("song_type", "unknown")
        
    def set_type(self, song_type):
        ''' Set the song type. '''
        self["song_type"] = song_type
        
    def get_str(self, key, xml=False):    
        '''Get a formated version of the tag information.'''
        if key == "uri":
            value = utils.unescape_string_for_display(self.get("uri"))
        elif key == "title":    
            value = self.get("title")
            if not value:
                value = self.get_filename()
                
        elif key == "#size":        
            value = utils.str_size(self.get("#size"))
        elif key == "#bitrate":
            value = self.get("#bitrate")
            if value: 
                if isinstance(value, (int, float)):
                    test_v = value / 1000
                    if test_v > 1:
                        value = "%dk" % test_v
                
        elif key == "#duration":    
            value = utils.duration_to_string(self.get(key))
        elif key == "#lastplayed":    
            value = self.get(key)
            if value:
                value = datetime.fromtimestamp(int(value)).strftime("%Y-%m-%d")
            else:    
                value = "Never"
        elif key == "#playcount":        
            value = self.get(key) or "0"
        elif key in ["#date", "#added"]:
            value = self.get(key)
            if value:
                value = datetime.fromtimestamp(int(value)).strftime("%Y-%m-%d")
        elif key == "#rate":        
            rate = self.get("rate")
            if rate in [0,1,2,3,4,5,"0","1","2","3","4","5"]:
                value = "rate-" + str(rate)
            else: value = "rate-0"    
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
        if not value: value = self.get(key, "")    
        if isinstance(value, int) or isinstance(value, float): value = "%d" % value
        if value is None: value = ""
        return str(value)
        
    def get_filter(self):
        return " ".join([self.get_str("artist"),
                         self.get_str("album"),
                         self.get_str("title"),])
    
    def match(self, filter):
        if filter == None: return True
        search = self.get_filter()
        return len( [ s for s in filter if search.find(s) != -1 ] ) == len(filter)
        
    def get_sortable(self, key):
        '''Get sortable of the key.'''
        if key in ["album", "genre", "artist", "title"]:
            value = self.get("sort_%s" % key)
        elif key == "date":    
            value = self.get("#date")
            if not value: value = None
        elif key == "file":    
            try:
                value = locale.strxfrm(self.get_filename())
            except Exception:
                value = self.get_filename()
        else:    
            value = self.get(key, None)
            
        if not value and key[0] == "#": value = 0    
        return value
    
    
    def __setitem__(self, key, value):
        if key == "#track":
            value = str(value)
            if value is not None and not isinstance(value,int) and value.rfind("/")!=-1 and value.strip()!="":
                value = value.strip()

                disc_nr = value[value.rfind("/"):]
                try: 
                    disc_nr = int(disc_nr)
                except: 
                    disc_nr = self.get("#disc")
                self["#disc"] = disc_nr
                value = value[:value.rfind("/")]
        elif key == "date":
            try: 
                self["#date"] = utils.strdate_to_time(value)
            except: 
                value = None
                
        if key[0] == "#":       
            try:
                if key == "#date":
                    value = float(value)
                elif key == "#duration":    
                    value = long(value)
                else:    
                    value = int(value)
            except:        
                value = None
        if value is None:        
            if key in self:
                dict.__delitem__(self, key)
        else:        
            dict.__setitem__(self, key, value)
            
    def __sort_key(self):   
        return(
                self.get_sortable("album"),
                self.get_sortable("#disc"),
                self.get_sortable("#track"),
                self.get_sortable("artist"),
                self.get_sortable("title"),
                self.get_sortable("date"),
                self.get_sortable("#bitrate"),
                self.get_sortable("uri")
                )    
    sort_key = property(__sort_key)

         
    def __call__(self, key):        
        return self.get(key)
    
    def __hash__(self):
        return hash(self.get(self.owner_key))
    
    def __repr__(self):
        return "<Song %s>" % self.get(self.owner_key)
    
    def __cmp__(self, other_song):
        if not other_song: return -1
        try:
            return cmp(self.sort_key, other_song.sort_key)
        except AttributeError: return -1
        
    @property    
    def owner_key(self):
        return getattr(self, 'cmp_key', 'uri')

    
    def __eq__(self, other_song):
        try:
            return self.get(self.owner_key) == other_song.get(self.owner_key)
        except:
            return False
        
    def exists(self):    
        if self.get_type() == "cue":
            return utils.exists(self.get("real_uri"))
        elif self.get_type() == "audiocd":
            return os.path.exists(self.get("uri").split("#")[1])
        return utils.exists(self.get("uri"))
    
    def get_path(self):
        try:
            
            if self.get_type() == "cue":
                return utils.get_path_from_uri(self.get("real_uri"))
            elif self.get_type() == "local":
                return utils.get_path_from_uri(self.get("uri"))
            elif self.get_type() == "cdda":
                uri = self.get("uri")
                return utils.unquote(uri[uri.find("#") + 1:])
            else:
                return utils.unquote(self.get("uri"))
        except:
            return ""
    def get_dir(self):    
        try:
            return os.path.split(self.get_path())[0]
        except:
            return ""
        
    def get_scheme(self):    
        return utils.get_scheme(self.get("uri"))
    
    def get_ext(self, complete=True):
        return utils.get_ext(self.get("uri"), complete)
    
    def get_filename(self):
        value = self.get("uri")
        try:
            return os.path.splitext(utils.get_name(value))[0]
        except:
            return value
        
    def read_from_file(self):    
        ''' Read song infomation for file. '''
        # if self.get_scheme() == "file" and not self.exists():
        #     ret = False
        # if self.get_scheme() == "file" and common.file_is_supported(self.get_path()):
        #     ret = self.__read_from_local_file()
        # else:    
        #     ret = self.__read_from_remote_file()
        # return ret    
        try:
            ret = self.__read_from_local_file()
        except:    
            ret = False
        return ret    
    
    def __read_from_local_file(self):
        try:
            path = self.get_path()
            self["#size"]  = os.path.getsize(path)
            self["#mtime"] = os.path.getmtime(path)
            self["#ctime"] = os.path.getctime(path)
            
            audio = common.MutagenFile(self.get_path(), common.FORMATS)
            tag_keys_override = None

            if audio is not None:
                tag_keys_override = TAGS_KEYS_OVERRIDE.get(audio.__class__.__name__, None)
                for file_tag, tag in TAG_KEYS.iteritems():
                    if tag_keys_override and tag_keys_override.has_key(file_tag):
                        file_tag = tag_keys_override[file_tag]
                    if audio.has_key(file_tag) and audio[file_tag]:    
                        value = audio[file_tag]
                        if isinstance(value, list) or isinstance(value, tuple):
                            value = value[0]
                        fix_value = utils.fix_charset(value)    
                        if fix_value == "[Invalid Encoding]":
                            if tag == "title":
                                fix_value = self.get_filename()
                            else:    
                                fix_value = ""
                        self[tag] = fix_value
                            
                self["#duration"] = int(audio.info.length) * 1000        
                try:
                    self["#bitrate"] = int(audio.info.bitrate)
                except AttributeError: pass    
                
            
            else:    
                raise "W:Song:MutagenTag:No audio found"
            self.load_sort_object()    
        except Exception, e:    
            # print "W: Error while Loading (" + self.get_path() + ")\nTracback :", e
            self.last_error = "Error while reading" + ":" + self.get_filename()
            return False
        else:
            return True
        
    def load_sort_object(self):    
        for tag in ["artist", "title", "album", "genre"]:
            self["sort_%s" % tag ] = TransforDB.convert_first(self.get_str(tag).title())
            
        raw_text = "%s%s%s" % (self.get("title", ""), self.get("artist", ""), 
                               self.get("album", ""))
        raw_text = raw_text.lower().replace(" ", "")
        pinyin_text = TransforDB.convert(raw_text)
        self["search"] = "%s%s" % (pinyin_text, raw_text)
        

        
    def __read_from_remote_file(self):    
        ''' Load song information from remote file. '''
        
        GST_IDS = {"title"       : "title",
                   "genre"       : "genre",
                   "artist"      : "artist",
                   "album"       : "album",
                   "bitrate"     : "#bitrate",
                   'track-number':"#track"}
        is_finalize = False
        is_tagged = False
        
        def unknown_type(*param):
            raise "W:Song:GstTag:Gst decoder: type inconnu"
        
        def finalize(pipeline):
            state_ret = pipeline.set_state(gst.STATE_NULL)
            if state_ret != gst.STATE_CHANGE_SUCCESS:
                print "Failed change to null"
                
        def message(bus, message, pipeline):        
            if message.type == gst.MESSAGE_EOS:
                finalize(pipeline)
            elif message.type == gst.MESSAGE_TAG:    
                taglist = message.parse_tag()
                for key in taglist.keys():
                    if GST_IDS.has_key(key):
                        if key == "bitrate":
                            value = int(taglist[key] / 100)
                        elif isinstance(taglist[key], long):
                            value = int(taglist[key])
                        else:    
                            value = taglist[key]
                        self[GST_IDS[key]] = utils.fix_charset(value)
                        print key,":", utils.fix_charset(value)
                        
                is_tagged = True        
                
            elif message.type == gst.MESSAGE_ERROR:    
                err, debug = message.parse_error()
                finalize(pipeline)
                raise "W:Song:GstTag:Decoder error: %s\n%s" % (err,debug)
        try:    
            try:
                url = utils.get_uri_from_path(self.get("uri").encode("utf-8"))
                pipeline = gst.parse_launch("gnomevfssrc location="+url+" ! decodebin name=decoder ! fakesink")
            except gobject.GError:    
                raise "W:Song:GstTag:Failed to build pipeline to read metadata of",self.get("uri")
            
            decoder = pipeline.get_by_name("decoder")
            decoder.connect("unknown_type", unknown_type)
            bus = pipeline.get_bus()
            bus.connect("message", message, pipeline)
            bus.add_signal_watch()
            
            state_ret = pipeline.set_state(gst.STATE_PAUSED)
            timeout = 10
            while state_ret == gst.STATE_CHANGE_ASYNC and not is_finalize and timeout > 0:
                state_ret, _state, _pending_state = pipeline.get_state(1 * gst.SECOND)
                timeout -= 1
                
            if state_ret != gst.STATE_CHANGE_SUCCESS:    
                finalize(pipeline)
                print "W:Song:GstTag:Failed Read Media"
            else:    
                if not is_tagged:
                    bus.poll(gst.MESSAGE_TAG, 5 * gst.SECOND)
                try:    
                    query = gst.query_new_duration(gst.FORMAT_TIME)
                    if pipeline.query(query):
                        total = query.parse_duration()[1]
                    else: total = 0    
                except gst.QueryError: total = 0
                total //= gst.MSECOND
                self["#duration"] = total
                if not is_tagged:
                    print "W:Song:GstTag: Media found but no tag found" 
                finalize(pipeline)    
                
        except Exception, e:        
            print "W: Error while loading ("+self.get("uri")+")\nTracback :",e
            self.last_error = ("Error while reading") + ": " + self.get_filename()
            return False
        else:
            return True
        
    def write_to_file(self):    
        ''' Save tag information to file. '''
        if self.get_scheme() != "file":
            self.last_error = self.get_scheme() + " " + "Scheme not supported"
            return False
        if not utils.exists(self.get("uri")):
            self.last_error = self.get_filename() + " doesn't exist"
            return False
        if not os.access(self.get_path(), os.W_OK):
            self.last_error = self.get_filename() + " doesn't have enough permission"
            return False
        
        try:
            audio = common.MutagenFile(self.get_path(), common.FORMATS)
            tag_keys_override = None
            
            if audio is not None:
                if audio.tags is None:
                    audio.add_tags()
                tag_keys_override = TAGS_KEYS_OVERRIDE.get(audio.__class__.__name__, None)    
                
                for file_tag, tag in TAG_KEYS.iteritems():
                    if tag_keys_override and tag_keys_override.has_key(file_tag):
                        file_tag = tag_keys_override[file_tag]
                        
                    if self.get(tag):    
                        value = unicode(self.get(tag))
                        audio[file_tag] = value
                    else:    
                        try:
                            del(audio[file_tag]) # TEST
                        except KeyError:
                            pass
                        
                audio.save()        
                
            else:    
                raise "w:Song:MutagenTag:No audio found"
                
        except Exception, e:    
            # print traceback.format_exc()
            print "W: Error while writting ("+self.get("uri")+")\nTracback :",e
            self.last_error = "Error while writting" + ": " + self.get_filename()
            return False
        else:
            return True
    
