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

import chardet

class LrcParser(object):
    
    def __init__(self, filename=""):
        self.filename = ""
        self.lyrics_dict = {}
        self.parser_offset = 0
        self.nlyrics = 0
        self.tmp_lyrics_dict = {}
        self.time_dict = {}
        self.attr_dict = {"offset" : 0}
        self.current_id = 0
                
    def set_filename(self, filename):    
        self.filename = ""
        self.lyrics_dict = {}
        self.scroll_lyrics = []        
        self.parser_offset = 0
        self.nlyrics = 0
        self.tmp_lyrics_dict = {}
        self.time_dict = {}
        self.attr_dict = {"offset" : 0}
        self.current_id = 0
        self.parser_from_file(filename)
        
    def parser_from_file(self, filename):    
        if not filename:
            return 
        self.filename = filename
        try:
            fp = file(self.filename, "r")
            raw_buffer = fp.read()
            fp.close()
        except:
            pass
        else:
            convert_buffer = self.parser_code(raw_buffer)
            if convert_buffer:
                try:
                    while self.parser_buffer(convert_buffer) != "over":
                        self.parser_buffer(convert_buffer)
                    self.parse_lyrics()    
                except:    
                    return 
            
    def parser_code(self, raw_buffer):        
        try:
            raw_encoding = chardet.detect(raw_buffer)["encoding"]
        except:    
            return ""
        else:
            if raw_encoding.strip().lower() in ["utf8", "utf-8"]:
                return raw_buffer
            else:
                try:
                    return raw_buffer.decode(raw_encoding).encode("utf-8")
                except:
                    return ""
                
    def parser_buffer(self, convert_buffer):            
        if self.parser_offset == len(convert_buffer):
            return "over"
        tag = 0
        start = self.parser_offset
        if convert_buffer[self.parser_offset] == "[":
            tag = 1
            self.parser_offset += 1
            
        while self.parser_offset < len(convert_buffer):    
            if tag and convert_buffer[self.parser_offset] == "]":
                # A tag found
                tag = 2
                self.parser_offset += 1
                break
            elif convert_buffer[self.parser_offset] in ['\r', '\n']:
                self.parser_offset += 1
                break
            self.parser_offset += 1
            
        if tag == 2:        
            self.parse_tag(convert_buffer, start, self.parser_offset)
            
        else:    
            end = self.parser_offset
            if convert_buffer[end - 1] in ['\r', '\n']:
                end -= 1
            if self.parser_offset < len(convert_buffer) \
                    and self.parser_offset > 0  \
                    and convert_buffer[self.parser_offset - 1] == '\r' \
                    and convert_buffer[self.parser_offset] == '\n':
                self.parser_offset += 1
                
            self.parse_text(convert_buffer, start, end)    
            
    def parse_text(self, lrc_buffer, start, end):        
        self.tmp_lyrics_dict[self.nlyrics] = lrc_buffer[start:end]
        self.nlyrics += 1
        
    def parse_tag(self, lrc_buffer, begin, end):
        istime = True
        begin += 1
        end -= 1
        for i in range(begin, end):
            if not lrc_buffer[i].isdigit() and lrc_buffer[i] not in [":", "."]:
                istime = False
                break
        if istime:    
            time_list = lrc_buffer[begin:end].split(":")
            if len(time_list) == 2:
                timestamp = (float(time_list[0]) * 60 + float(time_list[1])) * 1000 + 0.5
            elif len(time_list) == 3:    
                timestamp = ((float(time_list[0]) * 60 + float(time_list[1])) * 60 + float(time_list[2])) * 1000 + 0.5
            else:    
                timestamp = 0.0
            self.time_dict[timestamp] = self.nlyrics 
        else:    
            try:
                attr = lrc_buffer[begin:end].split(":")
                if attr[0] == "offset":
                    self.attr_dict[attr[0]] = float(attr[1])
                else:    
                    self.attr_dict[attr[0]] = attr[1]
            except:        
                pass
            
    def parse_lyrics(self):        
        if not self.time_dict:
            return
        for key, value in self.time_dict.iteritems():
            self.lyrics_dict[key] = self.tmp_lyrics_dict.get(value, "").strip()
        self.sort_keys = sorted(self.lyrics_dict)    
        self.scroll_lyrics = [self.lyrics_dict[key] for key in self.sort_keys]
        del self.tmp_lyrics_dict
        del self.time_dict
            
    def get_lyric_by_time(self, timestamp, duration):        
        if not self.lyrics_dict or not timestamp or not duration:
            return
        left = 0 
        right = len(self.sort_keys) - 1
        while left < right:
            middle = int((left + right) / 2 + 1)
            if self.get_item_time(middle) < timestamp:
                left = middle
            else:    
                right = middle - 1
        if left == right:        
            self.current_id = left
            c_text = self.get_item_lyric(left)
            c_stamp = self.get_item_time(left)
            if left + 1 <= len(self.sort_keys) - 1:
                nextstamp = self.get_item_time(left + 1)
            else:    
                nextstamp = duration
            percentage = (timestamp - c_stamp) /(nextstamp - c_stamp) 
            return (c_text, percentage , left)
        
    def get_item_count(self):
        return len(self.lyrics_dict)
    
    def get_item_lyric(self, item_id):
        if not self.is_vaild_id(item_id):
            return None
        return self.lyrics_dict[self.sort_keys[item_id]]
    
    def is_vaild_id(self, item_id):
        if item_id < 0 or item_id >= self.get_item_count():
            return False
        else:
            return True
        
    def get_item_time(self, item_id):    
        if not self.is_vaild_id(item_id):
            return
        return self.sort_keys[item_id] - self.attr_dict.get("offset", 0)
    
    def get_next_id(self, item_id):
        next_id = item_id - 1
        if self.is_vaild_id(next_id):
            return next_id
        else:
            return -1
        
    def set_offset(self, offset):    
        if not offset:
            return
        self.attr_dict["offset"] = self.attr_dict.get("offset", 0) + offset
        if self.filename:
            self.save_to_file()
            
    def get_offset(self):        
        return self.attr_dict.get("offset", 0)
        
    def __save_attr(self, fp):    
        if not self.attr_dict:
            return 
        for key, value in self.attr_dict.iteritems():
            if value == None:
                fp.write('[%s]\n' % key)
            else:    
                fp.write('[%s:%s]\n' % (key, value))
                
    def __save_lyrics(self, fp):            
        if not self.sort_keys:
            return
        for timestamp in self.sort_keys:
            h = timestamp / 1000 / 60 / 60
            m = timestamp / 1000 / 60 % 60
            s = timestamp / 1000 % 60
            ms = timestamp / 10 % 100
            if int(h) != 0:
                fp.write("[%02d:%02d:%02d.%02d]" % (h, m, s, ms))
            else:    
                fp.write("[%02d:%02d.%02d]" % (m, s, ms))
            fp.write("%s\n" % self.lyrics_dict.get(timestamp, ""))    
            
    def save_to_file(self):        
        if not self.filename:
            return
        fp = None
        try:
            fp = file(self.filename, "w")
        except:    
            pass
        else:
            self.__save_attr(fp)
            self.__save_lyrics(fp)
        finally:    
            if fp is not None:
                fp.close()
