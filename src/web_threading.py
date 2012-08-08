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
        
        self.dbg = False
        self.info = None
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
        self.print_dbg("demand fetch url: ", url)
        self.condition.acquire()
        if self.mode_queue:
            self.info.append((url, func_cb, args_cb, kargs.get("func_fail"), kargs.get("func_fail_args")))
        else:    
            self.info = (url, func_cb, args_cb, kargs.get("func_fail"), kargs.get("func_fail_args"))
        self.condition.notify()    
        self.condition.release()
        
    def get_cache_path(self, url):    
        return get_cache_file("web/%s" % md5(url).hexdigest())
    
    def write_to_cache(self, url, data):
        f = open(self.get_cache_path(url), "w+")
        f.write(data)
        f.close()
        
    def url_in_cache(self, url):    
        if not self.mode_cache:
            return False
        path = self.get_cache_path(url)
        if not os.path.exists(path):
            return False
        elif time() - os.stat(path).st_mtime < WEBSERVICE_CACHE_EXPIRATION:
            return True
        else:
            try:
                os.unlink(path)
            except:    
                pass
            return False
        
    def read_from_cache(self, url):    
        path = self.get_cache_path(url)
        os.utime(path, None)
        f = open(path)
        data = f.read()
        f.close()
        return data
    
    def clean_expired_cache(self):
        path_dir = get_cache_dir("web")
        for name in os.listdir("web"):
            path = os.path_join(path_dir, name)
            if time() - os.stat(path).st_mtime >= WEBSERVICE_CACHE_EXPIRATION:
                try:
                    os.unlink(path)
                except:    
                    pass
                
    def fetch(self):            
        while True:
            self.condition.acquire()
            while not self.info:
                self.print_dbg("wait url")
                self.condition.wait()
                
            if self.mode_queue:    
                info = self.info.pop(0)
            else:    
                info = self.info
                self.info = None
                
            url = info[0]    
            if url.rfind("#") != -1:
                url = url[:url.rfind("#")]
            self.print_dbg("url_demanded: ", url)
            
            if not self.url_in_cache(url):
                self.condition.release()
                f = []
                def sub_thread(url, f):
                    self.print_dbg("open url: ", url)
                    try:
                        f.append(urllib2.urlopen(url))
                    except:    
                        f = []
                        
                t = threading.Thread(target=sub_thread, args=(url, f))        
                t.setDaemon(True)
                t.start()
                
                self.print_dbg("wait open url")
                start = time()
                self.condition.acquire()
                while self.mode_queue or self.info == None and time() - start < TIMEOUT and t.isAlive():
                    self.condition.release()
                    sleep(0.5)
                    self.condition.acquire()
                self.condition.release()    
                
                if f == [] or not f[0]:
                    self.print_dbg("failed connect to:", url)
                    self.condition.acquire()
                    if self.mode_queue or self.info == None:
                        if info[3]:
                            if info[4]:
                                args = info[4]
                            else:    
                                args = tuple()
                            gobject.idle_add(info[3], *args)    
                        else:    
                            self.last_url_fail = url
                            gobject.idle_add(self.func_fail_cb, *self.args_fail_cb)
                    self.condition.release()        
                    continue
                else:
                    f = f[0]
                    
                self.print_dbg("connected to:", url)
                
                f_data = StringIO()
                data = f.read(self.buffer_len)
                f_data.write(data)
                
                self.condition.acquire()
                while data and self.mode_queue or self.info == None:
                    self.condition.release()
                    data = f.read(self.buffer_len)
                    f_data.write(data)
                    self.condition.acquire()
                    
                f.clse()    
                if not self.info and self.mode_cache:
                    self.write_to_cache(url, f_data.getvalue())
            else:        
                self.print_dbg("cache used")
                f_data = StringIO()
                f_data.write(self.read_from_cache(url))
                
            f_data.seek(0)
            if self.mode_queue or self.info==None: 
                self.print_dbg("send retrieved data")
                gobject.idle_add(info[1],f_data,*info[2])
                self.condition.release()
            else:
                self.condition.release()
                self.print_dbg("retrived data break")  
                
    def print_dbg(self,*args):
        if self.dbg: print "DBG:WebFetchThread:id=",self.id,":",args
