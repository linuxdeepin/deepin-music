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
# from dtk.ui.utils import foreach_recursive

from player import Player
from widget.information import PlayInfo
from widget.timer import SongTimer, VolumeSlider

from widget.cover import PlayerCoverButton
from widget.skin import app_theme
from widget.ui import ProgressBox
from config import config
from helper import Dispatcher
from nls import _
from widget.ui_utils import create_left_align, create_right_align
import locale

def get_prefix():
    (lang, encode) = locale.getdefaultlocale()
    if lang == "zh_CN":
        prefix = "cn"
    elif lang in ["zh_HK", "zh_TW"]:
        prefix = "tw"
    else:    
        prefix = "en"
    return prefix


class FullHeaderBar(gtk.EventBox):
    def __init__(self):
        super(FullHeaderBar, self).__init__()
        self.set_visible_window(False)
        
        # init.
        self.cover_box = PlayerCoverButton()
        self.cover_box.show_all()
        
        # Main table
        main_table = gtk.Table(2, 3)
        # swap played status handler
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        
        # play button
        play_normal_pixbuf = app_theme.get_pixbuf("action/play_large_normal.png")
        pause_normal_pixbuf = app_theme.get_pixbuf("action/pause_large_normal.png")
        play_hover_pixbuf = app_theme.get_pixbuf("action/play_large_hover.png")
        pause_hover_pixbuf = app_theme.get_pixbuf("action/pause_large_hover.png")
        play_press_pixbuf = app_theme.get_pixbuf("action/play_large_press.png")
        pause_press_pixbuf = app_theme.get_pixbuf("action/pause_large_press.png")
       
        self.__play = ToggleButton(play_normal_pixbuf, pause_normal_pixbuf,
                                   play_hover_pixbuf, pause_hover_pixbuf,
                                   play_press_pixbuf, pause_press_pixbuf,
                                   )
        Tooltip.text(self.__play, _("Play/Pause"))
        
        self.__play.show_all()

        self.__id_signal_play = self.__play.connect("toggled", lambda w: Player.playpause())
        
        prev_button = self.__create_button("previous_large", _("Previous track"))
        next_button = self.__create_button("next_large", _("Next track"))
        
        self.vol = VolumeSlider()
        song_timer = SongTimer()
        
        mainbtn = gtk.HBox(spacing=3)
        prev_align = gtk.Alignment()
        prev_align.set(0.5, 0.5, 0, 0)
        prev_align.add(prev_button)
        
        next_align = gtk.Alignment()
        next_align.set(0.5, 0.5, 0, 0)
        next_align.add(next_button)
        
        # button group.
        mainbtn.pack_start(prev_align, False, False)
        mainbtn.pack_start(self.__play, False, False)
        mainbtn.pack_start(next_align, False, False)
         
        mainbtn_align = gtk.Alignment()
        mainbtn_align.set_padding(10, 0, 0, 0)
        mainbtn_align.add(mainbtn)
        
        mainbtn_box = gtk.HBox()
        mainbtn_box.pack_start(mainbtn_align, False, False)
        mainbtn_box.pack_start(create_left_align(), True, True)
        
        # time box.
        self.lyrics_button = self.__create_simple_toggle_button("lyrics", self.change_lyrics_status)        
        Tooltip.text(self.lyrics_button, _("Lyrics on/off"))
        
        plug_box = gtk.HBox(spacing=12)       
        lyrics_button_align = gtk.Alignment()
        lyrics_button_align.set_padding(1, 0, 0, 0)
        lyrics_button_align.add(self.lyrics_button)
        plug_box.pack_start(lyrics_button_align, False, False)
        vol_align = gtk.Alignment()
        vol_align.set_padding(2, 0, 0, 0)
        vol_align.add(self.vol)
        plug_box.pack_start(vol_align, False, False)        
        
        timer_align = gtk.Alignment()
        timer_align.set(0, 0, 0, 1)
        timer_box = gtk.HBox()
        timer_box.pack_start(timer_align, True, True)
        timer_box.pack_start(song_timer.get_label(), False, False)
        
        main_table.attach(PlayInfo(), 0, 1, 0, 1, xoptions=gtk.FILL)
        main_table.attach(plug_box, 0, 1, 1, 2, xoptions=gtk.FILL)
        main_table.attach(mainbtn_box, 1, 2, 0, 2, xoptions=gtk.FILL, xpadding=25)
        main_table.attach(timer_box, 2, 3, 1, 2, xpadding=17)
        
        cover_main_box = gtk.HBox(spacing=5)
        cover_main_box.pack_start(self.cover_box, False, False)
        cover_main_box.pack_start(main_table, True, True)
        cover_main_align = gtk.Alignment()
        cover_main_align.set_padding(5, 0, 12, 5)
        cover_main_align.set(1, 1, 1, 1)
        cover_main_align.add(cover_main_box)
        
        main_box = gtk.VBox(spacing=9)
        main_box.pack_start(cover_main_align, True, True)
        main_box.pack_start(song_timer, True, True)
        
        self.add(main_box)
        
        # right click
        self.connect("button-press-event", self.right_click_cb)
        # foreach_recursive(self, lambda w: w.connect("button-press-event", self.right_click_cb))
        Dispatcher.connect("close-lyrics", lambda w : self.lyrics_button.set_active(False))
        Dispatcher.connect("show-lyrics", lambda w: self.lyrics_button.set_active(True))
 
        self.signal_auto = False        
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        self.signal_auto = True    
        
    def change_lyrics_status(self, widget):    
        if self.signal_auto:
            if widget.get_active():
                Dispatcher.show_lyrics()
            else:    
                Dispatcher.close_lyrics()
            
    def __create_simple_toggle_button(self, name, callback): 
        prefix = get_prefix()
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("header/%s_%s_inactive_normal.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_normal.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_inactive_hover.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_hover.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_inactive_press.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_press.png" % (prefix, name)),
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
        # todo tip
        return button
    
    def player_control(self, button, name):
        name = name.strip("_large")
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)()

            
class SimpleHeadber(gtk.EventBox):            
    def __init__(self):
        super(SimpleHeadber, self).__init__()
        self.set_visible_window(False)
        
        # init.
        self.cover_box = PlayerCoverButton()
        self.cover_box.show_all()
        
        # Main table
        main_table = gtk.Table(2, 2)
        self.move_box = gtk.EventBox()
        self.move_box.set_visible_window(False)
                
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

        self.__id_signal_play = self.__play.connect("toggled", lambda w: Player.playpause())
        
        prev_button = self.__create_button("previous", _("Previous track"))
        next_button = self.__create_button("next", _("Next track"))
        
        self.vol = VolumeSlider()
        song_timer = SongTimer()
        
        mainbtn = gtk.HBox(spacing=3)
        prev_align = gtk.Alignment()
        prev_align.set(0.5, 0.5, 0, 0)
        prev_align.add(prev_button)
        
        next_align = gtk.Alignment()
        next_align.set(0.5, 0.5, 0, 0)
        next_align.add(next_button)
        
        # button group.
        mainbtn.pack_start(prev_align, False, False)
        mainbtn.pack_start(self.__play, False, False)
        mainbtn.pack_start(next_align, False, False)
         
        mainbtn_align = gtk.Alignment()
        mainbtn_align.set_padding(0, 0, 0, 10)
        mainbtn_align.add(mainbtn)
        
        mainbtn_box = gtk.HBox()
        mainbtn_box.pack_start(create_right_align(), True, True)
        mainbtn_box.pack_start(mainbtn_align, False, False)
        
        # time box.
        self.lyrics_button = self.__create_simple_toggle_button("lyrics", self.change_lyrics_status)        
        Tooltip.text(self.lyrics_button, _("Lyrics on/off"))
        
        plug_box = gtk.HBox(spacing=12)       
        vol_align = gtk.Alignment()
        vol_align.set_padding(2, 0, 0, 0)
        vol_align.add(self.vol)
        
        lyrics_button_align = gtk.Alignment()
        lyrics_button_align.set_padding(1, 0, 0, 0)
        lyrics_button_align.add(self.lyrics_button)
        plug_box.pack_start(lyrics_button_align, False, False)
        plug_box.pack_start(vol_align, False, False)        
        
        timer_align = gtk.Alignment()
        timer_align.set(0, 0, 0, 1)
        timer_box = gtk.HBox()
        timer_box.pack_start(timer_align, True, True)
        timer_label_align = gtk.Alignment()
        timer_label_align.set_padding(2, 0, 0, 0)
        timer_label_align.add(song_timer.get_label())
        timer_box.pack_start(timer_label_align, False, False)
        
        main_table.attach(PlayInfo(110), 0, 1, 0, 1, xoptions=gtk.FILL)
        main_table.attach(mainbtn_box, 1, 2, 0, 1,xoptions=gtk.FILL, xpadding=5)        
        main_table.attach(plug_box, 0, 1, 1, 2, xoptions=gtk.FILL)
        main_table.attach(timer_box, 1, 2, 1, 2, xoptions=gtk.FILL, xpadding=15)
        
        cover_main_box = gtk.HBox(spacing=5)
        cover_main_box.pack_start(self.cover_box, False, False)
        cover_main_box.pack_start(main_table, True, True)
        cover_main_align = gtk.Alignment()
        cover_main_align.set_padding(5, 0, 12, 5)
        cover_main_align.set(1, 1, 1, 1)
        cover_main_align.add(cover_main_box)
        
        main_box = gtk.VBox(spacing=9)
        main_box.pack_start(cover_main_align, False, False)
        main_box.pack_start(song_timer, True, True)
        self.add(main_box)

        # right click
        self.connect("button-press-event", self.right_click_cb)
        # foreach_recursive(self, lambda w: w.connect("button-press-event", self.right_click_cb))
        Dispatcher.connect("close-lyrics", lambda w : self.lyrics_button.set_active(False))
        Dispatcher.connect("show-lyrics", lambda w: self.lyrics_button.set_active(True))
 
        self.signal_auto = False        
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        self.signal_auto = True    
        
    def change_lyrics_status(self, widget):    
        if self.signal_auto:
            if widget.get_active():
                Dispatcher.show_lyrics()
            else:    
                Dispatcher.close_lyrics()
        
    def __create_simple_toggle_button(self, name, callback): 
        prefix = get_prefix()
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("header/%s_%s_inactive_normal.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_normal.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_inactive_hover.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_hover.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_inactive_press.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_press.png" % (prefix, name)),
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
        name = name.strip("_large")
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)()

            
class HeaderBer(gtk.EventBox):            
    def __init__(self):
        super(HeaderBer, self).__init__()
        self.set_visible_window(False)
        
        # init.
        self.cover_box = PlayerCoverButton()
        self.cover_box.show_all()
        
        # Main table
        main_table = gtk.Table(2, 2)
        self.move_box = gtk.EventBox()
        self.move_box.set_visible_window(False)
                
        # swap played status handler
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        
        # play button
        play_normal_pixbuf = app_theme.get_pixbuf("new_header/play_normal.png")
        pause_normal_pixbuf = app_theme.get_pixbuf("new_header/pause_normal.png")
        play_hover_pixbuf = app_theme.get_pixbuf("new_header/play_hover.png")
        pause_hover_pixbuf = app_theme.get_pixbuf("new_header/pause_hover.png")
        play_press_pixbuf = app_theme.get_pixbuf("new_header/play_press.png")
        pause_press_pixbuf = app_theme.get_pixbuf("new_header/pause_press.png")
       
        self.__play = ToggleButton(play_normal_pixbuf, pause_normal_pixbuf,
                                   play_hover_pixbuf, pause_hover_pixbuf,
                                   play_press_pixbuf, pause_press_pixbuf,
                                   )
        Tooltip.text(self.__play, _("Play/Pause"))
        self.__play.show_all()

        self.__id_signal_play = self.__play.connect("toggled", lambda w: Player.playpause())
        
        prev_button = self.__create_button("previous", _("Previous track"))
        next_button = self.__create_button("next", _("Next track"))
        
        self.vol = VolumeSlider()
        song_timer = SongTimer()
        
        mainbtn = gtk.HBox(spacing=3)
        prev_align = gtk.Alignment()
        prev_align.set(0.5, 0.5, 0, 0)
        prev_align.add(prev_button)
        
        next_align = gtk.Alignment()
        next_align.set(0.5, 0.5, 0, 0)
        next_align.add(next_button)
        
        # button group.
        mainbtn.pack_start(prev_align, False, False)
        mainbtn.pack_start(self.__play, False, False)
        mainbtn.pack_start(next_align, False, False)
         
        mainbtn_align = gtk.Alignment()
        mainbtn_align.set_padding(0, 0, 0, 10)
        mainbtn_align.add(mainbtn)
        
        mainbtn_box = gtk.HBox()
        mainbtn_box.pack_start(create_right_align(), True, True)
        mainbtn_box.pack_start(mainbtn_align, False, False)
        
        # time box.
        self.lyrics_button = self.__create_simple_toggle_button("lyrics", self.change_lyrics_status)        
        Tooltip.text(self.lyrics_button, _("Lyrics on/off"))
        
        plug_box = gtk.HBox(spacing=12)       
        vol_align = gtk.Alignment()
        vol_align.set_padding(2, 0, 0, 0)
        vol_align.add(self.vol)
        
        lyrics_button_align = gtk.Alignment()
        lyrics_button_align.set_padding(1, 0, 0, 0)
        lyrics_button_align.add(self.lyrics_button)
        plug_box.pack_start(lyrics_button_align, False, False)
        plug_box.pack_start(vol_align, False, False)        
        
        timer_align = gtk.Alignment()
        timer_align.set(0, 0, 0, 1)
        timer_box = gtk.HBox()
        timer_box.pack_start(timer_align, True, True)
        timer_label_align = gtk.Alignment()
        timer_label_align.set_padding(2, 0, 0, 0)
        timer_label_align.add(song_timer.get_label())
        timer_box.pack_start(timer_label_align, False, False)
        
        main_table.attach(PlayInfo(110), 0, 1, 0, 1, xoptions=gtk.FILL)
        main_table.attach(mainbtn_box, 1, 2, 0, 1,xoptions=gtk.FILL, xpadding=5)        
        main_table.attach(plug_box, 0, 1, 1, 2, xoptions=gtk.FILL)
        main_table.attach(timer_box, 1, 2, 1, 2, xoptions=gtk.FILL, xpadding=15)
        
        cover_main_box = gtk.HBox(spacing=5)
        cover_main_box.pack_start(self.cover_box, False, False)
        cover_main_box.pack_start(main_table, True, True)
        cover_main_align = gtk.Alignment()
        cover_main_align.set_padding(5, 0, 12, 5)
        cover_main_align.set(1, 1, 1, 1)
        cover_main_align.add(cover_main_box)
        
        main_box = gtk.VBox(spacing=9)
        main_box.pack_start(cover_main_align, False, False)
        main_box.pack_start(song_timer, True, True)
        self.add(main_box)

        # right click
        self.connect("button-press-event", self.right_click_cb)
        # foreach_recursive(self, lambda w: w.connect("button-press-event", self.right_click_cb))
        Dispatcher.connect("close-lyrics", lambda w : self.lyrics_button.set_active(False))
        Dispatcher.connect("show-lyrics", lambda w: self.lyrics_button.set_active(True))
 
        self.signal_auto = False        
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        self.signal_auto = True    
        
    def change_lyrics_status(self, widget):    
        if self.signal_auto:
            if widget.get_active():
                Dispatcher.show_lyrics()
            else:    
                Dispatcher.close_lyrics()
        
    def __create_simple_toggle_button(self, name, callback): 
        prefix = get_prefix()
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("header/%s_%s_inactive_normal.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_normal.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_inactive_hover.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_hover.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_inactive_press.png" % (prefix, name)),
            app_theme.get_pixbuf("header/%s_%s_active_press.png" % (prefix, name)),
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
        name = name.strip("_large")
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)() 
