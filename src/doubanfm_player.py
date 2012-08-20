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

import gobject
import urllib

from config import config
from helper import Dispatcher
from web_threading import WebFetchThread
from library import MediaDB
from logger import Logger


import utils

class DoubanFmBadUser(Exception):
    pass

class DoubanFmPlayerWrapper(gobject.GObject, Logger):
    __gsignals__ = {"new-song", (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (object,))}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        self.__debug = True
        self.download_thread = WebFetchThread(1024, self.__fail_fetch)
        self.download_thread.set_mode_queue(True)
        
        self.__metadata_lock = False
        self.song = None
        
    def __fail_fetch(self, *args):
        self.__metadata_lock = False
        self.loginfo("fail retrieve url %s", self.download_thread.last_url_fail)
