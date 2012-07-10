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


from dtk.ui.global_key import GlobalKey, enable_global_key, disable_global_key
from dtk.ui.threads import post_gui

from helper import Dispatcher
from player import Player
from logger import Logger
from config import config
import utils

def toggle_window():
    instance = utils.get_main_window()
    instance.toggle_window()
    
def change_lyrics_lock_status():    
    if config.getboolean("lyrics", "locked"):
        Dispatcher.unlock_lyrics()
    else:    
        Dispatcher.lock_lyrics()
        
def change_lyrics_status():        
    if config.getboolean("lyrics", "status"):
        Dispatcher.close_lyrics()
    else:    
        Dispatcher.show_lyrics()

class GlobalHotKeys(Logger):
    
    func = {
        "previous" : Player.previous,
        "next" : Player.next,
        "playpause" : Player.playpause,
        "increase_vol" : Player.increase_volume,
        "decrease_vol" : Player.decrease_volume,
        "toggle_window" : toggle_window,
        "toggle_lyrics_lock" : change_lyrics_lock_status,
        "toggle_lyrics_status" : change_lyrics_status,
        }
    
    def __init__(self):
        config.connect("config-changed", self.__on_config_changed)
        self.keybinder = GlobalKey()
        
        for field in self.func.keys():
            keystr = config.get("globalkey", field)
            if keystr:
                self.__bind(keystr, field)
            config.set("globalkey", "%s_last" % field, keystr)    
            
        self.keybinder.start()    
            
    @post_gui    
    def __handle_callback(self, text, callback):
        self.logdebug(text)
        callback()
    
    def __bind(self, key, field):
        try:
            self.__try_unbind(key)
        except:    
            pass
        
        self.keybinder.bind(key, lambda : self.__handle_callback(key, 
                                                             self.func[field]))
        
        self.logdebug("Bound %s" % key)
        
    def __try_unbind(self, key):
        try:
            self.logdebug("Unbinding %s" % key)
            self.keybinder.unbind(key)
            self.logdebug("Unbound %s" % key)
        except:    
            self.logdebug("Did not unbind %s" % key)

    def __on_config_changed(self, obj, section, option, value):        
        if section == "globalkey" and option.find("_last") == -1 and option != "enable":
            self.__try_unbind(config.get(section, option + "_last", value))
            
            if value:
                self.__bind(config.get(section, option, value), option)
                config.set(section, option + "_last", value)
                
        if section == "globalkey" and option == "enable":        
            if value == "true":
                self.play()
            else:    
                self.pause()
                
    def play(self):            
        enable_global_key()
        
    def pause(self):    
        disable_global_key()
        
    def stop(self):    
        self.keybinder.exit()
        
global_hotkeys = GlobalHotKeys()        
