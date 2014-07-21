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


from utils import load_db
import os


class Transfer(object):
    '''Chinese Transfer Pinyin.'''
    PINYIN_DICT_FILE = os.path.join((os.path.dirname(os.path.realpath(__file__))), "data/chinese_dict.db")
    SINGLE_CHARS = "'\"`~!@#$%^&*()=+[]{}\\|;:,.<>/?"
    WIDTH_CHARS = "－—！#＃%％&＆（）*，、。：；？？　@＠＼{｛｜}｝~～‘’“”《》【】+＋=＝×￥·…　".decode("utf-8")

    def __init__(self, spliter=""):
        '''Init.'''
        self.spliter = spliter
        
    def load(self):
        # Load the db file.
        try:
            self.dict_objs = load_db(self.PINYIN_DICT_FILE)
        except:    
            self.dict_objs = {}
        
    def get_spliter(self):    
        return self.spliter
    
    def set_spliter(self, value):
        self.spliter = value
        
    def to_unicode(self, chars):    
        try:
            if not isinstance(chars, unicode):
                return unicode(chars, "utf-8")
            else:
                return chars
        except:    
            return chars
        
            
    def convert_full(self, chars):
        unicode_chars = self.to_unicode(chars)
        pinyin_list = [self.filter_char(char, False) for char in unicode_chars]
        return "".join(pinyin_list)
    
    def convert_first(self, chars):
        ''' Convert Unicode chinese_chars to PinYin. '''
        unicode_chars = self.to_unicode(chars)
        pinyin_list = [self.filter_char(char) for char in unicode_chars]
        return "".join(pinyin_list)
    
    def convert(self, chars):
        result = self.convert_first(chars) + self.convert_full(chars)
        return result
        
    def filter_char(self, unicode_char, first_spell=True):    
        if not self.dict_objs:
            return unicode_char.encode("utf-8", "ignore")
        
        if unicode_char == ' ': return self.spliter
        if set(unicode_char).issubset(self.SINGLE_CHARS): return self.spliter
        if set(unicode_char).issubset(self.WIDTH_CHARS): return ""    
            
        if not self.dict_objs.has_key(unicode_char): return unicode_char.encode("utf-8", "ignore")
        else:
            if first_spell:
                return self.dict_objs.get(unicode_char, unicode_char)[:1].encode("utf-8", "ignore")
            else: 
                return self.dict_objs.get(unicode_char, unicode_char).encode("utf-8", "ignore")

TransforDB = Transfer()        
 