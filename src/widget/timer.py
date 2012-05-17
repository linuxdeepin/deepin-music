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

from dtk.ui.volume_button import VolumeButton
from dtk.ui.frame import HorizontalFrame
from dtk.ui.label import Label
from dtk.ui.utils import get_content_size

import utils
from widget.scalebar import HScalebar
from widget.ui import app_theme
from player import Player
from config import config

class SongTimer(gtk.HBox):
    __gsignals__ = {
        "play-end" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())
    }
    def __init__(self):
        super(SongTimer, self).__init__()

        self.label_time = Label("00:00/00:00", app_theme.get_color("labelText"), 9)
        self.label_time.set_size_request(*get_content_size("00:00/00:00", 9))

        self.bar = HScalebar()
        self.bar.set_draw_value(False)
        self.bar.set_range(0, 1000)
        self.bar.set_value(0)
        self.bar.connect("button_press_event", self.on_bar_press)
        self.bar.connect("button_release_event", self.on_bar_release)
        self.__value_changed_id = self.bar.connect("value-changed", self.on_bar_value_changed)
        self.bar.handler_block(self.__value_changed_id)
        self.pack_start(self.bar, True, True)
        self.update_bar = 1
        self.duration = 0
        self.__idle_release_id = None
        self.delete = False
        self.__need_report = False

        Player.connect("instant-new-song", self.set_duration)

        Player.bin.connect("tick", self.on_tick)
        Player.connect("seeked", self.on_seek)
        Player.connect("stopped", self.set_duration)
        if not Player.song:
            self.bar.set_sensitive(False)

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

        total = utils.duration_to_string(duration * 1000, "00:00")                
        if pos > 0 and pos < duration:
            current = utils.duration_to_string(pos, "00:00", 1) 
        else:    
            current = "00:00"
            
        text = "%s/%s" % (current, total)
        self.label_time.set_text(text)
        
        # if pos >= duration:        
        #     text = utils.duration_to_string(pos, "00:00", 1)
        # else:    

        #     current = utils.duration_to_string(pos, "00:00", 1)

    def on_seek(self, *args, **kwargs):
        self.__need_report = False
    
    def on_bar_value_changed(self, widget):        
        pos = self.bar.get_value()
        self.set_current_time(pos, self.duration)

    def on_bar_press(self, widget, event):
        if self.__idle_release_id:
            gobject.source_remove(self.__idle_release_id)
            self.__idle_release_id = None
        self.update_bar = 0
        self.__need_report = False

        self.bar.handler_unblock(self.__value_changed_id)

    def __idle_release(self):
        self.update_bar = 1
        self.__idle_release_id = None

    def on_bar_release(self, widget, event):
        self.bar.handler_block(self.__value_changed_id)
        
        s = Player.song
        if s is not None and s.get_type() in [ "webradio", "volatile_webradio"]:
            return
        Player.seek(self.bar.get_value())

        # wait a bit that the player are really seek to update the progress bar
        if not self.__idle_release_id:
            self.__idle_release_id = gobject.idle_add(self.__idle_release)

class VolumeSlider(gtk.HBox):
    def __init__(self):
        super(VolumeSlider, self).__init__()
        volume_button = VolumeButton(1.0, 0.0, 1.0, 2)
        volume_frame = HorizontalFrame(10, 0, 0, 0, 0)
        volume_frame.add(volume_button)
        self.volume_progressbar = volume_button.volume_progressbar
        self.mute_button = volume_button.volume_button
        self.mute_button.connect("toggled", self.toggled_volume)
        self.volume_progressbar.connect("value-changed",self.__volume_changed)
        volume = float(config.get("player","volume"))
        self.change_volume(None,volume)
        self.pack_start(volume_frame, False, False)

    def change_volume(self,helper,value):
        self.volume_progressbar.set_value(value)
        self.__volume_changed()
        
    def toggled_volume(self, widget):    
        val = self.volume_progressbar.get_value()
        val = (2 ** val) - 1
        if widget.get_active():
            Player.volume = val
        else:    
            Player.volume = 0.0

    def __volume_changed(self,*args):
        val = self.volume_progressbar.get_value()
        val = (2 ** val) - 1
        config.set("player","volume","%f" % val)
        Player.volume = val
        if val == 0.0:
            self.mute_button.set_active(False)
        else:    
            self.mute_button.set_active(True)
