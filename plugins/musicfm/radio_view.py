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
import threading
import copy

from contextlib import contextmanager 
from dtk.ui.new_treeview import TreeView
from dtk.ui.iconview import IconView
from dtk.ui.threads import post_gui
from dtk.ui.scrolled_window import ScrolledWindow

from radio_item import RadioListItem, MoreIconItem, CommonIconItem

from widget.ui_utils import draw_alpha_mask, switch_tab
from widget.ui import SearchPrompt

from helper import Dispatcher
from player import Player
from posterlib import fmlib
from cover_manager import cover_thread_pool

import utils
from xdg_support import get_config_file
from nls import _

class RadioView(TreeView):    
    __gsignals__ = {
        "begin-add-items" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "empty-items" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        }
    
    def __init__(self, *args, **kwargs):
        TreeView.__init__(self, *args, **kwargs)
        targets = [("text/deepin-radios", gtk.TARGET_SAME_APP, 1),]        
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP, targets, gtk.gdk.ACTION_COPY)
        
        self.connect_after("drag-data-received", self.on_drag_data_received)
        self.connect("double-click-item", self.on_double_click_item)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("delete-select-items", self.try_emit_empty_signal)
        
        Dispatcher.connect("play-radio", self.on_dispatcher_play_radio)
        Player.connect("play-end", self.on_play_end)
        
        self.lock = threading.Lock()
        
        self.current_index = 0
        self.playlist = []
        self.limit_number = 25
        self.preview_db_file = get_config_file("preview_radios.db")
        
        # Dispatcher.connect("being-quit", lambda obj: self.save())

        # if MediaDB.isloaded():
        #     self.__on_db_loaded(MediaDB)
        # else:    
        #     MediaDB.connect("loaded", self.__on_db_loaded)
    @contextmanager        
    def keep_list_lock(self):
        self.lock.acquire()
        try:
            yield
        except Exception, e:    
            self.logwarn("keep stream state lock failed %s", e)
            
        else:    
            self.lock.release()
        
    def __on_db_loaded(self, db):        
        self.load()
        
    def draw_mask(self, cr, x, y, width, height):    
        draw_alpha_mask(cr, x, y, width, height, "layoutLeft")
        
    def is_empty(self):
        return len(self.get_items()) == 0        
        
    def try_emit_empty_signal(self, widget, items):    
        if len(self.get_items()) <= 0:
            self.emit("empty-items")
            
    def emit_add_signal(self):            
        self.emit("begin-add-items")
        
    def on_dispatcher_play_radio(self, obj, channel_info):    
        self.add_channels([channel_info], pos=0, play=True)
        
    def on_double_click_item(self, widget, item, column, x, y):    
        if item:
            self.reset_playlist()
            self.set_highlight_item(item)
            self.fetch_playlist(play=True)
            
    def reset_playlist(self):        
        with self.keep_list_lock():
            self.playlist = []
            self.current_index = 0
            
    @utils.threaded        
    def on_play_end(self, player):        
        song = player.song
        if song:
            if song.get_type() == "douban":
                fmlib.played_song(self.highlight_item.channel_id, 
                                  song.get("sid"), song.get("aid"))
            
    @utils.threaded       
    def fetch_playlist(self, play=False):
        if not self.highlight_item: return
        songs = fmlib.new_playlist(self.highlight_item.channel_id, self.get_history_sids())
        
        with self.keep_list_lock():
            self.playlist = songs
            if songs and play:
                Player.play_new(songs[0])
                self.current_index = 0
                Player.set_source(self)
            
    def get_next_song(self, maunal=False):        
        if maunal:
            self.mark_as_skip()
        
        if self.playlist is None:
            self.fetch_playlist()
            return     
        
        with self.keep_list_lock():
            self.current_index += 1
            current_index = self.current_index        
            self.extent_playlist()
            return self.playlist[current_index]
    
    @utils.threaded
    def mark_as_skip(self):
        if Player.song:
            try:
                songs = fmlib.skip_song(self.highlight_item.channel_id, 
                                        Player.song.get("sid"), Player.song.get("aid"), 
                                        self.get_history_sids())
                self.load_more_songs(songs)
            except:    
                pass
            
    def songs_to_sids(self, songs):        
        if not songs:
            return []
        return map(lambda song: song.get("sid"), songs)
            
    def get_history_sids(self):        
        history_sids = self.songs_to_sids(self.playlist[:self.current_index])
        return history_sids
    
    def get_rest_sids(self):
        rest_songs = self.playlist[self.current_index + 1:]
        rest_sids = self.songs_to_sids(rest_songs)
        return rest_sids
    
    @utils.threaded
    def extent_playlist(self):
        if len(self.playlist) - self.current_index - 1 < 5:
            songs = fmlib.new_playlist(self.highlight_item.channel_id, self.get_history_sids())            
            if songs:
                self.playlist.extend(songs)
    
    def load_more_songs(self, songs):
        if len(self.playlist) - self.current_index - 1 < 5:
            self.playlist.extend(songs)
            
    def get_previous_song(self):
        self.current_index -= 1
        if self.current_index < 0:
            self.current_index = 0
        return self.playlist[self.current_index]    
    
    def on_drag_data_received(self, widget, context, x, y, selection, info, timestamp):
        root_y = widget.allocation.y + y
        try:
            pos = self.get_coordinate_row(root_y)
        except: pos = None
        
        if pos == None:
            pos = len(self.get_items())
            
        if selection.target == "text/deepin-radios":    
            channels_data =  selection.data
            channel_infos =  eval(channels_data)
            self.add_channels(channel_infos, pos)    
            
    def add_channels(self, channels, pos=None, sort=False, play=False):        
        if not channels:
            return
        if not isinstance(channels, (list, tuple, set)):
            channels = [ channels ]
            
        channel_items = [ RadioListItem(info) for info in channels ]    
        channel_items = filter(lambda item : item not in self.get_items(), channel_items)
        if channel_items:
            if not self.get_items():
                self.emit_add_signal()
            self.add_items(channel_items, pos, sort)    
            
            if len(self.get_items()) > self.limit_number:
                being_delete_items = self.get_items()[self.limit_number:]
                if self.highlight_item in being_delete_items:
                    being_delete_items.remove(self.highlight_item)
                self.delete_items(being_delete_items)
            
        if len(channels) >= 1 and play:
            del self.select_rows[:]
            self.queue_draw()
            self.set_highlight_channel(channels[0])
            self.reset_playlist()
            self.fetch_playlist(play=True)
            
                        
    def set_highlight_channel(self, channel):        
        if not channel: return 
        item_index = None
        for index, item in enumerate(self.get_items()):
            if channel.get("id") == item.channel_id:
                item_index = index
                break
        if item_index is not None:    
            self.set_highlight_item(self.get_items()[item_index])
            self.visible_highlight()
            self.queue_draw()
        else:    
            self.add_channels([channel], pos=0)
            self.set_highlight_item(self.get_items()[0])
            self.visible_highlight()
            self.queue_draw()
            
    def on_button_press_event(self, widget, event):        
        pass
    
    def save(self):
        channel_infos = [ item.channel_info for item in self.get_items() ]
        utils.save_db(channel_infos, self.preview_db_file)
        
    def load(self):    
        try:
            channel_infos = utils.load_db(self.preview_db_file)
        except:    
            channel_infos = None
            
        if channel_infos is not None:    
            self.add_channels(channel_infos)
            
