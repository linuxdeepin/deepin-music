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

import gobject
import threading
import gtk
import os
import pango
from dtk.ui.window import Window
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.box import ImageBox
from dtk.ui.menu import Menu

from helper import Dispatcher
from widget.lyrics import LyricsWindow
from widget.ui import app_theme
from widget.lyrics_search import SearchUI
from lrc_parser import LrcParser
from config import config
from player import Player
from lrc_manager import LrcManager
import utils


MESSAGE_DURATION_MS = 3000

PREDEFINE_COLORS = {
   "vitality_yellow" : ["#ffffff", "#7cbee8", "#3993d2", "#fff5c9", "#ffe888", "#ffcc00"],
   "fresh_green"     : ["#ffffff", "#9ce265", "#71ce2e", "#fff5c9", "#ffe888", "#ffcc00"],
   "playful_pink"    : ["#ffffff", "#7cbee8", "#3993d2", "#ffc9e1", "#ff3490", "#ff2586"],
   "cool_blue"       : ["#ffffff", "#66c3ff", "#23a2ff", "#f8f8f8", "#dedede", "#b4b4b4"],
   "default"         : ["#1CC8FA", "#2226E5", "#51DEF1", "#662600", "#FFFF00", "#FF8000"],
    }

class LyricsModule(object):
    def __init__(self):
        self.desktop_lyrics = LyricsWindow()
        self.desktop_lyrics.connect("moved", self.adjust_toolbar_rect)
        self.desktop_lyrics.connect("resized", self.adjust_toolbar_rect)
        self.desktop_lyrics.connect("hide-bg", self.hide_toolbar)
        self.desktop_lyrics.connect("show-bg", self.show_toolbar)
        
        Player.connect("instant-new-song", self.instant_update_lrc)
        Player.connect("played", self.play_time_source)
        Player.connect("paused", self.pause_time_source)
        Dispatcher.connect("reload-lrc", self.update_lrc)
        
        self.lrc_manager = LrcManager()
        self.lrc = LrcParser()
        self.search_ui = SearchUI()
        self.lrc_id = -1
        self.lrc_next_id = -1
        self.current_line = 0
        self.message_source = None
        self.time_source = None
        self.song_duration = 0
        self.__find_flag = False
        
        self.init_toolbar()
        
        self.current_song = None
        self.next_lrc_to_download = None
        self.condition = threading.Condition()
        self.thread = threading.Thread(target=self.func_thread)
        self.thread.setDaemon(True)
        self.thread.start()
        
        
        
    def init_toolbar(self):    
        self.toolbar = Window(window_type=gtk.WINDOW_POPUP) 
        self.toolbar.background_dpixbuf = app_theme.get_pixbuf("lyric/background.png")
        self.toolbar.set_size_request(-1, 41)

        padding_x, padding_y = 10, 5
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
        self.toolbar.window_frame.pack_start(main_align)
        
        self.load_button_status()
        
    def load_button_status(self):    
        if not config.getboolean("lyrics","karaoke_mode"):
            self.karaoke_button.set_active(True)
        if config.getint("lyrics", "line_count") == 1:
            self.line_button.set_active(True)
        
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
        
    def __create_button(self, name, tip_msg=None):   
        button = ImageButton(
            app_theme.get_pixbuf("lyric/%s_normal.png" % name),
            app_theme.get_pixbuf("lyric/%s_hover.png" % name),
            app_theme.get_pixbuf("lyric/%s_press.png" % name)
            )
        button.connect("clicked", self.player_control, name)
        return button
    
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
        old_size= self.desktop_lyrics.get_font_size()
        if name == "zoom_in":
            new_size = old_size + 2
            if new_size > 70:
                new_size = 70
            self.desktop_lyrics.set_font_size(new_size)
        elif name == "zoom_out":    
            new_size = old_size - 2
            if new_size < 16:
                new_size = 16
            self.desktop_lyrics.set_font_size(new_size)
        
    def player_control(self, button, name):   
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)()
    
    def before_offset(self, widget):        
        self.lrc.set_offset(-500)
        
    def after_offset(self, widget):    
        self.lrc.set_offset(500)
        
    def open_search_window(self, widget):
        try:
            self.search_ui.result_view.clear()
            self.search_ui.artist_entry.entry.set_text(Player.song.get_str("artist"))
            self.search_ui.title_entry.entry.set_text(Player.song.get_str("title"))
            self.search_ui.search_lyric_cb(None)
        except:    
            pass
        self.search_ui.show_window()
    
    def close_lyric_window(self, widget):
        self.hide_all()
        Dispatcher.close_lyrics()
        
    def open_setting_window(self, widget):
        pass
    
    def change_lock_status(self, widget):        
        pass
    
    def change_karaoke_status(self, widget):
        self.desktop_lyrics.set_karaoke_mode()
    
    def change_line_status(self, widget):
        if widget.get_active():
            self.desktop_lyrics.set_line_count(1)
        else:    
            self.desktop_lyrics.set_line_count(2)
        
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
        
    def set_predefine_color(self, key):    
        values = PREDEFINE_COLORS[key]
        config.set("lyrics", "inactive_color_upper", values[0])
        config.set("lyrics", "inactive_color_middle", values[1])
        config.set("lyrics", "inactive_color_bottom", values[2])
        config.set("lyrics", "active_color_upper", values[3])
        config.set("lyrics", "active_color_middle", values[4])
        config.set("lyrics", "active_color_bottom", values[5])
        
        
    def func_thread(self):    
        while True:
            self.condition.acquire()
            while not self.next_lrc_to_download:
                self.condition.wait()
            next_lrc_to_download = self.next_lrc_to_download    
            self.next_lrc_to_download = None
            self.condition.release()
            self.set_current_lrc(True, next_lrc_to_download)
        
    def set_duration(self, duration):    
        if not duration:
            return 
        self.song_duration = duration
        
    def set_lrc_file(self, filename):    
        if filename and self.message_source != None:
            self.clear_message()
        self.clear_lyrics()    
        self.lrc.set_filename(filename)

    def set_played_time(self, played_time):    
        info = self.lrc.get_lyric_by_time(played_time, self.song_duration)
        if not info:
            return
        text, percentage, lyric_id = info
        real_id, real_lyric = self.get_real_lyric(lyric_id)

        if real_lyric == None:
            nid = -1
        else: 
            nid = real_id
            
        if self.lrc_id != nid:    
            if nid == -1:
                self.clear_lyrics()
                return
            if nid != self.lrc_next_id:
                self.current_line = 0
                if real_lyric:
                    self.desktop_lyrics.set_lyric(self.current_line, real_lyric)
                if nid != lyric_id:
                    self.desktop_lyrics.set_current_percentage(0.0)
                self.update_next_lyric(real_id)    
            else:    
                self.desktop_lyrics.set_line_percentage(self.current_line, 1.0)
                self.current_line = 1 - self.current_line
                
            self.lrc_id = nid    
            self.desktop_lyrics.set_current_line(self.current_line)
            
        if nid == lyric_id and percentage > 0.5:    
            self.update_next_lyric(real_id)
            
        if nid == lyric_id:    
            self.desktop_lyrics.set_current_percentage(percentage)
                
    def update_next_lyric(self, item_id):            
        if self.desktop_lyrics.get_line_count() == 1:
            self.lrc_next_id = -1
            return
        item_id += 1
        real_id, real_lyric = self.get_real_lyric(item_id)
        if real_lyric == None:
            if self.lrc_next_id == -1:
                return
            else:
                self.lrc_next_id = -1
                self.desktop_lyrics.set_lyric(1 - self.current_line, "")
        else:        
            if self.lrc_next_id == real_id:
                return
            if real_lyric:
                self.lrc_next_id = real_id
                self.desktop_lyrics.set_lyric(1 - self.current_line, real_lyric)
        self.desktop_lyrics.set_line_percentage(1 - self.current_line, 0.0)        
        
                
    def get_real_lyric(self, item_id):             
        while True:
            if self.lrc.get_item_lyric(item_id) != "":
                break
            item_id += 1
        return item_id, self.lrc.get_item_lyric(item_id)
    
    def clear_lyrics(self):
        self.desktop_lyrics.set_lyric(0, "")
        self.desktop_lyrics.set_lyric(1, "")
        self.current_line = 0
        self.lrc_id = -1
        self.lrc_next_id = -1
        
    def set_message(self, message, duration_ms):    
        if not message:
            return
        self.desktop_lyrics.set_current_line(0)
        self.desktop_lyrics.set_current_percentage(1.0)
        self.desktop_lyrics.set_lyric(0, message)
        self.desktop_lyrics.set_lyric(1, "")
        
        if self.message_source != None:
            gobject.source_remove(self.message_source)
        self.message_source = gobject.timeout_add(duration_ms, self.hide_message)
            
    def hide_message(self):    
        self.desktop_lyrics.set_lyric(0, "")
        self.message_source = None
        return False
    
    def clear_message(self):
        if self.message_source != None:
            gobject.source_remove(self.message_source)
            self.hide_message()
    
    def set_search_message(self, message):
        self.set_message(message, -1)
        
    def set_search_fail_message(self, message):
        self.set_message(message, MESSAGE_DURATION_MS)
        
    def set_download_fail_message(self, message):
        self.set_message(message, MESSAGE_DURATION_MS)
        
    def run(self):
        config.set("lyrics", "status", "true")
        self.play_time_source()
        screen_w, screen_h = gtk.gdk.get_default_root_window().get_size()
        w , h = self.desktop_lyrics.lyrics_win.get_size()
        try:
            x = config.getint("lyrics", "x")
            y = config.getint("lyrics", "y")
        except:    
            x = screen_w / 2 - w / 2
            y = screen_h - h

        self.desktop_lyrics.lyrics_win.move(x, y) 
        self.desktop_lyrics.lyrics_win.show_all()           

        
    def hide_toolbar(self, widget):    
        self.toolbar.hide_all()
        
    def show_toolbar(self, widget):    
        self.toolbar.show_all()
        self.toolbar.hide_all()
        l_x, l_y = self.desktop_lyrics.lyrics_win.get_position()
        l_w, l_h = self.desktop_lyrics.lyrics_win.get_size()
        rect = gtk.gdk.Rectangle(int(l_x), int(l_y), int(l_w), int(l_h))
        self.adjust_toolbar_rect(None, rect)
        self.toolbar.show_all()
        
    def hide_all(self):    
        x, y = self.desktop_lyrics.lyrics_win.get_position()
        config.set("lyrics", "x", str(x))
        config.set("lyrics", "y", str(y))
        self.desktop_lyrics.lyrics_win.hide_all()
        self.toolbar.hide_all()
        config.set("lyrics", "status", "false")
        self.pause_time_source()
        
    def adjust_toolbar_rect(self, widget, rect):    
        screen_w, screen_h = gtk.gdk.get_default_root_window().get_size()
        centre_x = rect.x + rect.width / 2
        l_w, l_h = self.toolbar.get_size()
        l_x = centre_x - l_w / 2
        if rect.y <  l_h:
            l_y = rect.y + rect.height
        elif rect.y > screen_h - rect.height:    
            l_y = rect.y - l_h
        else:    
            l_y = rect.y - l_h
        self.toolbar.move(l_x, l_y)    
        
    def update_lrc(self, widget, songs):
        if isinstance(songs, list):
            if self.current_song in songs:
                self.current_song = songs[songs.index(self.current_song)]
        else:        
            self.current_song = songs
            
        if self.current_song is not None:    
            if not self.set_current_lrc(False):
                self.condition.acquire()
                self.next_lrc_to_download = self.current_song
                self.condition.notify()
                self.condition.release()
                
    def real_show_lyrics(self):            
        played_timed = Player.get_lyrics_position()
        self.set_played_time(played_timed)
        return True
    
    def pause_time_source(self, *args):
        if self.time_source != None:
            gobject.source_remove(self.time_source)
            self.time_source = None
            
    def play_time_source(self, *args):        
        self.pause_time_source()
        if not self.__find_flag:
            return 
        
        if not config.getboolean("lyrics", "status"):
            return 
        self.time_source = gobject.timeout_add(100, self.real_show_lyrics)
                
    def set_current_lrc(self, try_web=True, force_song=None):        
        ret = False
        if not force_song:
            force_song = self.current_song
        filename = self.lrc_manager.get_lrc(force_song, try_web)    
        if filename and os.path.exists(filename):
            if self.time_source != None:
                gobject.source_remove(self.time_source)
                self.time_source = None
                self.clear_lyrics()
            if try_web:
                gobject.idle_add(self.set_lrc_file, filename)
            else:    
                self.set_lrc_file(filename)
                ret = True
            self.set_duration(force_song.get("#duration"))    
            self.__find_flag = True
            if config.getboolean("lyrics", "status"):
                self.time_source = gobject.timeout_add(100, self.real_show_lyrics)
        else:    
            if self.time_source != None:
                gobject.source_remove(self.time_source)
                self.time_source = None
                self.clear_lyrics()
            if try_web:    
                self.set_search_fail_message("没有搜索到歌词!")
            else:    
                self.set_search_fail_message("正在搜索歌词......")
            self.__find_flag = False    
        return ret    
        
    def instant_update_lrc(self, widget, song):    
        self.update_lrc(widget, song)
