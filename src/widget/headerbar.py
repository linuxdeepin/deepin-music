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
from dtk.ui.draw import draw_text
import dtk.ui.tooltip as Tooltip

from player import Player
from widget.information import PlayInfo
from widget.timer import  VolumeSlider
from widget.new_timer import SongTimer

from widget.cover import PlayerCoverButton
from widget.skin import app_theme
from config import config
from helper import Dispatcher
from nls import _
from widget.ui_utils import set_widget_vcenter

class SimpleHeadbar(gtk.EventBox):            
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        # init.
        self.cover_box = PlayerCoverButton()
        self.cover_box.show_all()
        
        
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
        # swap played status handler
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        
        self.__id_signal_play = self.__play.connect("toggled", self.on_player_playpause)
        prev_button = self.__create_button("previous", _("Previous"))
        next_button = self.__create_button("next", _("Next"))
        
        
        # lyrics button
        self.lyrics_button = self.__create_simple_toggle_button("lyrics", self.change_lyrics_status)        
        Tooltip.text(self.lyrics_button, _("Show/Hide Lyrics"))
        
        
        lyrics_button_align = gtk.Alignment()
        lyrics_button_align.set_padding(0, 0, 0, 10)
        lyrics_button_align.set(0.5, 0.5, 0, 0)
        lyrics_button_align.add(self.lyrics_button)
        
        volume_button_align = gtk.Alignment()
        volume_button_align.set_padding(0, 0, 10, 0)
        volume_button_align.set(0.5, 0.5, 0, 0)
        self.volume_slider = VolumeSlider()
        volume_button_align.add(self.volume_slider)
        # volume_button_align.add(VolumeButton())
        
        # action_box
        action_box = gtk.HBox()
        action_box.pack_start(lyrics_button_align, False, False)
        action_box.pack_start(set_widget_vcenter(prev_button), False, False)
        action_box.pack_start(set_widget_vcenter(self.__play), False, False)
        action_box.pack_start(set_widget_vcenter(next_button), False, False)
        action_box.pack_start(volume_button_align, False, False)
         
        # combo_box
        playinfo_align = gtk.Alignment()
        playinfo_align.set_padding(0, 0, 12, 0)
        playinfo_align.add(PlayInfo(200))
        
        self.action_box_align = gtk.Alignment()
        self.action_box_align.set_padding(6, 0, 6, 0)
        self.action_box_align.add(action_box)
        self.action_box_align.connect("expose-event", self.on_expose_event)
        
        self.timer_label = "00:00"
        self.song_timer = SongTimer(self.draw_timer_label)
        
        combo_vbox = gtk.VBox(spacing=5)
        combo_vbox.pack_start(playinfo_align, False, False)
        combo_vbox.pack_start(self.action_box_align, False, False)
        
        cover_box_align = gtk.Alignment()
        cover_box_align.set_padding(0, 0, 10, 0)
        cover_box_align.add(self.cover_box)
        
        cover_main_box = gtk.HBox()
        cover_main_box.pack_start(cover_box_align, False, False)
        cover_main_box.pack_start(combo_vbox, False, False)
        
        body_vbox = gtk.VBox(spacing=6)
        body_vbox.pack_start(cover_main_box, False, True)
        body_vbox.pack_start(self.song_timer, False, True)
        # progressbar_align = gtk.Alignment()
        # progressbar_align.add(ProgressBar())
        # progressbar_align.set(1, 1, 1, 1)
        # progressbar_align.set_padding(0, 0, 2, 2)
        # body_vbox.pack_start(progressbar_align, False, True)
        
        
        
        self.add(body_vbox)

        # right click
        self.connect("button-press-event", self.right_click_cb)
        Dispatcher.connect("close-lyrics", lambda w : self.lyrics_button.set_active(False))
        Dispatcher.connect("show-lyrics", lambda w: self.lyrics_button.set_active(True))
 
        self.signal_auto = False        
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        self.signal_auto = True    
        
        
    def draw_timer_label(self, timer_text):    
        self.timer_text = timer_text
        rect = self.action_box_align.allocation
        rect.x += 12
        rect.width  = 68
        rect.height = 15
        self.action_box_align.queue_draw_area(rect.x, rect.y, rect.width, rect.height)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        rect.x += 12
        rect.width  = 68
        rect.height = 15
        
        draw_text(cr, self.timer_text, rect.x, rect.y, rect.width, rect.height, text_size=8,
                  gaussian_radious=2,
                  gaussian_color="#000000",
                  border_radious=1,
                  border_color="#000000",
                  text_color = "#FFFFFF"
                  )
        
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
            
    def sync_volume(self):        
        self.volume_slider.load_volume_config()
