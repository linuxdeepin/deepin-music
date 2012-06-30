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

import keybinder


from helper import Dispatcher
from player import Player
from logger import Logger
from config import config
import utils

def toggle_window():
    instance = utils.get_main_window()
    instance.toggle_visible()
    
def change_lyrics_lock_status():    
    if config.getboolean("lyrics", "locked"):
        Dispatcher.unlock_lyrics()
    else:    
        Dispatcher.lock_lyrics()

class GlobalKeys(Logger):
    
    func = {
        "previous" : Player.previous,
        "next" : Player.next,
        "toggle_window" : toggle_window,
        "toggle_lyrics_lock" : change_lyrics_lock_status
        }
    
    def __init__(self):
        config.connect("config-changed", self.__on_config_changed)
        
        for field in self.func.keys():
            keystr = config.get("globalkey", field)
            if keystr:
                self.__bind(keystr, field)
            config.set("globalkey", "%s_last" % field, keystr)    
    
    def __handle_callback(self, text, callback):
        self.logdebug(text)
        callback()
    
    def __bind(self, key, field):
        try:
            self.__try_unbind(key)
        except:    
            pass
        
        keybinder.bind(key, lambda(text): self.__handle_callback(text, 
                                                             self.func[field]),
                       "Global binding for %s pressed (%s)" % (field, key))
        
        self.logdebug("Bound %s" % key)
        
    def __try_unbind(self, key):
        try:
            self.logdebug("Unbinding %s" % key)
            keybinder.unbind(key)
            self.logdebug("Unbound %s" % key)
        except:    
            self.logdebug("Did not unbind %s" % key)

    def __on_config_changed(self, obj, section, option, value):        
        if section == "globalkey" and option.find("_last") == -1:
            self.__try_unbind(config.get(section, option + "_last", value))
            
            if value:
                self.__bind(config.get(section, option, value), option)
                config.set(section, option + "_last", value)
