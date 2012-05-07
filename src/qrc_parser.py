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

import re
from xml.etree import ElementTree as ET

class LrcParser(object):
    def __init__(self, filename=""):
        self.attr_dict = {"offset" : 0}
        self.lyrics_dict = {}
        self.word_dict = {}
        self.sort_words_keys = None
        self.sort_lyrics_keys = None
        self.attr_pattern = re.compile(r'(.*?)\:(.*?)\]')
        self.time_pattern = re.compile(r'(\d+),(\d+)\](.*)')
        self.word_pattern = re.compile(r'(.*?)\((\d+),(\d+)\)')
        self._realy = False
        
    def set_filename(self, filename):    
        self._realy = False
        self.attr_dict = {"offset" : 0}
        self.lyrics_dict = {}
        self.word_dict = {}
        self.sort_words_keys = None
        self.sort_lyrics_keys = None
        self.parser_from_file(filename)
        
        
    def parser_from_file(self, filename):    
        if not filename:
            return 
        self.filename = filename
        try:
            xml_obj = ET.parse(filename)
            raw_buffer = xml_obj.find("LyricInfo/Lyric_1").get("LyricContent")
        except:    
            return
        else:
            self.parser_buffer(raw_buffer.encode("utf-8"))
        
                
    def parser_buffer(self, raw_buffer):            
        lyrics_list = raw_buffer.split('[')
        for item in lyrics_list:
            attr = self.attr_pattern.search(item)
            if attr:
                self.attr_dict[attr.group(1)] = attr.group(2)
                
            else:    
                time_p = self.time_pattern.search(item)
                if time_p:
                    word_p = self.word_pattern.findall(time_p.group(3))
                    line_word = "".join([word[0] for word in word_p])
                    line_start = int(time_p.group(1))
                    line_length = int(time_p.group(2))
                    self.lyrics_dict[line_start] = {}
                    self.lyrics_dict[line_start]["lyric"] = line_word
                    self.lyrics_dict[line_start]["length"] = line_length
                    for each_group in word_p:
                        word_start = int(each_group[1])
                        word_length = int(each_group[2])
                        self.word_dict[word_start] = {}
                        self.word_dict[word_start]["line_start"] = line_start
                        self.word_dict[word_start]["line_length"] = line_length
                        self.word_dict[word_start]["start"]  = word_start
                        self.word_dict[word_start]["length"]  = word_length
                        self.word_dict[word_start]["word"] = each_group[0]
                        
        if self.lyrics_dict and self.word_dict:                
            self.sort_lyrics_keys = sorted(self.lyrics_dict)
            self.sort_words_keys = sorted(self.word_dict)
            self.__realy = True
            
    def get_lyric_by_time(self, timestamp, duration):        
        if not self.__realy:
            return 
        if not self.__get_lyric(timestamp) or not self.__get_percentage(timestamp):
            return
        else:
            lyric, item_id = self.__get_lyric(timestamp)
            percentage = self.__get_percentage(timestamp)
            return (lyric,  percentage, item_id)
        
    def __get_percentage(self, timestamp):    
        timestamp = float(timestamp)
        left = 0
        right = len(self.sort_words_keys) - 1
        while left < right:
            middle = int((left + right) / 2 + 1)
            if self.get_word_time(middle) < timestamp:
                left = middle
            else:    
                right = middle -1
        if left == right:        
            word_item = self.get_word_item(left)
            word_percentage = (timestamp - word_item["start"]) / word_item["length"]
            line_percentage = (timestamp - word_item["line_start"]) / word_item["line_length"]
            print word_percentage, word_item['word']
            return  line_percentage
        return None
            
    def get_word_item(self, word_id):        
        return self.word_dict[self.sort_words_keys[word_id]]
        
    def get_word_time(self, word_id):        
        return self.word_dict[self.sort_words_keys[word_id]]["start"] - self.get_offset()
        
    def __get_lyric(self, timestamp):    
        lyric = ""
        left = 0
        right = len(self.sort_lyrics_keys) - 1
        
        while left < right:
            middle = int((left + right) / 2 + 1)
            if self.get_item_time(middle) < timestamp:
                left = middle
            else: right = middle - 1    
        if left == right:
            self.current_id = left
            lyric = self.get_item_lyric(left)
            return (lyric, left)
        return None
            
    def get_item_time(self, item_id):        
        return self.sort_lyrics_keys[item_id] - self.get_offset()
        
    def get_offset(self):    
        return int(self.attr_dict.get("offset", 0))
    
    def get_item_lyric(self, item_id):
        if not self.is_vaild_id(item_id):
            return None
        return self.lyrics_dict[self.sort_lyrics_keys[int(item_id)]]["lyric"]
    
    def get_next_id(self, item_id):
        next_id = item_id - 1
        if self.is_vaild_id(next_id):
            return next_id
        else:
            return -1
                    
    def get_item_count(self):    
        return len(self.sort_lyrics_keys) 
    
    def is_vaild_id(self, item_id):
        if item_id < 0 or item_id >= self.get_item_count():
            return False
        else:
            return True
                        

            
if __name__ == "__main__":
    import sys
    a = LrcParser()
    a.parser_from_file(sys.argv[1])

        
                
                
        
        
                    
                    
                    
            
        
        
        
