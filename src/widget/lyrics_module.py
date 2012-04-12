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
from widget.lyrics import desktop_lyrics
from lrc_parser import lrc_parser
from config import config
from widget.toolbar import lyric_toolbar
from player import Player
from lrc_manager import lrc_manager
from lyrics_search import search_ui


MESSAGE_DURATION_MS = 3000

class LyricsModule(object):
    
    def __init__(self):
        self.win = desktop_lyrics
        self.win.connect("moved", self.adjust_toolbar_rect)
        self.win.connect("resized", self.adjust_toolbar_rect)
        self.win.connect("hide-bg", self.hide_toolbar)
        self.win.connect("show-bg", self.show_toolbar)
        self.time_source = 0
        
        Player.connect("instant-new-song", self.instant_update_lrc)
        # Player.connect("play-end", self.stop_source_time)
        search_ui.connect("finish", self.update_lrc)
        
        self.lrc = lrc_parser
        self.lrc_id = -1
        self.lrc_next_id = -1
        self.current_line = 0
        self.message_source = 0
        self.song_duration = 0
        
        self.current_song = None
        self.next_lrc_to_download = None
        self.condition = threading.Condition()
        self.thread = threading.Thread(target=self.func_thread)
        self.thread.setDaemon(True)
        self.thread.start()
        
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
        if not filename and self.message_source == 0:
            self.clear_lyrics()
        if filename and self.message_source != 0:
            self.clear_message()
        self.lrc.set_filename(filename)
        
    def stop_source_time(self, player):    
        if self.time_source != 0:
            gobject.source_remove(self.time_source)

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
                    self.win.set_lyric(self.current_line, real_lyric)
                if nid != lyric_id:
                    self.win.set_current_percentage(0.0)
                self.update_next_lyric(real_id)    
            else:    
                self.win.set_line_percentage(self.current_line, 1.0)
                self.current_line = 1 - self.current_line
                
            self.lrc_id = nid    
            self.win.set_current_line(self.current_line)
            
        if nid == lyric_id and percentage > 0.5:    
            self.update_next_lyric(real_id)
            
        if nid == lyric_id:    
            self.win.set_current_percentage(percentage)
                
    def update_next_lyric(self, item_id):            
        if self.win.get_line_count() == 1:
            self.lrc_next_id = -1
            return
        item_id += 1
        real_id, real_lyric = self.get_real_lyric(item_id)
        if real_lyric == None:
            if self.lrc_next_id == -1:
                return
            else:
                self.lrc_next_id = -1
                self.win.set_lyric(1 - self.current_line, "")
        else:        
            if self.lrc_next_id == real_id:
                return
            if real_lyric:
                self.lrc_next_id = real_id
                self.win.set_lyric(1 - self.current_line, real_lyric)
        self.win.set_line_percentage(1 - self.current_line, 0.0)        
        
                
    def get_real_lyric(self, item_id):             
        while True:
            if self.lrc.get_item_lyric(item_id) != "":
                break
            item_id += 1
        return item_id, self.lrc.get_item_lyric(item_id)
    
    def clear_lyrics(self):
        self.win.set_lyric(0, "")
        self.win.set_lyric(1, "")
        self.current_line = 0
        self.lrc_id = -1
        self.lrc_next_id = -1
        
    def set_message(self, message, duration_ms):    
        if not message:
            return
        self.win.set_current_line(0)
        self.win.set_current_percentage(1.0)
        self.win.set_lyric(0, message)
        self.win.set_lyric(1, "")
        
        if self.message_source != 0:
            gobject.source_remove(self.message_source)
        self.message_source = gobject.timeout_add(duration_ms, self.hide_message)
            
    def hide_message(self):    
        self.win.set_lyric(0, "")
        self.message_source = 0
        return False
    
    def clear_message(self):
        if self.message_source != 0:
            gobject.source_remove(self.message_source)
            self.hide_message()
    
    def set_search_message(self, message):
        self.set_message(message, -1)
        
    def set_search_fail_message(self, message):
        self.set_message(message, MESSAGE_DURATION_MS)
        
    def set_download_fail_message(self, message):
        self.set_message(message, MESSAGE_DURATION_MS)
        
    def run(self):
        screen_w, screen_h = gtk.gdk.get_default_root_window().get_size()
        w , h = self.win.lyrics_win.get_size()
        try:
            x = config.getint("lyrics", "x")
            y = config.getint("lyrics", "y")
        except:    
            x = screen_w / 2 - w / 2
            y = screen_h - h

        self.win.lyrics_win.move(x, y) 
        self.win.lyrics_win.show_all()           
        config.set("lyrics", "status", "true")
        
    def hide_toolbar(self, widget):    
        lyric_toolbar.hide_all()
        
    def show_toolbar(self, widget):    
        lyric_toolbar.show_all()
        lyric_toolbar.hide_all()
        l_x, l_y = self.win.lyrics_win.get_position()
        l_w, l_h = self.win.lyrics_win.get_size()
        rect = gtk.gdk.Rectangle(int(l_x), int(l_y), int(l_w), int(l_h))
        self.adjust_toolbar_rect(None, rect)
        lyric_toolbar.show_all()

        
    def hide_all(self):    
        x, y = self.win.lyrics_win.get_position()
        config.set("lyrics", "x", str(x))
        config.set("lyrics", "y", str(y))
        self.win.lyrics_win.hide_all()
        lyric_toolbar.hide_all()
        config.set("lyrics", "status", "false")
        
    def adjust_toolbar_rect(self, widget, rect):    
        screen_w, screen_h = gtk.gdk.get_default_root_window().get_size()
        centre_x = rect.x + rect.width / 2
        l_w, l_h = lyric_toolbar.get_size()
        l_x = centre_x - l_w / 2
        if rect.y <  l_h:
            l_y = rect.y + rect.height
        elif rect.y > screen_h - rect.height:    
            l_y = rect.y - l_h
        else:    
            l_y = rect.y - l_h
        lyric_toolbar.move(l_x, l_y)    
        
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
                
    def set_current_lrc(self, try_web=True, force_song=None):        
        ret = False
        if not force_song:
            force_song = self.current_song
        filename = lrc_manager.get_lrc(force_song, try_web)    
        if filename and os.path.exists(filename):
            if self.time_source != 0:
                gobject.source_remove(self.time_source)
                self.clear_lyrics()
            if try_web:
                gobject.idle_add(self.set_lrc_file, filename)
            else:    
                self.set_lrc_file(filename)
                ret = True
            self.set_duration(force_song.get("#duration"))    
            self.time_source = gobject.timeout_add(100, self.real_show_lyrics)    
        else:    
            if self.time_source != 0:
                gobject.source_remove(self.time_source)
                self.clear_lyrics()    
            if try_web:    
                self.set_search_fail_message("没有搜索到歌词!")
            else:    
                self.set_search_fail_message("正在搜索歌词......")
        return ret    
        
    def instant_update_lrc(self, widget, song):    
        self.update_lrc(widget, song)
        
        
lyrics_display = LyricsModule()        



