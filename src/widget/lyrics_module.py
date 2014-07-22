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
import time

from collections import OrderedDict
from contextlib import contextmanager
from dtk.ui.window import Window
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.box import ImageBox
from dtk.ui.menu import Menu
import dtk.ui.tooltip as Tooltip

from helper import Dispatcher
from widget.lyrics import DesktopLyrics, ScrollLyrics
from widget.skin import app_theme
from widget.lyrics_search import SearchUI
from widget.ui_utils import draw_alpha_mask
from widget.dialog import WinFile
from lrc_parser import LrcParser
from config import config
from player import Player
from lrc_manager import LrcManager
from constant import LRC_DESKTOP_MODE, LRC_WINDOW_MODE, PREDEFINE_COLORS
from nls import _
import utils


MESSAGE_DURATION_MS = 3000

class LyricsModule(object):
    def __init__(self):
        self.desktop_lyrics_win = DesktopLyrics()
        self.desktop_lyrics_win.connect("moved", self.adjust_toolbar_rect)
        self.desktop_lyrics_win.connect("resized", self.adjust_toolbar_rect)
        self.desktop_lyrics_win.connect("hide-bg", self.hide_toolbar)
        self.desktop_lyrics_win.connect("show-bg", self.show_toolbar)
        self.desktop_lyrics_win.connect("button-press-event", self.popup_desktop_right_menu)

        self.desktop_lyrics_win.connect("configure-event", self.lyrics_desktop_configure_event)
        
        self.scroll_lyrics = ScrollLyrics()
        self.scroll_lyrics.connect("configure-event", self.lyrcis_scroll_configure_event)
        self.scroll_lyrics.revert_button.connect("clicked", lambda w: self.switch_to_desktop_lyrics())        
        self.scroll_lyrics.connect("seek", self.seek_cb)
        self.scroll_lyrics.connect("right-press", self.scroll_right_press_cb)
        
        Player.connect("instant-new-song", self.instant_update_lrc)
        Player.connect("played", self.play_time_source)
        Player.connect("paused", self.pause_time_source)
        Player.connect("play-end", self.pause_time_source)
        Dispatcher.connect("reload-lrc", self.update_lrc)
        Dispatcher.connect("unlock-lyrics", self.__unlock_lyrics)
        Dispatcher.connect("lock-lyrics", self.__lock_lyrics)
        Dispatcher.connect("show-lyrics", lambda w : self.run())
        Dispatcher.connect("close-lyrics", lambda w: self.hide_all())
        Dispatcher.connect("search-lyrics", lambda w: self.open_search_window(w))
        Dispatcher.connect("dialog-run", self.on_dialog_run)
        Dispatcher.connect("dialog-close", self.on_dialog_close)
        
        config.connect("config-changed", self.on_config_changed)
        
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
        self.__lyrics_mode = config.getint("lyrics", "mode")
        self.__dialog_locked_flag = False        
        
        self.init_toolbar()
        
        self.current_song = None
        self.next_lrc_to_download = None
        self.condition = threading.Condition()
        self.thread = threading.Thread(target=self.func_thread)
        self.thread.setDaemon(True)
        self.thread.start()
        
    def init_toolbar(self):    
        self.toolbar = Window(window_type=gtk.WINDOW_POPUP) 
        self.toolbar.set_size_request(-1, 41)
        padding_x, padding_y = 10, 5
        play_box = gtk.HBox(spacing=2)
        play_box.connect("expose-event", self.expose_toolbar_mask)
        
        # swap played status handler
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        self.playpause_button = ToggleButton(
            app_theme.get_pixbuf("lyric/play_normal.png"),
            app_theme.get_pixbuf("lyric/pause_normal.png"),
            app_theme.get_pixbuf("lyric/play_hover.png"),
            app_theme.get_pixbuf("lyric/pause_hover.png"),
            app_theme.get_pixbuf("lyric/play_press.png"),
            app_theme.get_pixbuf("lyric/pause_press.png"),
            )
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
        
        zoom_in_align = self.__create_zoom_button("zoom_in", _("Increase the lyrics size"))
        zoom_out_align = self.__create_zoom_button("zoom_out", _("Decrease the lyrics size"))
        predefine_align = self.__create_simple_button("predefine_color", self.popup_predefine_menu, 
                                                      _("Select color theme"), True)
        lock_align = self.__create_simple_button("lock", self.__lock_lyrics, _("Lock Lyrics"))
        karaoke_align, self.karaoke_button = self.__create_single_toggle_button("karaoke", 
                                                                                self.change_karaoke_status,
                                                                                _("Karaoke on/off"))
        line_align, self.line_button = self.__create_simple_toggle_button("single_line", "double_line",
                                                                          None,_("Switch lines"))
        self.line_button_toggled_id = self.line_button.connect("toggled", self.change_line_status)
        setting_align = self.__create_simple_button("setting", self.open_setting_window, _("Open settings panel"))
        search_align = self.__create_simple_button("search", self.open_search_window, _("Search lrc file for current track"))
        close_align = self.__create_simple_button("close", self.close_lyric_window, _("Close lyrics"))
        before_align = self.__create_simple_button("before", self.before_offset, _("Lyrics rewind"))
        after_align = self.__create_simple_button("after", self.after_offset, _("Lyrics forward"))
        lrc_align = self.__create_simple_button("lrc", self.switch_to_scroll_lyrics, _("Switch to window mode"))

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
        play_box.pack_start(lrc_align, False, False)
        play_box.pack_start(search_align, False, False)
        play_box.pack_start(close_align, False, False)
        
        main_align = gtk.Alignment()
        main_align.set_padding(0, 0, padding_x, padding_x)
        main_align.set(0.5, 0.5, 0, 0)
        main_align.add(play_box)
        self.toolbar.window_frame.pack_start(main_align)
        
        self.load_button_status()
        
        
    def on_config_changed(self, obj, selection, option, value):    
        if selection == "lyrics" and option == "line_count":
            is_active = self.line_button.get_active()
            if value == "1" and not is_active:
                with self.line_button_toggled_status():
                    self.line_button.set_active(True)
            elif value == "2" and is_active:    
                with self.line_button_toggled_status():
                    self.line_button.set_active(False)
                
    @contextmanager
    def line_button_toggled_status(self):
        self.line_button.disconnect(self.line_button_toggled_id)        
        try:
            yield
        finally:
            self.line_button_toggled_id = self.line_button.connect("toggled", self.change_line_status)
        
    def expose_toolbar_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x - 9, rect.y, rect.width + 18, rect.height, "lyricsMask")
        return False
        
    def scroll_right_press_cb(self, widget, event):    
        menu_items = [
            (self.get_scroll_menu_pixbufs("lrc"), _("Switch to desktop mode"), self.switch_to_desktop_lyrics),
            None,
            (self.get_scroll_menu_pixbufs("before"), _("Lyrics rewind"), lambda : self.before_offset(None)),
            (self.get_scroll_menu_pixbufs("after"), _("Lyrics forward"), lambda : self.after_offset(None)),
            None,
            (self.get_scroll_menu_pixbufs("search"), _("Search"), lambda :self.open_search_window(None)),
            (self.get_scroll_menu_pixbufs("setting"), _("Settings"), lambda : Dispatcher.show_scroll_page()),
                      ]
        Menu(menu_items, True).show((int(event.x_root), int(event.y_root)))
        
    def popup_desktop_right_menu(self, widget, event):    
        if event.button == 3 and Player.song:
            adjust_menu_item = [(None, _("Forward 0.5 seconds"), lambda : self.after_offset(None)), 
                                (None, _("Rewind 0.5 seconds"), lambda : self.before_offset(None))]
            menu_items = [
                (None, _("Search"), lambda : self.open_search_window(None)),
                (None, _("Adjust lyrics"), Menu(adjust_menu_item)),
                None,
                (None, _("Choose local lrc"), self.allocation_lrc),
                (None, _("Open directory"), self.open_lrc_dir),
                None,
                (None, _("Settings"), lambda : self.open_setting_window(None)),
                (None, _("Switch to window mode"), lambda : self.switch_to_scroll_lyrics(None))
                ]
            
            if Player.song.get("location_lrc", None):
                menu_items.insert(4, (None, _("Lyrics acquired by Network"), self.disassociate_lrc))
            Menu(menu_items, True).show((int(event.x_root), int(event.y_root)))
            
    def allocation_lrc(self):        
        lrc_path = WinFile(False).run()
        if lrc_path:
            self.lrc_manager.allocation_lrc_file(Player.song, lrc_path)
            
    def disassociate_lrc(self):        
        self.lrc_manager.unallocation_lrc_file(Player.song)
            
    def open_lrc_dir(self):        
        save_dir = os.path.expanduser(config.get("lyrics", "save_lrc_path", "~/.lyrics"))
        utils.run_command("xdg-open %s" % save_dir)
        
    def get_scroll_menu_pixbufs(self, name):    
        return (
            app_theme.get_pixbuf("lyric/%s_normal.png" % name),
            None, None)
        
    def switch_to_scroll_lyrics(self, widget):    
        config.set("lyrics", "mode", str(LRC_WINDOW_MODE))
        self.__lyrics_mode = LRC_WINDOW_MODE
        self.hide_desktop_lyrics()
        self.show_scroll_lyrics()
        self.play_time_source()
        
    def switch_to_desktop_lyrics(self):    
        config.set("lyrics", "mode", str(LRC_DESKTOP_MODE))
        self.__lyrics_mode = LRC_DESKTOP_MODE
        self.hide_scroll_lyrics()
        self.show_desktop_lyrics()
        self.play_time_source()
    
    def seek_cb(self, widget, lyric_id, percentage):
        item_time = self.lrc.get_item_time(lyric_id)
        new_time = item_time / 1000
        if Player.song.get_type() == "cue":
            new_time += Player.song.get("seek", 0)
        Player.seek(new_time)        
        self.scroll_lyrics.set_progress(lyric_id, percentage)
        time.sleep(0.1)
        
        
    def load_button_status(self):    
        if not config.getboolean("lyrics","karaoke_mode"):
            self.karaoke_button.set_active(True)
        if config.getint("lyrics", "line_count") == 1:
            self.line_button.set_active(True)
        
    def __create_simple_button(self, name, callback, tip_msg=None, has_event=False):    
        button = ImageButton(
            app_theme.get_pixbuf("lyric/%s_normal.png" % name),
            app_theme.get_pixbuf("lyric/%s_hover.png" % name),
            app_theme.get_pixbuf("lyric/%s_press.png" % name)
            )
        if has_event:
            button.connect("button-press-event", callback)
        else:    
            button.connect("clicked", callback)
            
        if tip_msg:    
            Tooltip.text(button, tip_msg)
            
        button_align = gtk.Alignment()
        button_align.set(0.5, 0.5, 0, 0)
        button_align.add(button)
        return button_align
        
    def __create_simple_toggle_button(self, normal_name, active_name, callback=None, tip_msg=None):
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("lyric/%s_normal.png" % normal_name),
            app_theme.get_pixbuf("lyric/%s_normal.png" % active_name),
            app_theme.get_pixbuf("lyric/%s_hover.png" % normal_name),
            app_theme.get_pixbuf("lyric/%s_hover.png" % active_name),
            app_theme.get_pixbuf("lyric/%s_press.png" % normal_name),
            app_theme.get_pixbuf("lyric/%s_press.png" % active_name),
            )
        
        if callback:
            toggle_button.connect("toggled", callback)
        toggle_align = gtk.Alignment()
        toggle_align.set(0.5, 0.5, 0, 0)
        toggle_align.add(toggle_button)
        
        if tip_msg:
            Tooltip.text(toggle_button, tip_msg)
        return toggle_align, toggle_button
    
    def __create_single_toggle_button(self, normal_name, callback, tip_msg=None):
        if normal_name == "karaoke":
            toggle_button = ToggleButton(
                app_theme.get_pixbuf("lyric/%s_press.png" % normal_name),
                app_theme.get_pixbuf("lyric/%s_normal.png" % normal_name),            
                app_theme.get_pixbuf("lyric/%s_hover.png" % normal_name),
                )
        else:    
            toggle_button = ToggleButton(
                app_theme.get_pixbuf("lyric/%s_normal.png" % normal_name),
                app_theme.get_pixbuf("lyric/%s_press.png" % normal_name),            
                app_theme.get_pixbuf("lyric/%s_hover.png" % normal_name),
                )
            
        toggle_button.connect("toggled", callback)
        toggle_align = gtk.Alignment()
        toggle_align.set(0.5, 0.5, 0, 0)
        toggle_align.add(toggle_button)
        
        if tip_msg:
            Tooltip.text(toggle_button, tip_msg)
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
        if msg:
            Tooltip.text(button, msg)
        align = gtk.Alignment()
        align.set(0.5, 0.5, 0, 0)
        align.add(button)
        return align
    
    def change_font_size(self, widget, name):    
        old_size= self.desktop_lyrics_win.get_font_size()
        if name == "zoom_in":
            new_size = old_size + 2
            if new_size > 70:
                new_size = 70
            config.set("lyrics", "font_size", str(new_size))
        elif name == "zoom_out":    
            new_size = old_size - 2
            if new_size < 16:
                new_size = 16
            config.set("lyrics", "font_size", str(new_size))
        
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
        Dispatcher.close_lyrics()
        
    def open_setting_window(self, widget):
        Dispatcher.show_desktop_page()
        
    def on_dialog_close(self, sender):    
        if self.__dialog_locked_flag:
            self.desktop_lyrics_win.set_locked(False)
    
    def on_dialog_run(self, sender):
        if config.getboolean("lyrics", "status"):
            if not self.desktop_lyrics_win.get_locked():
                self.__dialog_locked_flag = True
                self.desktop_lyrics_win.set_locked()
        
    def __unlock_lyrics(self, *args):
        self.desktop_lyrics_win.set_locked(False)
        
    def __lock_lyrics(self, *args):        
        self.desktop_lyrics_win.set_locked()
    
    def change_karaoke_status(self, widget):
        self.desktop_lyrics_win.set_karaoke_mode()
    
    def change_line_status(self, widget):
        if widget.get_active():
            self.desktop_lyrics_win.set_line_count(1)
        else:    
            self.desktop_lyrics_win.set_line_count(2)
        
    def popup_predefine_menu(self, widget, event):    
        menu_dict = OrderedDict()
        menu_dict["vitality_yellow"] = _("Vitality yellow")
        menu_dict["fresh_green"]  = _("Fresh green")
        menu_dict["playful_pink"] = _("Playful pink")
        menu_dict["cool_blue"] = _("Cool blue")
        
        menu_items = []
        save_predefine_color = config.get("lyrics", "predefine_color", "vitality_yellow")
        for key, value in menu_dict.iteritems():
            item_pixbuf = None
            if key == save_predefine_color:
                item_pixbuf = (app_theme.get_pixbuf("menu/tick.png"),
                               app_theme.get_pixbuf("menu/tick_press.png"),
                               app_theme.get_pixbuf("menu/tick_disable.png"))
                
            if item_pixbuf is None:    
                menu_items.append((None, value, self.set_predefine_color, key))    
            else:    
                menu_items.append((item_pixbuf, value, self.set_predefine_color, key))    
        predefine_menu = Menu(menu_items, True)
        predefine_menu.show((int(event.x_root), int(event.y_root)))
        
    def set_predefine_color(self, key):    
        if key in PREDEFINE_COLORS.keys():
            values = PREDEFINE_COLORS[key]
            config.set("lyrics", "predefine_color", key)
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
        # self.clear_lyrics()    
        self.lrc.set_filename(filename)
        self.scroll_lyrics.set_whole_lyrics(self.lrc.scroll_lyrics)
        
    def set_scroll_played_time(self, played_time):    
        info = self.lrc.get_lyric_by_time(played_time, self.song_duration)
        if not info:
            return
        text, percentage, lyric_id = info
        self.scroll_lyrics.set_progress(lyric_id, percentage)

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
                    self.desktop_lyrics_win.set_lyric(self.current_line, real_lyric)
                if nid != lyric_id:
                    self.desktop_lyrics_win.set_current_percentage(0.0)
                self.update_next_lyric(real_id)    
            else:    
                self.desktop_lyrics_win.set_line_percentage(self.current_line, 1.0)
                self.current_line = 1 - self.current_line
                
            self.lrc_id = nid    
            self.desktop_lyrics_win.set_current_line(self.current_line)
            
        if nid == lyric_id and percentage > 0.5:    
            self.update_next_lyric(real_id)
            
        if nid == lyric_id:    
            self.desktop_lyrics_win.set_current_percentage(percentage)
                
    def update_next_lyric(self, item_id):            
        if self.desktop_lyrics_win.get_line_count() == 1:
            self.lrc_next_id = -1
            return
        item_id += 1
        real_id, real_lyric = self.get_real_lyric(item_id)
        if real_lyric == None:
            if self.lrc_next_id == -1:
                return
            else:
                self.lrc_next_id = -1
                self.desktop_lyrics_win.set_lyric(1 - self.current_line, "")
        else:        
            if self.lrc_next_id == real_id:
                return
            if real_lyric:
                self.lrc_next_id = real_id
                self.desktop_lyrics_win.set_lyric(1 - self.current_line, real_lyric)
        self.desktop_lyrics_win.set_line_percentage(1 - self.current_line, 0.0)        
        
        
    def get_real_lyric(self, item_id):             
        while True:
            if self.lrc.get_item_lyric(item_id) != "":
                break
            item_id += 1
        return item_id, self.lrc.get_item_lyric(item_id)
    
    def clear_lyrics(self):
        self.desktop_lyrics_win.set_lyric(0, "")
        self.desktop_lyrics_win.set_lyric(1, "")
        self.current_line = 0
        self.lrc_id = -1
        self.lrc_next_id = -1
        
    def set_message(self, message, duration_ms=None):    
        if not message:
            return
        self.desktop_lyrics_win.set_current_line(0)
        self.desktop_lyrics_win.set_current_percentage(0.0)
        self.desktop_lyrics_win.set_lyric(0, message)
        self.desktop_lyrics_win.set_lyric(1, "")
        
        if self.message_source != None:
            gobject.source_remove(self.message_source)
            
        if duration_ms:    
            self.message_source = gobject.timeout_add(duration_ms, self.hide_message)
            
    def hide_message(self):    
        self.desktop_lyrics_win.set_lyric(0, "")
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
        if self.__lyrics_mode == LRC_WINDOW_MODE:
            self.show_scroll_lyrics()
        else:    
            self.show_desktop_lyrics()

    def hide_toolbar(self, widget):    
        self.toolbar.hide_all()
        
    def show_toolbar(self, widget):    
        self.toolbar.show_all()
        self.toolbar.hide_all()
        l_x, l_y = self.desktop_lyrics_win.get_position()
        l_w, l_h = self.desktop_lyrics_win.get_size()
        rect = gtk.gdk.Rectangle(int(l_x), int(l_y), int(l_w), int(l_h))
        self.adjust_toolbar_rect(None, rect)
        self.toolbar.show_all()
        
    def hide_all(self):    
        config.set("lyrics", "status", "false")
        self.hide_scroll_lyrics()
        self.hide_desktop_lyrics()
        self.pause_time_source()
        
    def hide_without_config(self):    
        self.hide_scroll_lyrics()
        self.hide_desktop_lyrics()
        self.pause_time_source()
        
    def hide_scroll_lyrics(self):    
        self.scroll_lyrics.hide_all()
        
    def lyrcis_scroll_configure_event(self, widget, event):    
        if widget.get_property("visible"):
            if widget.get_resizable():
                config.set("lyrics","scroll_w","%d"%event.width)
                config.set("lyrics","scroll_h","%d"%event.height)
            config.set("lyrics","scroll_x","%d"%event.x)
            config.set("lyrics","scroll_y","%d"%event.y)
            
    def show_scroll_lyrics(self):    
        if config.get("lyrics", "scroll_x") != "-1":
            x = config.getint("lyrics", "scroll_x")
            y = config.getint("lyrics", "scroll_y")
            self.scroll_lyrics.move(int(x), int(y))
        try:    
            w = config.getint("lyrics", "scroll_w")
            h = config.getint("lyrics", "scroll_h")
            self.scroll_lyrics.resize(int(w), int(h))
        except: pass    

        if not self.__find_flag:
            self.update_lrc(None, Player.song)
        self.scroll_lyrics.show_all()        
        
    def hide_desktop_lyrics(self):    
        self.desktop_lyrics_win.hide_all()
        self.toolbar.hide_all()
        
    def lyrics_desktop_configure_event(self, widget, event):    
        if widget.get_property("visible"):
            if widget.get_resizable():
                config.set("lyrics","desktop_h","%d"%event.height)
                config.set("lyrics","desktop_w","%d"%event.width)
            config.set("lyrics","desktop_y","%d"%event.y)
            config.set("lyrics","desktop_x","%d"%event.x)
            widget.update_lyric_rects()
            
    def show_desktop_lyrics(self):    
        if config.get("lyrics", "desktop_x") == "-1":
            screen_w, screen_h = gtk.gdk.get_default_root_window().get_size()
            w , h = self.desktop_lyrics_win.get_size()
            x = screen_w / 2 - w / 2
            y = screen_h - h
        else:    
            x = config.getint("lyrics", "desktop_x")
            y = config.getint("lyrics", "desktop_y")
            
        self.desktop_lyrics_win.move(x, y)    
        try:
            d_w = config.getint("lyrics", "desktop_w")
            d_h = config.getint("lyrics", "desktop_h")
            self.desktop_lyrics_win.resize(d_w, d_h)          
        except: pass    

        if not self.__find_flag:
            self.update_lrc(None, Player.song)            
        self.desktop_lyrics_win.show_all()           

        
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
        if not config.getboolean("lyrics", "status"):
            return 
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
        if self.__lyrics_mode == LRC_WINDOW_MODE:
            self.set_scroll_played_time(played_timed)
        else:
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
        
        if self.__lyrics_mode == LRC_WINDOW_MODE:
            self.time_source = gobject.timeout_add(200, self.real_show_lyrics)
        else:    
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
                self.play_time_source()
        else:    
            if self.current_song != force_song:
                return 
            if self.time_source != None:
                gobject.source_remove(self.time_source)
                self.time_source = None
                self.clear_lyrics()
            if try_web:    
                self.set_message(self.get_default_message(force_song) + " "+ _("No lyrics found!"))
                self.scroll_lyrics.set_message(self.get_default_message(force_song) + " " +  _("No lyrics found!"))
            else:    
                self.set_search_fail_message(_("Searching for lyrics..."))
                self.scroll_lyrics.set_message(_("Searching for lyrics..."))
            self.__find_flag = False    
        return ret    
        
    def instant_update_lrc(self, widget, song):    
        if song.get_type() in ('webcast', 'cue'):
            return 
        self.scroll_lyrics.set_whole_lyrics([])
        self.set_message(self.get_default_message(song))
        self.update_lrc(widget, song)
        
    def get_default_message(self, song):    
        artist = song.get_str("artist")
        title = song.get_str("title")
        if artist:
            return "%s-%s" % (artist, title)
        else:
            return "%s" % title
