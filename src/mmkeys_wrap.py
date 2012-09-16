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

from logger import Logger
from player import Player


class MMKeys(Logger):
    def __init__(self):
        self.application = "DMusic"
        self.use = None
        if "DISABLE_DMUSIC_MMKEYS" not in os.environ:
            self.grab()
            
    def grab(self):        
        self.use = self.__use_gnome() or self.__use_mmkeys()
        self.loginfo("MMKeys mode: %s", self.use)
        
    def on_gnome_mmkey(self, app, key):    
        if app == self.application:
            if key == "Play":
                Player.playpause()
            if key == "PlayPause":    
                Player.playpause()
            if key == "Stop":    
                Player.stop()
            if key == "Next":    
                Player.next(True)
            if key == "Previous":    
                Player.previous()
                
    def __use_gnome(self):            
        
        try:        
            # Gnome >= 2.22.X
            import dbus
            bus = dbus.SessionBus()
            obj = bus.get_object("org.gnome.SettingsDaemon", "/org/gnome/SettingsDaemon/MediaKeys")
            self.__gnome = dbus.Interface(obj, "org.gnome.SettingsDaemon.MediaKeys")
            self.__gnome.GrabMediaPlayerKeys(self.application, 0)
            self.__gnome.connect_to_signal("MediaPlayerKeyPressed", self.on_gnome_mmkey)
        except:    
            try:
                # Gnome < 2.22.X                
                import dbus
                obj = bus.get_object('org.gnome.SettingsDaemon', '/org/gnome/SettingsDaemon')
                self.__gnome = dbus.Interface(obj, 'org.gnome.SettingsDaemon')
                self.__gnome.GrabMediaPlayerKeys(self.application, 0)
                self.__gnome.connect_to_signal("MediaPlayerKeyPressed", self.on_gnome_mmkey)
            except:    
                return None
        return "gnome"    
    
    def __use_mmkeys(self):
        try:                
            import mmkeys
        except:    
            return None
        
        self.__keys = mmkeys.MmKeys()
        self.__keys.connect("mm_prev", self.mm_previous)
        self.__keys.connect("mm_next", self.mm_next)
        self.__keys.connect('mm_stop', self.mm_play)
        self.__keys.connect("mm_playpause", self.mm_playpause)
        return 'mmkeys'
    
    def release(self):
        if self.use == "gnome":
            self.__gnome.ReleaseMediaPlayerKeys(self.application)
            
    def mm_play(self, a, b):        
        Player.play()
        
    def mm_next(self,a,b):
        Player.next(True)
        
    def mm_previous(self,a,b):
        Player.previous()

    def mm_playpause(self, a, b):    
        Player.playpause()