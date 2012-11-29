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


import gtk
from dtk.ui.window import Window
from dtk.ui.button import ToggleButton, ImageButton, MenuButton, MinButton, CloseButton
import dtk.ui.tooltip as Tooltip

from helper import Dispatcher
from config import config
from widget.skin import app_theme
from widget.ui_utils import set_widget_gravity, set_widget_vcenter
from player import Player
from widget.timer import VolumeSlider

class MiniWindow(Window):
    
    def __init__(self):
        Window.__init__(self)
        
        self.body_box = gtk.VBox()
        
        self.control_box = gtk.HBox()
        self.action_box = gtk.HBox()
        self.event_box = gtk.HBox()
        self.info_box = gtk.HBox()
        
        # Build control box
        self.lyrics_button = self.create_lyrics_button()
        self.signal_auto = False        
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        self.signal_auto = True    
        
        
        lyrics_button_align = set_widget_gravity(self.lyrics_button, (0.5, 0.5, 0, 0),
                                                 (0, 0, 10, 8))
        previous_button = self.create_button("previous")
        next_button = self.create_button("next")
        self.playpause_button = self.create_playpause_button()
        self.volume_button = VolumeSlider()
        volume_button_align = set_widget_gravity(self.volume_button, (0.5, 0.5, 0, 0),
                                                 (0, 0, 8, 0))
        self.action_box.pack_start(lyrics_button_align, False, False)
        self.action_box.pack_start(set_widget_vcenter(previous_button), False, False)
        self.action_box.pack_start(set_widget_vcenter(self.playpause_button), False, False)
        self.action_box.pack_start(set_widget_vcenter(next_button), False, False)
        self.action_box.pack_start(volume_button_align, False, False)
        
        # Build event box.
        menu_button = MenuButton()
        menu_button.connect("button-press-event", self.on_menu_button_press)
        min_button = MinButton()
        min_button.connect("clicked", lambda w: self.min_window())
        close_button = CloseButton()
        close_button.connect("clicked", lambda w: self.hide_all())
        self.event_box.pack_start(menu_button, False, False)
        self.event_box.pack_start(min_button, False, False)
        self.event_box.pack_start(close_button, False, False)
        event_box_align = set_widget_gravity(self.event_box, paddings=(0, 0, 8, 0))
        
        self.control_box.pack_start(self.action_box, False, True)
        self.control_box.pack_end(event_box_align, False, True)
        
        self.connect("configure-event", self.on_configure_event)
        
        if config.get("mini", "x") == "-1":
            self.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.move(int(config.get("mini","x")),int(config.get("mini","y")))
        
        self.body_box.add(self.control_box)
        self.add_move_event(self)
        self.window_frame.add(self.body_box)
        self.set_size_request(300, 55)
        
    def on_menu_button_press(self, widget, event):    
        Dispatcher.show_main_menu(int(event.x_root), int(event.y_root))
        
    def create_lyrics_button(self):    
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("lyrics_button/lyrics_inactive_normal.png"),
            app_theme.get_pixbuf("lyrics_button/lyrics_active_normal.png"),
            app_theme.get_pixbuf("lyrics_button/lyrics_inactive_hover.png"),
            app_theme.get_pixbuf("lyrics_button/lyrics_active_hover.png"),
            app_theme.get_pixbuf("lyrics_button/lyrics_inactive_press.png"),
            app_theme.get_pixbuf("lyrics_button/lyrics_active_press.png"),
            ) 
        
        toggle_button.connect("toggled", self.change_lyrics_status)
        return toggle_button
    
    def change_lyrics_status(self, widget):    
        if self.signal_auto:
            if widget.get_active():
                Dispatcher.show_lyrics()
            else:    
                Dispatcher.close_lyrics()
    
    def create_button(self, name, callback=None, tip_msg=None):
        button = ImageButton(
            app_theme.get_pixbuf("action/%s_normal.png" % name),
            app_theme.get_pixbuf("action/%s_hover.png" % name),
            app_theme.get_pixbuf("action/%s_press.png" % name),
            )
        
        if callback:
            self.connect("clicked", callback)

        if tip_msg:
            Tooltip.text(button, tip_msg)
        return button
    
    def create_playpause_button(self):
        play_normal_pixbuf = app_theme.get_pixbuf("action/play_normal.png")
        pause_normal_pixbuf = app_theme.get_pixbuf("action/pause_normal.png")
        play_hover_pixbuf = app_theme.get_pixbuf("action/play_hover.png")
        pause_hover_pixbuf = app_theme.get_pixbuf("action/pause_hover.png")
        play_press_pixbuf = app_theme.get_pixbuf("action/play_press.png")
        pause_press_pixbuf = app_theme.get_pixbuf("action/pause_press.png")
       
        playpause_button = ToggleButton(play_normal_pixbuf, pause_normal_pixbuf,
                     play_hover_pixbuf, pause_hover_pixbuf,
                     play_press_pixbuf, pause_press_pixbuf)
        
        playpause_button.connect("toggled", self.on_player_playpause)
        return playpause_button    
    
    def on_player_playpause(self, widget):    
        if Player.song:
            Player.playpause()
            
    def on_configure_event(self, widget, event):        
        if widget.get_property("visible"):
            config.set("mini","x","%d" % event.x)
            config.set("mini","y","%d" % event.y)
            
    def toggle_visible(self, bring_to_front=False):        
        if self.get_property("visible"):
            if self.is_active():
                if not bring_to_front:
                    self.hide_all()
            else:    
                self.present()
        else:        
            self.show_from_tray()
            
    def toggle_window(self):        
        if self.get_property("visible"):
            self.hide_to_tray()
        else:    
            self.show_from_tray()

    def show_from_tray(self):        
        if config.get("mini", "x") != "-1":
            self.move(int(config.get("mini", "x")), int(config.get("mini", "y")))
        self.show_all()    
        
    def hide_to_tray(self):    
        self.hide_all()
