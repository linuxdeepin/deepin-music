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
import gobject
from dtk.ui.button import ToggleButton, ImageButton
from dtk.ui.menu import Menu
from dtk.ui.utils import foreach_recursive

from player import Player
from widget.information import PlayInfo
from widget.timer import SongTimer, VolumeSlider
from widget.equalizer import EqualizerWindow
from widget.cover import PlayerCoverButton
from widget.lyrics_module import LyricsModule
from widget.skin import app_theme
from widget.ui import ProgressBox
from config import config
from helper import Dispatcher


class HeaderBar(gtk.EventBox):
    def __init__(self):
        super(HeaderBar, self).__init__()
        self.set_visible_window(False)
        
        # init.
        self.cover_box = PlayerCoverButton()
        self.cover_box.show_all()
        self.equalizer_win = EqualizerWindow()
        self.equalizer_win.equalizer_win.connect("hide", self.__set_equalizer_status)
        self.lyrics_display = LyricsModule()
        
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
        self.__play.show_all()

        self.__id_signal_play = self.__play.connect("toggled", lambda w: Player.playpause())
        
        prev_button = self.__create_button("previous")
        next_button = self.__create_button("next")
        
        self.vol = VolumeSlider()
        song_timer = SongTimer()
        
        mainbtn = gtk.HBox(spacing=3)
        prev_align = gtk.Alignment()
        prev_align.set(0.6, 0.4, 0, 0)
        prev_align.add(prev_button)
        
        next_align = gtk.Alignment()
        next_align.set(0.6, 0.4, 0, 0)
        next_align.add(next_button)
        
        # button group.
        mainbtn.pack_start(prev_align, False, False)
        mainbtn.pack_start(self.__play, False, False)
        mainbtn.pack_start(next_align, False, False)
         
        # time box.
        self.lyrics_button = self.__create_simple_toggle_button("lyrics", self.start_lyrics)        
        time_align = gtk.Alignment()
        time_align.set(0, 0, 0, 1)
        time_box = gtk.HBox()       

        time_box.pack_start(self.lyrics_button, False, False)
        time_box.pack_start(self.vol, False, False)        
        time_box.pack_start(time_align, True, True)
        time_box.pack_start(song_timer.get_label(), False, False)
        
        # playinfo box.
        playinfo_box = gtk.HBox()
        # playinfo_box.set_spacing(80)
        playinfo_box.pack_start(PlayInfo(), False, False)
        playinfo_box.pack_start(mainbtn, False, False)
        
        cover_right_box = gtk.VBox()
        cover_right_box.pack_start(playinfo_box, True, True)
        cover_right_box.pack_start(time_box, False, False)
        
        cover_main_box = gtk.HBox(spacing=5)
        cover_main_box.pack_start(self.cover_box, False, False)
        cover_main_box.pack_start(cover_right_box, True, True)
        cover_main_align = gtk.Alignment()
        cover_main_align.set_padding(5, 0, 6, 5)
        cover_main_align.set(1, 1, 1, 1)
        cover_main_align.add(cover_main_box)
        
        main_box = gtk.VBox(spacing=4)
        main_box.pack_start(cover_main_align, True, True)
        main_box.pack_start(ProgressBox(song_timer), True, True)
        
        self.add(main_box)
        
        Dispatcher.connect("close-lyrics", self.sync_lyrics_status)
        gobject.idle_add(self.load_config)
                
        # right click
        self.connect("button-press-event", self.right_click_cb)
        foreach_recursive(self, lambda w: w.connect("button-press-event", self.right_click_cb))
        
        
    def load_config(self):    
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        else:    
            self.lyrics_button.set_active(False)
        
    def open_dir(self, widget):    
        if widget.get_active():
            self.equalizer_win.run()
        else:    
            self.equalizer_win.hide_win(None)
            
    def __create_simple_toggle_button(self, name, callback):
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("header/%s_normal.png" % name),
            app_theme.get_pixbuf("header/%s_press.png" % name),
            app_theme.get_pixbuf("header/%s_hover.png" % name),
            )
        toggle_button.connect("toggled", callback)
        return toggle_button

    def sync_lyrics_status(self, obj):
        self.lyrics_button.set_active(False)
    
    def start_lyrics(self, widget):        
        if widget.get_active():
            self.lyrics_display.run()
        else:    
            self.lyrics_display.hide_all()
            
    def hide_lyrics(self):        
        self.lyrics_display.hide_without_config()
            
    def start_playlist(self, widget):        
        pass
            
    def save_db(self, widget):    
        pass
    
    def __set_equalizer_status(self, widget):
        config.set("equalizer", "status", "false")
        self.musicbox_button.set_active(False)
        
    def right_click_cb(self, widget, event):    
        if event.button == 3:
            Menu([(None, "均衡器", lambda : self.equalizer_win.run())], True).show((int(event.x_root), int(event.y_root)))
                    
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
        # todo tip
        return button
    
    def player_control(self, button, name):
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)()

            
            