TAG_HOT = 1    
TAG_FAST = 2
TAG_GENRE = 3
TAG_SEARCH = 4

class RadioIconView(IconView):    
    
    def __init__(self, tag, limit=10, has_add=True, padding_x=0, padding_y=0):
        IconView.__init__(self, padding_x=padding_x, padding_y=padding_y)
        
        targets = [("text/deepin-radios", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        self.connect("drag-data-get", self.__on_drag_data_get) 

        self.tag = tag
        self.__start = 0
        self.__limit = limit
        self.__fetch_thread_id = 0
        self.__render_id = 0
        self.__genre_id = "335"
        self.__keyword = ""
        
        self.connect("single-click-item", self.on_iconview_single_click_item)
        self.connect("motion-item", self.on_motion_item)        
        if has_add:
            self.add_items([MoreIconItem()])
        
    def __on_drag_data_get(self, widget, context, selection, info, timestamp):        
        item = widget.highlight_item
        if not item: return
        channel_info = str([ item.chl ])
        selection.set("text/deepin-radios", 8, channel_info)
    
    def set_genre_id(self, genre_id):    
        self.__genre_id = genre_id
        self.__start = 0
        
    def set_keyword(self, keyword):    
        self.__keyword = keyword
        self.__start = 0
        
    def fetch_failed(self):
        pass
    
    def fetch_successed(self):
        pass
        
    def on_iconview_single_click_item(self, widget, item, x, y):    
        if item:
            if hasattr(item, "is_more") and item.mask_flag:
                self.start_fetch_channels()
            else:    
                if item.mask_flag:
                    Dispatcher.emit("play-radio", item.chl)
                    
    def start_fetch_channels(self):                
        self.__fetch_thread_id += 1
        fetch_thread_id = copy.deepcopy(self.__fetch_thread_id)
        utils.ThreadFetch(
            fetch_funcs=(self.fetch_channels, (self.__start,)),
            success_funcs=(self.load_channels, (fetch_thread_id, self.__start)),
            fail_funcs=(self.fetch_failed, ())).start()
                
    def fetch_channels(self, start):    
        if self.tag == TAG_HOT:
            ret = fmlib.get_hot_chls(start=start, limit=self.__limit)
        elif self.tag == TAG_FAST:    
            ret = fmlib.get_uptrending_chls(start=start, limit=self.__limit)
        elif self.tag == TAG_GENRE:    
            ret = fmlib.get_genre_chls(genre_id=self.__genre_id, start=start, limit=self.__limit)
        elif self.tag == TAG_SEARCH:    
            ret = fmlib.get_search_chls(keyword=self.__keyword, start=start, limit=self.__limit)
        return  ret.get("data", {}).get("channels", [])
            
    @post_gui
    def load_channels(self, channels, thread_id, start):
        channel_items = []
        thread_items = []
        if thread_id != self.__fetch_thread_id:
            return
        if not channels:
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
        
        self.fetch_successed()
        
    def add_radios(self, items):    
        pass
            
    def draw_mask(self, cr, x, y, w, h):    
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")
        return False
    
    def clear_items(self, add_more=True):
        self.__start = 0
        self.clear()
        if add_more:
            self.add_items([MoreIconItem()])
        
    def get_scrolled_window(self):   
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(self)
        return scrolled_window
    
    def on_motion_item(self, widget, item, x, y):
        if not hasattr(item, "is_more"):
            item.try_show_notify(x, y)

            
class RadioSearchView(gtk.VBox):            
    
    def __init__(self, source_tab):
        gtk.VBox.__init__(self)
        self.source_tab = source_tab        
        
        self.radio_view = RadioIconView(tag=TAG_SEARCH, padding_y=10)
        self.radio_view_sw = self.radio_view.get_scrolled_window()
        self.radio_view.fetch_failed = self.switch_to_search_prompt
        self.radio_view.fetch_successed = self.switch_to_radio_view
        self.radio_view.connect_after("single-click-item", self.on_radio_view_click_item)
        self.search_prompt = SearchPrompt(_("MusicFM"))
        self.add(self.radio_view_sw)
        self.keyword = ""
        
    def start_search_songs(self, keyword):    
        self.radio_view.clear_items()
        self.radio_view.set_keyword(keyword)
        self.radio_view.start_fetch_channels()
        self.keyword = keyword
        
    def switch_to_radio_view(self):   
        switch_tab(self, self.radio_view_sw)
        
    def switch_to_search_prompt(self):    
        self.search_prompt.update_keyword(self.keyword)
        switch_tab(self, self.search_prompt)
        
    def on_radio_view_click_item(self, *args):
        Dispatcher.emit("switch-source", self.source_tab)
