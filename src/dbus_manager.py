# -*- coding: utf-8 -*-
#! /usr/bin/env python

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


import sys
import dbus
import dbus.service

from logger import Logger
from player import Player
from helper import Dispatcher
import utils

OBJECT_PATH = "/com/linuxdeepin/DMusic"
SERVICE_NAME = "com.linuxdeepin.DMusic"

def check_dbus(bus, interface):
    obj = bus.get_object('org.freedesktop.DBus', '/org/freedesktop/DBus')
    dbus_iface = dbus.Interface(obj, 'org.freedesktop.DBus')
    avail = dbus_iface.ListNames()
    return interface in avail



def check_exit(options, args):
    iface = None
    if not options.NewInstance:
        bus = dbus.SessionBus()
        if check_dbus(bus, SERVICE_NAME):
            remote_object = bus.get_object(SERVICE_NAME, OBJECT_PATH)
            iface = dbus.Interface(remote_object, SERVICE_NAME)
            iface.TestService("testing dbus service")
            if args:
                if args[0] == "-":
                    args = sys.stdin.read().split("n")
                iface.Enqueue(args)
                
    if not iface:            
        for command in [ "GetArtist", "GetTitle", "GetAlbum", "GetLength", "GetPath",
                         "Next", "Prev", "PlayPause", "Forward", "Rewind", "Stop", "ChangeVolume",
                         "CurrentPosition", "GuiToggleVisible",
                         ]:
            if getattr(options, command):
                return "command"
        return "continue"    
    
    run_commands(options, iface)
    return "exit"

def run_commands(options, iface):
    '''
        Actually invoke any commands passed in.
    '''
    comm = False
    info_commands = [ "GetArtist", "GetTitle", "GetAlbum", "GetLength", "GetPath" ]
    for command in info_commands:
        if getattr(options, command):
            value = getattr(iface, command)()
            print value
            comm = True    
            
            
    control_commands = [ "Next", "Prev", "PlayPause", "Forward", "Rewind", "GuiToggleVisible" ]       
    for command in control_commands:
        if getattr(options, command):
            getattr(iface, command)()
            comm = True
            
            
    volume_commands = [ "ChangeVolume" ]        
    for command in volume_commands:
        value = getattr(options, command)
        if value:
           iface.ChangeVolume(value)
            
    status_commands = [ "CurrentPosition" ]
    for command in status_commands:
        if getattr(options, command):
            print getattr(iface, command)()
            comm = True
            
    if not comm:        
        iface.GuiToggleVisible()
    
class DeepinMusicDBus(dbus.service.Object, Logger):
    
    def __init__(self):
        self.bus = dbus.SessionBus()
        self.bus_name = dbus.service.BusName(SERVICE_NAME, bus=self.bus)
        dbus.service.Object.__init__(self, self.bus_name, OBJECT_PATH)

    @dbus.service.method(SERVICE_NAME, "s")
    def TestService(self, arg):
        '''
            Just test the dbus object.
        '''
        self.loginfo(arg)
        
    @dbus.service.method(SERVICE_NAME)
    def Quit(self):
        Dispatcher.quit()
        return "Successful command "
        
    @dbus.service.method(SERVICE_NAME, None, "b")
    def IsPlaying(self):    
        return not Player.is_paused()
    
    @dbus.service.method(SERVICE_NAME)
    def Prev(self):
        Player.previous()
        return "Successful command "
    
    @dbus.service.method(SERVICE_NAME)
    def Forward(self):
        Player.forward()
        return "Successful command "
    
    @dbus.service.method(SERVICE_NAME)
    def Rewind(self):
        Player.rewind()
        return "Successful command "
    
    @dbus.service.method(SERVICE_NAME)
    def Next(self):
        Player.next(True)
        return "Successful command "
    
    @dbus.service.method(SERVICE_NAME)
    def PlayPause(self):
        Player.playpause()
        return "Successful command "

    @dbus.service.method(SERVICE_NAME)
    def Stop(self):
        Player.stop()
        return "Successful command "
        
    @dbus.service.method(SERVICE_NAME, None, "s")
    def GetPath(self):
        if not Player.is_paused():
            song = Player.song    
            return song.get_path()
        else:
            return ""
        
    @dbus.service.method(SERVICE_NAME, None, "s")
    def GetTitle(self):
        if not Player.is_paused():
            song = Player.song
            return song.get_str("title")
        else:
            return ""

    @dbus.service.method(SERVICE_NAME, None, "s")
    def GetArtist(self):
        if not Player.is_paused():
            song = Player.song
            return song.get_str("artist")
        else:
            return ""
    
    @dbus.service.method(SERVICE_NAME, None, "s")
    def GetUri(self):
        if not Player.is_paused():
            song = Player.song
            return song.get_str("uri")
        else:
            return ""

    @dbus.service.method(SERVICE_NAME, None, "s")
    def GetAlbum(self):
        if not Player.is_paused():
            song = Player.song
            return song.get_str("album")
        else:
            return ""
        
    @dbus.service.method(SERVICE_NAME, None, "i")
    def GetLength(self):
        return Player.get_length()

    @dbus.service.method(SERVICE_NAME, None, "i")
    def CurrentPosition(self):
        return Player.get_position()
    
    @dbus.service.method(SERVICE_NAME, "d", "b")
    def ChangeVolume(self, value):
        try: value = float(value)
        
        except:
            return False
        else:
            Dispatcher.volume(value)
            return True
        
    @dbus.service.method(SERVICE_NAME, 's')
    def PlayFile(self, filename):
        """
            Plays the specified file
        """
        app_instance = utils.get_main_window()
        current_view = app_instance.playlist_ui.get_selected_song_view()
        app_instance.list_manager.switch_to_local()
        if current_view:
            current_view.add_file(filename, True)
            
    @dbus.service.method(SERVICE_NAME, 'as')
    def Enqueue(self, locations):        
        app_instance = utils.get_main_window()
        current_view = app_instance.playlist_ui.get_selected_song_view()
        app_instance.list_manager.switch_to_local()        
        if current_view:
            uris = utils.convert_args_to_uris(locations)                
            current_view.play_uris(uris)
            # current_view.async_add_uris(uris)

    @dbus.service.method(SERVICE_NAME, None, "s")
    def DumpGstplayerState(self):
        return Player.bin.dump_state()
    
    @dbus.service.method(SERVICE_NAME)
    def GuiToggleVisible(self):
        app_instance = utils.get_main_window()
        app_instance.toggle_visible(True)
