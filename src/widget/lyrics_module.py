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
from widget.lyrics import LyricsWindow
from lrc_parser import LrcParser


MESSAGE_DURATION_MS = 3000

class LyricsModule(object):
    
    def __init__(self):
        self.win = LyricsWindow()
        self.lrc = LrcParser()
        self.lrc_id = -1
        self.lrc_next_id = -1
        self.current_line = 0
        self.message_source = 0
        self.song_duration = 0
        
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
        self.win.lyrics_win.show_all()
        
    def hide_all(self):    
        self.win.lyrics_win.hide_all()

lyrics_display = LyricsModule()        

