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

import  gtk
import gobject

from dtk.ui.label import Label

import dtk.ui.tooltip as Tooltip

import utils
from widget.scalebar import HScalebar
from widget.skin import app_theme
from widget.volume_button import VolumeButton
from player import Player
from config import config
from helper import Dispatcher

class SongTimer(gtk.HBox):
    __gsignals__ = {
        "play-end" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())
    }
    def __init__(self, draw_time_callback=None):
        super(SongTimer, self).__init__()

        self.label_time = Label("00:00", app_theme.get_color("labelText"), 8, enable_gaussian=True)
        self.draw_time_callback = draw_time_callback
        if draw_time_callback:
            draw_time_callback(self.label_time.get_text())
            
        self.bar = HScalebar()
        self.bar.set_draw_value(False)
        self.bar.set_range(0, 1000)
        self.bar.set_value(0)
        bar_align = gtk.Alignment()
        bar_align.set_padding(0, 0, 2, 2)
        bar_align.set(1, 1, 1, 1)
        bar_align.add(self.bar)
        self.bar.connect("button-press-event", self.on_bar_press)
        self.bar.connect("button-release-event", self.on_bar_release)
        self.__value_changed_id = self.bar.connect("value-changed", self.on_bar_value_changed)

        self.pack_start(bar_align, True, True)
        self.update_bar = 1
        self.duration = 0
        self.__idle_release_id = None
        self.delete = False
        self.__need_report = False
        self.press_flag = False

        Player.connect("instant-new-song", self.set_duration)
        Player.connect("init-status", self.on_player_init_status)

        Player.bin.connect("tick", self.on_tick)
        Player.connect("seeked", self.on_seek)
        Player.connect("stopped", self.set_duration)
        if not Player.song:
            self.bar.set_sensitive(False)
            
    def on_player_init_status(self, player):        
        self.label_time.set_text("00:00")
        self.bar.set_value(0)
        self.bar.set_sensitive(True)
        if self.draw_time_callback:
            self.draw_time_callback(self.label_time.get_text())
            

    def get_label(self):
        return self.label_time
    
    def stop(self):
        self.delete = True

    def set_duration(self, player, song=None):
        if not song:
            if player.song: song = player.song
            else: return
        else:
            self.__need_report = True
        
        self.duration = song.get("#duration", 0) / 1000
        self.set_current_time(0, self.duration)
        
    def on_tick(self, bin, pos, duration):
        if Player.song:
            if Player.song.get_type() == "webcast":
                duration = 0
        
        pos /= 1000.0
        duration /= 1000.0
        
        if Player.song and Player.song.get_type() == "cue":
            duration = Player.song.get("#duration") / 1000
            pos = pos - Player.song.get("seek", 0)
            
        self.duration = duration
        if self.update_bar == 1:
            self.set_current_time(pos, duration)
        return not self.delete

    def set_current_time(self, pos, duration):
        if self.update_bar == 1:
            if duration == 0 or pos >= duration:
                self.bar.set_range(0, 1000)
                self.bar.set_value(0)
                self.bar.set_sensitive(False)
            else :
                self.bar.set_sensitive(True)
                self.bar.set_range(0, duration)
                self.bar.set_value(pos)

        # total = utils.duration_to_string(duration * 1000, "00:00")                
        if pos > 0 and pos < duration:
            current = utils.duration_to_string(pos, "00:00", 1) 
        else:    
            current = "00:00"
            
        if pos >0 and Player.song and Player.song.get_type() == "webcast":    
            current = utils.duration_to_string(pos, "00:00", 1) 
            # total = "--:--"
        # text = "%s/%s" % (current, total)
        text = current
        self.label_time.set_text(text)
        if self.draw_time_callback:
            self.draw_time_callback(text)

    def on_seek(self, *args, **kwargs):
        self.__need_report = False
    
    def on_bar_value_changed(self, widget):        
        pos = self.bar.get_value()
        self.set_current_time(pos, self.duration)

    def on_bar_press(self, widget, event):
        if not self.press_flag:
            if self.__idle_release_id:
                gobject.source_remove(self.__idle_release_id)
                self.__idle_release_id = None
            self.update_bar = 0
            self.__need_report = False
            
            
            self.bar.disconnect(self.__value_changed_id)
            self.press_flag = True

    def __idle_release(self):
        self.update_bar = 1
        self.__idle_release_id = None

    def on_bar_release(self, widget, event):
        if self.press_flag:
            self.__value_changed_id = self.bar.connect("value-changed", self.on_bar_value_changed)
            
            s = Player.song
            if not s :
                return
            if s.get_type() in [ "webcast",]:
                return
            
            if s.get_type() == "cue":
                Player.seek(s.get("seek", 0) + self.bar.get_value())
            else:    
                Player.seek(self.bar.get_value())
            
            # wait a bit that the player are really seek to update the progress bar
            # if not self.__idle_release_id:
            self.__idle_release()
            self.__idle_release_id = gobject.idle_add(self.__idle_release)
            self.press_flag = False

class VolumeSlider(gtk.VBox):
    def __init__(self, auto_hide=True, mini_mode=False):
        super(VolumeSlider, self).__init__()
        self.volume_button = VolumeButton(auto_hide=auto_hide, mini_mode=mini_mode)
        Tooltip.custom(self.volume_button, self.get_tip_label).always_update(self.volume_button, True)
        self.volume_button.connect("volume-state-changed",self.__volume_changed)
        self.load_volume_config()
        Dispatcher.connect("volume", self.change_volume)
        self.add(self.volume_button)
        
    def load_volume_config(self):    
        save_volume = float(config.get("player","volume"))        
        volume_mute = config.getboolean("player", "volume_mute")
        if volume_mute:
            self.volume_button.set_value(int(save_volume * 100), False)            
            self.volume_button.set_mute()            
            Player.volume = 0
        else:    
            self.volume_button.set_value(int(save_volume * 100), False)            
            self.volume_button.unset_mute()
            Player.volume = save_volume
                    
    def get_tip_label(self):    
        return Label(str(int(self.volume_button.get_value())))
        
    def change_volume(self,helper,value):
        config.set("player", "volume_mute", "false")
        self.volume_button.unset_mute()
        val = value * 100
        self.volume_button.set_value(int(val))

    def __volume_changed(self, widget, value, mute_status):
        val = value / 100.0
        if mute_status:
            config.set("player", "volume_mute", "true")
            Player.volume = 0.0
        else:    
            config.set("player", "volume_mute", "false")
            config.set("player","volume","%0.1f" % val)
            Player.volume = val
