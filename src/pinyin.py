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


from utils import load_db, fix_charset 
from logger import Logger
import os

PINYIN_DICT_FILE = os.path.join((os.path.dirname(os.path.realpath(__file__))), "data/chinese_dict.db")
SINGLE_CHARS = "'\"`~!@#$%^&*()=+[]{}\\|;:,.<>/?"
WIDTH_CHARS = "－—！#＃%％&＆（）*，、。：；？？　@＠＼{｛｜}｝~～‘’“”《》【】+＋=＝×￥·…　".decode("utf-8")

class Transfer(Logger):
    '''Chinese Transfer Pinyin.'''
    def __init__(self, first_spell=True, spliter="",):
        '''Init.'''
        self.spliter = spliter
        self.first_spell = first_spell
        # Load the db file.
        try:
            self.dict_objs = load_db(PINYIN_DICT_FILE)
        except:    
            self.logexception("Failed to load Library")
            self.dict_objs = {}
                
    def get_first_spell(self):            
        return self.first_spell
    
    def set_first_spell(self, value):
        self.first_spell = value
        
    def get_spliter(self):    
        return self.spliter
    
    def set_spliter(self, value):
        self.spliter = value
            
    def convert(self, chars):
        ''' Convert Unicode chinese_chars to PinYin. '''
        unicode_chars = unicode(chars)
        pinyin_list = [self.filter_char(char) for char in unicode_chars]
                
        return "".join(pinyin_list)
        
    def filter_char(self, unicode_char):    
        if not self.dict_objs:
            return unicode_char
        
        if unicode_char == ' ': return self.spliter
        if set(unicode_char).issubset(SINGLE_CHARS): return self.spliter
        if set(unicode_char).issubset(WIDTH_CHARS): return ""    
            
        if not self.dict_objs.has_key(unicode_char): return unicode_char
        else:
            if self.first_spell:
                return self.dict_objs.get(unicode_char, unicode_char)[:1]
            else: 
                return self.dict_objs.get(unicode_char, unicode_char)

def transfer(chars, first_spell=True):
    conv = Transfer(first_spell)    
    return conv.convert(chars)
        
