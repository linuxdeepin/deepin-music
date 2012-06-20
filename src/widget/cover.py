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
import gtk
import threading
from dtk.ui.draw import draw_pixbuf
from dtk.ui.utils import get_optimum_pixbuf_from_file


from player import Player
from library import MediaDB
from cover_manager import CoverManager, COVER_SIZE
from widget.skin import app_theme

class CoverButton(gtk.Button):
    def __init__(self):
        super(CoverButton, self).__init__()
        
        self.current_cover_pixbuf = CoverManager.get_default_cover(COVER_SIZE["x"], COVER_SIZE["y"])
        self.cover_side_pixbuf = app_theme.get_pixbuf("cover/side.png").get_pixbuf()
        self.set_size_request(self.cover_side_pixbuf.get_width(), self.cover_side_pixbuf.get_height())

        self.connect("expose-event", self.expose_button_cb)
        MediaDB.connect("simple-changed", self.update_cover)
        self.current_song = None
        self.next_cover_to_download = None
        
        self.condition = threading.Condition()
        self.thread = threading.Thread(target=self.func_thread)
        self.thread.setDaemon(True)
        self.thread.start()
        
    def expose_button_cb(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        # Draw cover side.
        draw_pixbuf(cr, self.cover_side_pixbuf, rect.x, rect.y)
        
        draw_pixbuf(cr, self.current_cover_pixbuf, rect.x + 4, rect.y + 4)
        
        return True
        
        
    def func_thread(self):    
        while True:
            self.condition.acquire()
            while not self.next_cover_to_download:
                self.condition.wait()
            next_cover_to_download = self.next_cover_to_download    
            self.next_cover_to_download = None
            self.condition.release()
            self.set_current_cover(True, next_cover_to_download)
                

    def update_default_cover(self, widget, song):            
        if not self.current_song or CoverManager.get_cover_search_str(self.current_song) != CoverManager.get_cover_search_str(song):
            pixbuf = CoverManager.get_pixbuf_from_album("", COVER_SIZE["x"], COVER_SIZE["y"])
            self.current_cover_pixbuf = pixbuf
            self.queue_draw()
            
    def update_cover(self, widget, songs):        
        if isinstance(songs, list):
            if self.current_song in songs:
                self.current_song  = songs[songs.index(self.current_song)]
        else:        
            self.current_song = songs
            
        if self.current_song is not None:        
            if not self.set_current_cover(False):
                self.condition.acquire()
                self.next_cover_to_download = self.current_song
                self.condition.notify()
                self.condition.release()
                
    def set_current_cover(self, try_web=True, force_song=None):            
        if not force_song:
            force_song = self.current_song
        filename = CoverManager.get_cover(force_song, try_web)    
        try:
            pixbuf = get_optimum_pixbuf_from_file(filename, COVER_SIZE["x"], COVER_SIZE["y"])
            self.current_cover_pixbuf = pixbuf
        except gobject.GError:    
            pass
        else:
            if try_web:
                self.current_cover_pixbuf = pixbuf
                del pixbuf
            else:    
                self.current_cover_pixbuf = pixbuf
                del pixbuf
                return CoverManager.default_cover != filename
            self.queue_draw()
            
class PlayerCoverButton(CoverButton):    
    def __init__(self):
        super(PlayerCoverButton, self).__init__()
        Player.connect("new-song", self.update_cover)
        Player.connect("instant-new-song", self.instant_update_cover)
        
    def instant_update_cover(self, widget, song):    
        if not CoverManager.get_cover(song, False):
            self.update_default_cover(widget, song)
        else:    
            self.update_cover(widget, song)
