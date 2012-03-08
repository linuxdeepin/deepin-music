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

import utils
from player import Player
from config import config
from ui_toolkit import song_scalebar, VolumeButton, HorizontalFrame

class SongTimer(gtk.HBox):
    __gsignals__ = {
        "play-end" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())
    }
    def __init__(self):
        super(SongTimer, self).__init__()

        self.label_time = gtk.Label("<span size=\"small\" color=\"#A7A8A7\">00:00</span>")
        self.label_time.set_alignment(1, 1)
        self.label_time.set_use_markup(True)

        self.bar = song_scalebar
        self.bar.set_draw_value(False)
        self.bar.set_range(0, 1000)
        self.bar.set_value(0)
        # self.bar.set_increments(1, 20)
        self.bar.connect("button_press_event", self.on_bar_press)
        self.bar.connect("button_release_event", self.on_bar_release)
        self.__value_changed_id = self.bar.connect("value-changed", self.on_bar_value_changed)
        self.bar.handler_block(self.__value_changed_id)
        self.bar.set_size_request(250, -1)
        self.pack_start(self.bar, True, True)
        self.pack_start(self.label_time,False, False)
        self.update_bar = 1
        self.duration = 0
        self.__idle_release_id = None
        self.delete = False
        self.__need_report = False

        Player.connect("instant-new-song", self.set_duration)

        Player.bin.connect("tick", self.on_tick)
        Player.connect("seeked", self.on_seek)
        Player.connect("stopped", self.set_duration)

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

        if pos >= duration:
            text = utils.duration_to_string(pos, "00:00", 1)
        elif pos > 0:
            text = utils.duration_to_string(pos, "00:00", 1) 
        else:    
            text = "00:00"
        self.label_time.set_label("<span size=\"small\" color=\"#A7A8A7\">" + text + "</span>")


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
        volume_button = VolumeButton(1.0, 0.0, 1.0, True)
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
