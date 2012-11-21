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
import gobject
import pango
from dtk.ui.listview import ListView
from dtk.ui.draw import draw_pixbuf, draw_text
from dtk.ui.iconview import IconView
from dtk.ui.threads import post_gui

from widget.ui_utils import draw_single_mask, draw_alpha_mask, draw_line
from widget.skin import app_theme
from widget.radio_item import RadioListItem, MoreIconItem, CommonIconItem
from helper import Dispatcher
from player import Player
from song import Song
from doubanfm import fmlib
from cover_manager import cover_thread_pool

import utils

class RadioView(ListView):    
    __gsignals__ = {
        "begin-add-items" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "empty-items" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        }
    
    def __init__(self, *args, **kwargs):
        ListView.__init__(self, *args, **kwargs)
        targets = [("text/deepin-webcasts", gtk.TARGET_SAME_APP, 1),]        
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP, targets, gtk.gdk.ACTION_COPY)
        
        # self.connect_after("drag-data-received", self.on_drag_data_received)
        self.connect("double-click-item", self.on_double_click_item)
        # self.connect("button-press-event", self.on_button_press_event)
        # self.connect("delete-select-items", self.try_emit_empty_signal)
        Dispatcher.connect("play-radio", self.on_dispatcher_play_radio)
        self.set_expand_column(0)
        
        self.current_index = 0
        self.current_cid = None
        self.playlist = None
        
    def draw_mask(self, cr, x, y, width, height):    
        draw_alpha_mask(cr, x, y, width, height, "layoutLeft")
        
    def try_emit_empty_signal(self, widget, items):    
        if len(self.items) <= 0:
            self.emit("empty-items")
            
    def emit_add_signal(self):            
        self.emit("begin-add-items")
        
    def on_dispatcher_play_radio(self, obj, channel_info):    
        self.add_items([RadioListItem(channel_info)])
        self.fetch_playlist(channel_info.get("id"), True)
        
    def on_double_click_item(self, widget, item, column, x, y):    
        if item:
            self.current_cid = item.channel_info.get("id")
            self.fetch_playlist(self.current_cid, True)
            
    @utils.threaded       
    def fetch_playlist(self, channel_id, play=False):
        songs = fmlib.new_playlist_no_user(channel_id)
        self.playlist = songs
        if songs and play:
            Player.play_new(songs[0])
            self.current_index = 0
            Player.set_source(self)
            
    def get_next_song(self, maunal=False):        
        if self.playlist is None:
            self.fetch_playlist(self.current_cid)
            return     
        self.current_index += 1
        current_index = self.current_index        
        
        if self.current_index == len(self.playlist) - 1:
            print "next"
            self.current_index = -1
            self.fetch_playlist(self.current_cid)
        return self.playlist[current_index]
    
    
    def get_previous_song(self):
        self.current_index -= 1
        if self.current_index < 0:
            self.current_index = 0
        return self.playlist[self.current_index]    
    
TAG_HOT = 1    
TAG_FAST = 2
TAG_GENRE = 3

class RadioIconView(IconView):    
    
    def __init__(self, tag, *args, **kwargs):
        IconView.__init__(self, *args, **kwargs)
        
        # targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        # icon_view.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        # icon_view.connect("drag-data-get", self.__on_drag_data_get) 
        # icon_view.connect("double-click-item", self.on_iconview_double_click_item)
        self.tag = tag
        self.__start = 0
        self.__limit = 10
        self.__fetch_thread_id = 0
        self.__render_id = 0
        
        self.connect("single-click-item", self.on_iconview_single_click_item)
        self.add_items([MoreIconItem()])
        
    def on_iconview_single_click_item(self, widget, item, x, y):    
        if item:
            if hasattr(item, "is_more") and item.mask_flag:
                self.__fetch_thread_id += 1
                utils.ThreadFetch(
                    fetch_funcs=(self.fetch_channels, (self.__start,)),
                    success_funcs=(self.load_channels, (self.__fetch_thread_id, self.__start))).start()
            else:    
                if item.mask_flag:
                    Dispatcher.emit("play-radio", item.chl)
                
    def fetch_channels(self, start):    
        if self.tag == TAG_HOT:
            ret = fmlib.get_hot_chls(start=start, limit=self.__limit)
        elif self.tag == TAG_FAST:    
            ret = fmlib.get_uptrending_chls(start=start, limit=self.__limit)
        return  ret.get("data", {}).get("channels", [])
            
    @post_gui
    def load_channels(self, channels, thread_id, start):
        channel_items = []
        thread_items = []
        if thread_id != self.__fetch_thread_id:
            return
        for hot_chl in channels:
            common_item = CommonIconItem(hot_chl)
            if not common_item.is_loaded_cover:
                thread_items.append(common_item)
            channel_items.append(common_item)    
            
        if thread_items:    
            cover_thread_pool.add_missions(thread_items)
        self.add_items(channel_items, -1)    
        self.__start = start + self.__limit
        
    def add_radios(self, items):    
        pass
            
    def draw_mask(self, cr, x, y, w, h):    
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")
        draw_line(cr, (x + 1, y), 
                  (x + 1, y + h), "#b0b0b0")
        return False
    
