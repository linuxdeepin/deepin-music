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


import gtk
from dtk.ui.button import ToggleButton, ImageButton
import dtk.ui.tooltip as Tooltip

from player import Player
from widget.information import PlayInfo
from widget.timer import SongTimer, VolumeSlider

from widget.cover import PlayerCoverButton
from widget.skin import app_theme
from config import config
from helper import Dispatcher
from nls import _
from widget.ui_utils import create_left_align, create_right_align, set_widget_vcenter

class SimpleHeadbar(gtk.EventBox):            
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        # init.
        self.cover_box = PlayerCoverButton()
        self.cover_box.show_all()
        
        # Main table
        main_table = gtk.Table(2, 2)
                
        # swap played status handler
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        
        # play button
        play_normal_pixbuf = app_theme.get_pixbuf("action/play_normal.png")
        pause_normal_pixbuf = app_theme.get_pixbuf("action/pause_normal.png")
        play_hover_pixbuf = app_theme.get_pixbuf("action/play_hover.png")
        pause_hover_pixbuf = app_theme.get_pixbuf("action/pause_hover.png")
        play_press_pixbuf = app_theme.get_pixbuf("action/play_press.png")
        pause_press_pixbuf = app_theme.get_pixbuf("action/pause_press.png")
       
        self.__play = ToggleButton(play_normal_pixbuf, pause_normal_pixbuf,
                                   play_hover_pixbuf, pause_hover_pixbuf,
                                   play_press_pixbuf, pause_press_pixbuf,
                                   )
        Tooltip.text(self.__play, _("Play/Pause"))
        self.__play.show_all()
        self.__id_signal_play = self.__play.connect("toggled", self.on_player_playpause)
        prev_button = self.__create_button("previous", _("Previous track"))
        next_button = self.__create_button("next", _("Next track"))
        
        self.vol = VolumeSlider()
        song_timer = SongTimer()
        
        # lyrics button
        self.lyrics_button = self.__create_simple_toggle_button("lyrics", self.change_lyrics_status)        
        Tooltip.text(self.lyrics_button, _("Lyrics on/off"))
        
        
        
        # action_box
        action_box = gtk.HBox()
        action_box.pack_start(set_widget_vcenter(self.lyrics_button), False, False)
        action_box.pack_start(set_widget_vcenter(prev_button), False, False)
        action_box.pack_start(set_widget_vcenter(self.__play), False, False)
        action_box.pack_start(set_widget_vcenter(next_button), False, False)
         
        # combo_box
        combo_vbox = gtk.VBox()
        combo_vbox.pack_start(PlayInfo(200), False, False)
        combo_vbox.pack_start(action_box, False, False)
        
        cover_main_box = gtk.HBox(spacing=5)
        cover_main_box.pack_start(self.cover_box, False, False)
        cover_main_box.pack_start(combo_vbox, False, False)
        
        body_vbox = gtk.VBox(spacing=5)
        body_vbox.pack_start(cover_main_box, False, True)
        body_vbox.pack_start(song_timer, False, True)
        
        self.add(body_vbox)

        # right click
        self.connect("button-press-event", self.right_click_cb)
        Dispatcher.connect("close-lyrics", lambda w : self.lyrics_button.set_active(False))
        Dispatcher.connect("show-lyrics", lambda w: self.lyrics_button.set_active(True))
 
        self.signal_auto = False        
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        self.signal_auto = True    
        
    def on_player_playpause(self, widget):    
        if Player.song:
            Player.playpause()
        
    def change_lyrics_status(self, widget):    
        if self.signal_auto:
            if widget.get_active():
                Dispatcher.show_lyrics()
            else:    
                Dispatcher.close_lyrics()
        
    def __create_simple_toggle_button(self, name, callback): 
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("lyrics_button/%s_inactive_normal.png" % name),
            app_theme.get_pixbuf("lyrics_button/%s_active_normal.png" % name),
            app_theme.get_pixbuf("lyrics_button/%s_inactive_hover.png" % name),
            app_theme.get_pixbuf("lyrics_button/%s_active_hover.png" % name),
            app_theme.get_pixbuf("lyrics_button/%s_inactive_press.png" % name),
            app_theme.get_pixbuf("lyrics_button/%s_active_press.png" % name),
            ) 
        if callback:
            toggle_button.connect("toggled", callback)
        return toggle_button
        
    def right_click_cb(self, widget, event):    
        if event.button == 3:
            Dispatcher.show_main_menu(int(event.x_root), int(event.y_root))
                    
    def __swap_play_status(self, obj, active):    
        self.__play.handler_block(self.__id_signal_play)
        self.__play.set_active(active)
        self.__play.handler_unblock(self.__id_signal_play)
        
    def __create_button(self, name, tip_msg=None):    
        button = ImageButton(
            app_theme.get_pixbuf("action/%s_normal.png" % name),
            app_theme.get_pixbuf("action/%s_hover.png" % name),
            app_theme.get_pixbuf("action/%s_press.png" % name),
            )
        button.connect("clicked", self.player_control, name)
        if tip_msg:
            Tooltip.text(button, tip_msg)
        return button
    
    def player_control(self, button, name):
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)()

            
