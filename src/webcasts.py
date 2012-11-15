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


import os
import gobject
from utils import load_db
from xdg_support import get_config_file

class WebcastsDatabase(gobject.GObject):
    __gsignals__ = {"loaded" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        self.raw_db_file = os.path.join((os.path.dirname(os.path.realpath(__file__))), "data", "webcasts.db")
        self.favorite_db_file = get_config_file("favorite_webcasts.db")
        self.custom_db_file = get_config_file("custom_webcasts.db")
        self.__is_loaded = False
    
    def load(self):
        try:
            self.raw_db_objs = load_db(self.raw_db_file)
        except:    
            self.raw_db_objs = None
            
        try:    
            self.favorite_db_objs = load_db(self.favorite_db_file)
        except:    
            self.favorite_db_objs =None
            
        try:    
            self.custom_db_objs = load_db(self.custom_db_file)
        except:    
            self.custom_db_objs = None
            
        gobject.idle_add(self.__delay_post_load)    

        
    def isloaded(self):    
        return self.__is_loaded
        
    def __delay_post_load(self):    
        self.__is_loaded = True
        self.emit("loaded")
            
    def get_keys_from_categroy(self, categroy):        
        if not self.raw_db_objs:
            return []
        else:
            return self.raw_db_objs.get(categroy, {}).keys()
        
    def get_items(self, categroy, key):    
        if not self.raw_db_objs:
            return []
        else:
            return self.raw_db_objs.get(categroy, {}).get(key, [])
        
    def get_custom_items(self):    
        if not self.custom_db_objs:
            return []
        return self.custom_db_objs
    
    def get_favorite_items(self):
        if not self.favorite_db_objs:
            return []
        else:
            return self.favorite_db_objs
        
    def is_collected(self, uri):    
        if not self.favorite_db_objs:
            return False
        for obj in self.favorite_db_objs:
            if obj.get("uri", "") == uri:
                return True
        return False    
        
    def save(self):    
        pass
        
WebcastsDB = WebcastsDatabase()        
