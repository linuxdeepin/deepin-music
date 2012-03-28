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
import pango
import random
from dtk.ui.listview import ListView
from dtk.ui.menu import Menu, MENU_POS_TOP_LEFT
from ui_toolkit import app_theme
import utils
from config import config
from widget.song_item import SongItem
from player import Player
from findfile import get_config_file
from library import MediaDB
from logger import Logger



class SongView(ListView):
    ''' song view. '''
    def __init__(self, *args):
        super(SongView, self).__init__(*args)
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2), ("text/plain", 0, 3)]
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_HIGHLIGHT | gtk.DEST_DEFAULT_DROP,
                           targets, gtk.gdk.ACTION_COPY)
        self.connect("drag-data-received", self.on_drag_data_received)

        
    def get_songs(self):        
        songs = []
        for song_item in self.items:
            songs.append(song_item.get_song())
        return songs    
    
    def is_empty(self):
        return len(self.items) == 0
    
    def get_loop_mode(self):
        return config.get("setting", "loop_mode")
        
    def set_loop_mode(self, value):    
        config.set("setting", "loop_mode", value)
        
    def get_previous_song(self):
        if self.is_empty():
            if config.get("setting", "empty_random") == "true":
                return MediaDB.get_random_song("local")
        else:    
            if config.get("setting", "loop_mode") == "random_mode":
                return self.get_random_song()
            if self.highlight_item != None:
                if self.highlight_item in self.items:
                    current_index = self.items.index(self.highlight_item)
                    prev_index = current_index - 1
                    if prev_index < 0:
                        prev_index = len(self.items) - 1
                    self.highlight_item = self.items[prev_index]    
            else:        
                self.highlight_item = self.items[0]
            return self.highlight_item.get_song()
    
    def get_next_song(self, manual=False):
        
        if self.is_empty():
            if config.getboolean("setting", "empty_random"):
                return MediaDB.get_random_song("local")
        else:    
            if manual:
                if config.get("setting", "loop_mode") != "random_mode":
                    return self.get_manual_song()
                else:
                    return self.get_random_song()
            
            elif config.get("setting", "loop_mode") == "list_mode":
                return self.get_manual_song()
            
            elif config.get("setting", "loop_mode") == "order_mode":            
                if self.highlight_item != None:
                    if self.highlight_item in self.items:
                        current_index = self.items.index(self.highlight_item)
                        next_index = current_index + 1
                        if next_index <= len(self.items) -1:
                            self.highlight_item = self.items[next_index]    
                            return self.highlight_item.get_song()
                return None        
            
            elif config.get("setting", "loop_mode") == "single_mode":
                if self.highlight_item != None:
                    return self.highlight_item.get_song()
                
            elif config.get("setting", "loop_mode") == "random_mode":    
                return self.get_random_song()
                        
    def get_manual_song(self):                    
        if self.highlight_item != None:
            if self.highlight_item in self.items:
                current_index = self.items.index(self.highlight_item)
                next_index = current_index + 1
                if next_index > len(self.items) - 1:
                    next_index = 0
                self.highlight_item = self.items[next_index]    
        else:        
            self.highlight_item = self.items[0]
        return self.highlight_item.get_song()
    
    def get_random_song(self):
        if self.highlight_item in self.items:
            current_index = [self.items.index(self.highlight_item)]
        else:    
            current_index = [-1]
        items_index = set(range(len(self.items)))
        remaining = items_index.difference(current_index)
        self.highlight_item = self.items[random.choice(list(remaining))]
        return self.highlight_item.get_song()

    def add_songs(self, songs, pos=None, sort=False, play=False):    
        '''Add song to songlist.'''
        if songs == None:
            return
        if not isinstance(songs, (list, tuple)):
            songs = [ songs ]

        song_items = [ SongItem(song) for song in songs if song not in self.get_songs()]
        if song_items:
            self.add_items(song_items, pos, sort)
        
        if len(songs) == 1 and play:
            self.highlight_item = song_items[0]
            gobject.idle_add(Player.play_new, self.highlight_item.get_song())
            
    def add_uris(self, uris, pos=None, sort=True):
        if uris == None:
            return
        if not isinstance(uris, (tuple, list)):
            uris = [ uris ]
        songs = []
        for uri in uris:    
            song = MediaDB.get_song(uri)
            if song:
                songs.append(song)
        self.add_songs(songs, pos, sort)        
        
    def get_current_song(self):        
        return self.highlight_item.get_song()
    
    def random_reorder(self, *args):
        with self.keep_select_status():
            random.shuffle(self.items)
            self.update_item_index()
            self.queue_draw()
            
    def set_highlight_song(self, song):        
        if not song: return 
        if SongItem(song) in self.items:
            self.set_highlight(self.items[self.items.index(SongItem(song))])
        
    def play_select_item(self):    
        if len(self.select_rows) > 0:
            self.highlight_item = self.items[self.select_rows[0]]
            Player.play_new(self.highlight_item.get_song())
        return True    
    
    def remove_select_items(self):
        self.delete_select_items()
        return True
    
    def __clear_items(self):
        self.clear()
        return True
    
    def open_song_dir(self):
        if len(self.select_rows) > 0:
            song = self.items[self.select_rows[0]].get_song()
            utils.run_command("xdg-open %s" % song.get_dir())
        return True    
    
    def move_to_trash(self):
        flag = False
        if len(self.select_rows) > 0:
            songs = [ self.items[self.select_rows[index]].get_song() for index in range(0, len(self.select_rows))]
            if self.highlight_item and self.highlight_item.get_song() in songs:
                Player.stop()
                self.highlight_item = None
                flag = True
            [ utils.move_to_trash(song.get("uri")) for song in songs ]
            self.delete_select_items()            
            if flag:
                Player.next()
        return True    
        
    def on_drag_data_received(self, widget, context, x, y, selection, info, timestamp):    
        root_y = widget.allocation.y + y
        pos = self.get_coordinate_row(root_y)
        if pos != None:
            pos = pos + 1
        
        if selection.target in ["text/uri-list", "text/plain", "text/deepin-songs"]:
            self.get_toplevel().window.set_cursor(gtk.gdk.Cursor(gtk.gdk.WATCH))
            if selection.target == "text/deepin-songs" and selection.data:
                self.add_uris(selection.data.splitlines(), pos, False)
            elif selection.target == "text/uri-list":    
                self.add_uris(selection.get_uris(), pos, False)
                # utils.async_parse_uris(selection.get_uris(), False, False, self.add_uris, pos)
            elif selection.target == "text/plain":    
                pass
            else:    
                self.get_toplevel().window.set_cursor(None)


    
    def set_sort_keyword(self, keyword, reverse=False):
        with self.keep_select_status():
            self.items = sorted(self.items, 
                                key=lambda item: item.get_song().get_sortable(keyword),
                                reverse=reverse)
            self.update_item_index()
            self.queue_draw()
            
        
    def popup_menu(self):    
        mode_dict = utils.OrderDict()
        mode_dict["single_mode"] = "单曲循环"
        mode_dict["order_mode"] = "顺序播放"
        mode_dict["list_mode"] = "列表循环"
        mode_dict["random_mode"] = "随机循环"
        
        mode_items = []
        
        for key, value in mode_dict.iteritems():
            if self.get_loop_mode() == key:
                tick = app_theme.get_pixbuf("equalizer/tick1.png")
            else:    
                tick = None
            mode_items.append((tick, value, self.set_loop_mode, key))    
        play_mode_menu = Menu(mode_items, MENU_POS_TOP_LEFT)
        
        sort_dict = utils.OrderDict()
        sort_dict["album"] = "按专辑" 
        sort_dict["genre"] = "按流派"
        sort_dict["artist"] = "按艺术家"
        sort_dict["title"] = "按歌曲名"
        sort_dict["#playcount"] = "按播放次数"
        sort_dict["#added"] = "按添加时间"
        
        sort_items = [(None, value, self.set_sort_keyword, key) for key, value in sort_dict.iteritems()]
        sort_items.append(None)
        sort_items.append((None, "随机排序", self.random_reorder))
        sub_sort_menu = Menu(sort_items, MENU_POS_TOP_LEFT)
        
        return Menu([(None, "播放歌曲",  self.play_select_item),
                     (None, "添加到列表", None),
                     (None, "移动到列表", None),
                     (None, "发送到移动盘", None),
                     None,
                     (None, "删除", self.remove_select_items),
                     (None, "从本地删除", self.move_to_trash),
                     (None, "清空列表", self.__clear_items),
                     None,
                     (None, "播放模式", play_mode_menu),
                     (None, "歌曲排序", sub_sort_menu),
                     (None, "打开文件目录", self.open_song_dir),
                     (None, "编辑歌曲信息", None),
                     ], opacity=1.0, menu_pos=1)
