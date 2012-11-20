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
import threading
import urllib2

from StringIO import StringIO
from time import time, sleep
from hashlib import md5

from xdg_support import get_cache_file, get_cache_dir

THREAD_INSTANCE = []
WEBSERVICE_CACHE_EXPIRATION = 60 * 60 * 24
TIMEOUT = 30

class WebFetchThread(object):
    
    def __init__(self, buffer_len, func_fail_cb, *args_fail_cb):
        self.id = len(THREAD_INSTANCE)
        THREAD_INSTANCE.append(self)
        self.buffer_len = buffer_len
        self.func_fail_cb = func_fail_cb
        self.args_fail_cb = args_fail_cb
        
        self.condition = threading.Condition()
        t = threading.Thread(target=self.fetch)
        self.setDaemon(True)
        t.start()
        
        self.cache = {}
        self.mode_queue = False
        self.mode_cache = True
        self.last_url_fail = None
        self.clean_expired_cache()

    def set_mode_queue(self, value):    
        self.mode_queue = value
        if value:
            self.info = []
        else:    
            self.info = None
            
    def set_mode_cache(self, mode):        
        self.mode_cache = mode
        
    def fetch_url(self, url, func_cb, *args_cb, **kargs):
        self.condition.acquire()
        self.info = (func_cb, args_cb, kargs.get("func_fail"), kargs.get("func_fail_args"))
        self.condition.notify()    
        self.condition.release()
        
    def fetch(self):            
        while True:
            self.condition.acquire()
            while not self.info:
                self.print_dbg("wait url")
                self.condition.wait()
                
            info = self.info
            self.info = None
            self.condition.release()
            
                
    def print_dbg(self,*args):
        if self.dbg: print "DBG:WebFetchThread:id=",self.id,":",args
