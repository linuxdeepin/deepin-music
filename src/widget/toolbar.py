#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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
import pango
from dtk.ui.window import Window
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.box import ImageBox
from dtk.ui.menu import Menu

import utils
from widget.ui import app_theme
from player import Player
from render_lyrics import render_lyrics
from widget.lyrics import desktop_lyrics
from lrc_parser import lrc_parser
from config import config
from widget.lyrics_search import search_ui

PREDEFINE_COLORS = {
   "vitality_yellow" : ["#ffffff", "#7cbee8", "#3993d2", "#fff5c9", "#ffe888", "#ffcc00"],
   "fresh_green"     : ["#ffffff", "#9ce265", "#71ce2e", "#fff5c9", "#ffe888", "#ffcc00"],
   "playful_pink"    : ["#ffffff", "#7cbee8", "#3993d2", "#ffc9e1", "#ff3490", "#ff2586"],
   "cool_blue"       : ["#ffffff", "#66c3ff", "#23a2ff", "#f8f8f8", "#dedede", "#b4b4b4"],
   "default"         : ["#1CC8FA", "#2226E5", "#51DEF1", "#662600", "#FFFF00", "#FF8000"],
    }


class ToolBar(Window):
    def __init__(self):
        super(ToolBar, self).__init__(window_type=gtk.WINDOW_POPUP)
        self.background_dpixbuf = app_theme.get_pixbuf("lyric/background.png")
        padding_x, padding_y = 10, 5
        self.set_size_request(-1, 41)
        play_box = gtk.HBox(spacing=2)
        
        # swap played status handler
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        self.playpause_button = ToggleButton(
            app_theme.get_pixbuf("lyric/play_normal.png"),
            app_theme.get_pixbuf("lyric/pause_normal.png"))
        self.__id_signal_play = self.playpause_button.connect("toggled", lambda w: Player.playpause())
        prev = self.__create_button("previous")
        next = self.__create_button("next")
        
        prev_align = gtk.Alignment()
        prev_align.set(0.5, 0.5, 0, 0)
        prev_align.add(prev)
        next_align = gtk.Alignment()
        next_align.set(0.5, 0.5, 0, 0)
        next_align.add(next)
        
        play_align = gtk.Alignment()
        play_align.set_padding(2, 0, 0, 0)
        play_align.set(0.5, 0.5, 0, 0)
        play_align.add(self.playpause_button)
        
        # separte line
        separate_line = ImageBox(app_theme.get_pixbuf("lyric/separate.png"))
        sep_align = gtk.Alignment()
        sep_align.set(0.5, 0.5, 0, 0)
        sep_align.add(separate_line)
        
        zoom_in_align = self.__create_zoom_button("zoom_in")
        zoom_out_align = self.__create_zoom_button("zoom_out")
        predefine_align = self.__create_simple_button("predefine_color", self.popup_predefine_menu , True)
        lock_align, self.lock_button = self.__create_simple_toggle_button("lock", "unlock", self.change_lock_status)
        karaoke_align, self.karaoke_button = self.__create_simple_toggle_button("karaoke", "karaoke", self.change_karaoke_status)
        line_align, self.line_button = self.__create_simple_toggle_button("single_line", "double_line", self.change_line_status)
        setting_align = self.__create_simple_button("setting", self.open_setting_window)
        search_align = self.__create_simple_button("search", self.open_search_window)
        close_align = self.__create_simple_button("close", self.close_lyric_window)
        before_align = self.__create_simple_button("before", self.before_offset)
        after_align = self.__create_simple_button("after", self.after_offset)

        play_box.pack_start(prev_align, False, False)
        play_box.pack_start(play_align, False, False)
        play_box.pack_start(next_align, False, False)
        play_box.pack_start(sep_align, False, False)
        play_box.pack_start(zoom_in_align, False, False)
        play_box.pack_start(zoom_out_align, False, False)
        play_box.pack_start(before_align, False, False)
        play_box.pack_start(after_align, False, False)
        play_box.pack_start(predefine_align, False, False)
        play_box.pack_start(karaoke_align, False, False)
        play_box.pack_start(line_align, False, False)
        play_box.pack_start(lock_align, False, False)        
        play_box.pack_start(setting_align, False, False)
        play_box.pack_start(search_align, False, False)
        play_box.pack_start(close_align, False, False)
        
        main_align = gtk.Alignment()
        main_align.set_padding(0, 0, padding_x, padding_x)
        main_align.set(0.5, 0.5, 0, 0)
        main_align.add(play_box)
        self.window_frame.pack_start(main_align)
        self.load_config()
        
        
    def __create_simple_button(self, name, callback, has_event=False):    
        button = ImageButton(
            app_theme.get_pixbuf("lyric/%s_normal.png" % name),
            app_theme.get_pixbuf("lyric/%s_hover.png" % name),
            app_theme.get_pixbuf("lyric/%s_press.png" % name)
            )
        if has_event:
            button.connect("button-press-event", callback)
        else:    
            button.connect("clicked", callback)
        button_align = gtk.Alignment()
        button_align.set(0.5, 0.5, 0, 0)
        button_align.add(button)
        return button_align
        
    def __create_simple_toggle_button(self, normal_name, active_name, callback):
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("lyric/%s_normal.png" % normal_name),
            app_theme.get_pixbuf("lyric/%s_press.png" % active_name)
            )
        toggle_button.connect("toggled", callback)
        toggle_align = gtk.Alignment()
        toggle_align.set(0.5, 0.5, 0, 0)
        toggle_align.add(toggle_button)
        return toggle_align, toggle_button
    
    
    def before_offset(self, widget):
        lrc_parser.set_offset(-500)
        
    def after_offset(self, widget):    
        lrc_parser.set_offset(500)
        
    
    def load_config(self):
        if config.getint("lyrics", "line_count") == 1:
            self.line_button.set_active(True)
            
    
    def close_lyric_window(self, widget):
        x, y = desktop_lyrics.lyrics_win.get_position()
        config.set("lyrics", "x", str(x))
        config.set("lyrics", "y", str(y))
        config.set("lyrics", "status", "false")
        desktop_lyrics.lyrics_win.hide_all()
        self.hide_all()
        
    def open_search_window(self, widget):
        try:
            search_ui.result_view.clear()
            search_ui.artist_entry.entry.set_text(Player.song.get_str("artist"))
            search_ui.title_entry.entry.set_text(Player.song.get_str("title"))
            search_ui.search_lyric_cb(None)
        except:    
            pass
        search_ui.show_window()
    
    def open_setting_window(self, widget):
        pass
    
    def change_lock_status(self, widget):        
        pass
        
    def popup_predefine_menu(self, widget, event):    
        menu_dict = utils.OrderDict()
        menu_dict["default"] = "默认"
        menu_dict["vitality_yellow"] = "活力黄"
        menu_dict["fresh_green"]  = "清新绿"
        menu_dict["playful_pink"] = "俏皮粉"
        menu_dict["cool_blue"] = "清爽蓝"
        menu_items = [(None, value, self.set_predefine_color, key) for key, value in menu_dict.iteritems()]
        menu_win = Menu(menu_items)
        menu_win.menu_window.set_keep_above(True)
        width, height = menu_win.menu_window.get_size()
        y = event.y_root - height + 40
        menu_win.show((int(event.x_root), int(y)))
        # menu_win.menu_window.show_all()
        # menu_win.menu_window.move(int(event.x_root), int(y))
        
    def set_predefine_color(self, key):    
        values = PREDEFINE_COLORS[key]
        config.set("lyrics", "inactive_color_upper", values[0])
        config.set("lyrics", "inactive_color_middle", values[1])
        config.set("lyrics", "inactive_color_bottom", values[2])
        config.set("lyrics", "active_color_upper", values[3])
        config.set("lyrics", "active_color_middle", values[4])
        config.set("lyrics", "active_color_bottom", values[5])
    
    def change_karaoke_status(self, widget):
        desktop_lyrics.set_karaoke_mode()
    
    def change_line_status(self, widget):
        if widget.get_active():
            desktop_lyrics.set_line_count(1)
        else:    
            desktop_lyrics.set_line_count(2)
        
    def __swap_play_status(self, obj, active):    
        self.playpause_button.handler_block(self.__id_signal_play)
        self.playpause_button.set_active(active)
        self.playpause_button.handler_unblock(self.__id_signal_play)
        
    def __create_zoom_button(self, name, msg=None):    
        button = ImageButton(
            app_theme.get_pixbuf("lyric/%s_normal.png" % name),
            app_theme.get_pixbuf("lyric/%s_hover.png" % name),
            app_theme.get_pixbuf("lyric/%s_press.png" % name)
            )
        button.connect("clicked", self.change_font_size, name)
        align = gtk.Alignment()
        align.set(0.5, 0.5, 0, 0)
        align.add(button)
        return align
        
    def change_font_size(self, widget, name):    
        old_size= render_lyrics.get_font_size()
        if name == "zoom_in":
            new_size = old_size + 2
            if new_size > 70:
                new_size = 70
            render_lyrics.set_font_size(new_size)
        elif name == "zoom_out":    
            new_size = old_size - 2
            if new_size < 16:
                new_size = 16
            render_lyrics.set_font_size(new_size)
        
    def __create_button(self, name, tip_msg=None):   
        button = ImageButton(
            app_theme.get_pixbuf("lyric/%s_normal.png" % name),
            app_theme.get_pixbuf("lyric/%s_hover.png" % name),
            app_theme.get_pixbuf("lyric/%s_press.png" % name)
            )
        button.connect("clicked", self.player_control, name)
        return button
        
    def player_control(self, button, name):   
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)()
        
lyric_toolbar = ToolBar()        
        
