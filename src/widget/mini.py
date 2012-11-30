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
from widget.information import PlayInfo
from widget.ui_utils import set_widget_gravity, set_widget_vcenter, switch_tab, is_in_rect
from player import Player
from widget.timer import VolumeSlider

class MiniWindow(Window):
    
    def __init__(self):
        Window.__init__(self,
                        shape_frame_function=self.shape_mini_frame,
                        expose_frame_function=self.expose_mini_frame)
        
        self.set_property("skip-pager-hint", True)
        self.set_property("skip-taskbar-hint", True)
        
        self.body_box = gtk.VBox()
        
        self.control_box = gtk.HBox()
        self.action_box = gtk.HBox()
        self.event_box = gtk.HBox()
        self.info_box = gtk.HBox()
        # Build info box
        playinfo_align = set_widget_gravity(PlayInfo(200), (0.5, 0.5, 0, 0),
                                            (0, 0, 5, 5))
        self.info_box.add(playinfo_align)
        
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
        # swap played status handler
        self.__id_signal_play = self.playpause_button.connect("toggled", self.on_player_playpause)        
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        
        self.volume_button = VolumeSlider(auto_hide=False)
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
        self.connect("enter-notify-event", self.on_enter_notify_event)
        self.connect("leave-notify-event", self.on_leave_notify_event)
        
        Dispatcher.connect("close-lyrics", lambda w : self.lyrics_button.set_active(False))
        Dispatcher.connect("show-lyrics", lambda w: self.lyrics_button.set_active(True))
        
        if config.get("mini", "x") == "-1":
            self.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.move(int(config.get("mini","x")),int(config.get("mini","y")))
            
        # pixbufs    
        self.info_pixbuf = None
        self.control_pixbuf = None

        self.body_box.add(self.info_box)    
        self.add_move_event(self)
        self.window_frame.add(self.body_box)
        self.set_size_request(305, 55)
        
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
    
    def create_button(self, name, tip_msg=None):
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
            
    def is_in_window(self):        
        root_window = gtk.gdk.get_default_root_window()
        r_x, r_y = root_window.get_pointer()[:2]
        o_x, o_y = self.get_position()
        rect = self.allocation
        rect.x = o_x + 1
        rect.y = o_y + 1
        rect.width -= 2
        rect.height -= 2
        return is_in_rect((r_x, r_y), rect)
    
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
        

        return playpause_button    
    
    def on_player_playpause(self, widget):    
        if Player.song:
            Player.playpause()
            
    def __swap_play_status(self, obj, active):    
        self.playpause_button.handler_block(self.__id_signal_play)
        self.playpause_button.set_active(active)
        self.playpause_button.handler_unblock(self.__id_signal_play)
            
    def on_configure_event(self, widget, event):        
        if widget.get_property("visible"):
            config.set("mini","x","%d" % event.x)
            config.set("mini","y","%d" % event.y)
            
    def on_enter_notify_event(self, widget, event):        
        child = self.body_box.get_children()[0]
        if child != self.control_box:
            switch_tab(self.body_box, self.control_box)
        
    def on_leave_notify_event(self, widget, event):    
        if not self.is_in_window():
            switch_tab(self.body_box, self.info_box)
            
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
        
    def get_widget_pixbuf(self, widget=None):    
        if widget is None:
            widget = self
        drawable = widget.window
        x, y, width, height = widget.allocation
        pixbuf = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, False, 8, width, height)
        pixbuf.get_from_drawable(drawable, drawable.get_colormap(), x, y, 0, 0, width, height)
        return pixbuf
    
    def shape_mini_frame(self, widget, event):    
        pass
        
    def expose_mini_frame(self, widget, event):
        cr  = widget.window.cairo_create()
        rect = widget.allocation
        cr.set_source_rgba(1,1,1, 0.8)
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()

    
