#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2013 Deepin, Inc.
#               2011 ~ 2013 Hou ShaoHui
# 
# Author:     Hou ShaoHui <houshao55@gmail.com>
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


import keybinder

from dtk.ui.keymap import deepin_to_keybinder

from constant import PROGRAM_NAME_LONG
from helper import Dispatcher
from player import Player
from logger import Logger
from config import config
from dbus_notify import dbus_notify
from nls import _
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
        
        self.bind_flag = False
            
    def start_bind(self):        
        for field in self.func.keys():
            keystr = config.get("globalkey", field)
            if keystr and keystr != "None":
                self.__bind(keystr, field)
            config.set("globalkey", "%s_last" % field, keystr)    
        
    def __handle_callback(self, text, callback):
        self.logdebug(text)
        callback()
    
    def __bind(self, raw_key, field):
        key = deepin_to_keybinder(raw_key)
        try:
            self.__try_unbind(key)
        except:    
            pass
        
        try:
            result = keybinder.bind(key, lambda : self.__handle_callback(key, self.func[field]))
        except:    
            result = False
        else:    
            if not result:
                dbus_notify.set_summary(PROGRAM_NAME_LONG)
                dbus_notify.set_body(_("Failed to bind %s !") % utils.xmlescape(raw_key))
                dbus_notify.notify()
        return result    
        
    def __try_unbind(self, key):
        key = deepin_to_keybinder(key)
        try:
            self.logdebug("Unbinding %s" % key)
            keybinder.unbind(key)
            self.logdebug("Unbound %s" % key)
        except:    
            self.logdebug("Did not unbind %s" % key)

    def __on_config_changed(self, obj, section, option, value):        
        if section == "globalkey" and option.find("_last") == -1 and option != "enable":
            self.__try_unbind(config.get(section, option + "_last", value))
            
            if value:
                if value != "None":
                    try:
                        self.__bind(config.get(section, option, value), option)
                    except: pass
                    
                config.set(section, option + "_last", value)
                
        if section == "globalkey" and option == "enable":        
            if value == "true":
                self.start_bind()                
            else:    
                self.stop_bind()
                
    def stop_bind(self):            
        for field, _ in self.func.items():
            key = config.get("globalkey", field, "")
            if key:
                self.__try_unbind(key)
        
global_hotkeys = GlobalHotKeys()        
