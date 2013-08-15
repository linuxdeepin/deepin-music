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


import gobject    
from xdg_support import get_config_file
from ConfigParser import RawConfigParser as ConfigParser
from logger import Logger
from constant import CONFIG_FILENAME, FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT
from utils import glib_wait_seconds


class Config(gobject.GObject, Logger):
    __gsignals__ = {
        "config-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE,
                            (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_STRING))
        }
    
    def __init__(self):
        gobject.GObject.__init__(self)
        self._config       = ConfigParser()
        self.remove_option = self._config.remove_option
        self.has_option    = self._config.has_option
        self.add_section   = self._config.add_section
        self.getboolean    = self._config.getboolean
        self.getint        = self._config.getint
        self.getfloat      = self._config.getfloat
        self.options       = self._config.options
        
        # Load default configure.
        for section, items in self.__get_default().iteritems():
            self.add_section(section)
            for key, value in items.iteritems():
                self._config.set(section, key, value)
                
    def load(self):            
        ''' Load config items from the file. '''
        try:
            self._config.read(get_config_file(CONFIG_FILENAME))
            self._timeout_save()
        except:    
            pass
        
    @glib_wait_seconds(30)    
    def _timeout_save(self):    
        self.write()
        return True
    
    def get(self, section, option, default=""):
        ''' specified the section for read the option value. '''
        try:
            return self._config.get(section, option)
        except:
            return default
            
    def set(self, section, option, value):        
        if not self._config.has_section(section):
            self.logdebug("Section \"%s\" not exist. create...", section)
            self.add_section(section)
        self._config.set(section, option, value)    
        self.emit("config-changed", section, option, value)
        
    def write(self):    
        ''' write configure to file. '''
        filename = get_config_file(CONFIG_FILENAME)
        f = file(filename, "w")
        self._config.write(f)
        f.close()
        
    def __get_default(self):    
        return {
            "window" : {
                "x" : "-1",
                "y" : "-1",
                "width"  : str(FULL_DEFAULT_WIDTH),
                "height" : str(FULL_DEFAULT_HEIGHT),
                "state"  : "normal",
                },
            
            "mini" : {
                "x" : "-1",
                "y" : "-1",
                },
            
            "player" : {
                "uri"  : "",
                "play" : "false",
                "volume" : "1.0",
                "volume_mute" : "false",
                "seek" : "0",
                "state" : "stop",
                "crossfade" : "true",
                "crossfade_time" : "2.0",
                "crossfade_gapless_album" : "false",
                "play_on_startup" : "true",
                "resume_last_progress" : "false",
                "enqueue":"false",
                "click_enqueue":"false",
                "queuewholealbum":"false",
                "dynamic":"true",
                "vis":"goom",
                "stop_track":"-1",
                "selected_track":"-1"
                },

            "lyrics" : {
                "status" : "false",
                "mode" : "1",
                "save_lrc_path" : "~/.lyrics",
                "auto_download" : "true",
                "font_name" : "文泉驿微米黑",
                "font_type" : "Regular",
                "font_size" : "18",
                "locked" : "false",
                "dock_mode" : "true",
                "line_count" : "2",
                "blur_radius" : "2",
                "outline_width" : "2",
                "blur_color" : "#000000",
                "single_line_align"  : "centered",
                "double_line_align"  : "justified",
                "predefine_color" : "vitality_yellow",
                "inactive_color_upper" : "#e4dcb9",
                "inactive_color_middle" : "#ffea93",
                "inactive_color_bottom" : "#ffd631",
                "active_color_upper" : "#f7f4ea",
                "active_color_middle" : "#77d1ff",
                "active_color_bottom" : "#4199d5",
                "translucent_on_mouse_over" : "true",
                "karaoke_mode" : "true",
                "scroll_x" : "-1",
                "scroll_y" : "-1",
                "desktop_x" : "-1",
                "desktop_y" : "-1",
                },
            
            "scroll_lyrics" : {
                "font_name" : "文泉驿微米黑",
                "font_type" : "Regular",
                "font_size" : "10",
                "line_margin" : "1",
                "scroll_mode" : "0",
                "line_align" : "1",
                "inactive_color" : "#000000",
                "active_color" : "#00AEFF"
                },
            
            "setting" : {
                "empty_random" : "false",
                "loop_mode" : "list_mode",
                "offline" : "false",                
                "use_tray" : "true",
                "close_to_tray" : "true",
                "close_remember" : "false",
                "use_splash" : "true",
                "window_mode" : "simple",
                "app_mode" : "normal",
                },
            
            "playlist" : {
                "current_index" : "0",
                },
            
            "equalizer" : {
                "x" : "-1",
                },
            
            "globalkey" : {
                "enable" :          "false",
                "previous"           : "Alt + Left",
                "next"               : "Alt + Right",
                "playpause"          : "Alt + P",
                "increase_vol"        : "Alt + Up",
                "decrease_vol"        : "Alt + Down",
                "toggle_window"      : "Alt + W",
                "toggle_lyrics_lock" : "Alt + U",
                "toggle_lyrics_status" : "Alt + H",
                },
            
            "plugins" : {
                "enabled" : "mpris2",
                },
            
            "listmanager" : {
                "source" : "",
                }
            }
    
config = Config()    
