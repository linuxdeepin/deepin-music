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

import os
import gtk
import gobject
import pango
import cairo
import random
import time

from collections import OrderedDict
from dtk.ui.window import Window
from dtk.ui.theme import Theme
from dtk.ui.utils import move_window, is_double_click
from dtk.ui.titlebar import Titlebar
from dtk.ui.listview import ListView
from dtk.ui.menu import Menu, MENU_POS_TOP_LEFT
from dtk.ui.button import Button
from dtk.ui.threads import post_gui
from dtk.ui.draw import propagate_expose

import utils
from config import config
from player import Player
from findfile import get_config_file
from library import MediaDB
from logger import Logger
from widget.dialog import WinFile, WinDir
from widget.song_item import SongItem

app_theme = Theme(os.path.join((os.path.dirname(os.path.realpath(__file__))), "../../app_theme"))

class NormalWindow(object):
    
    def __init__(self, parent=None):
        
        # Init Window
        self.window = Window()
        self.window.set_position(gtk.WIN_POS_CENTER)
        # self.window.set_modal(True)
        self.titlebar = Titlebar(["close"])
        self.titlebar.close_button.connect("clicked", lambda w: self.hide_window())
        self.titlebar.drag_box.connect('button-press-event', lambda w, e: move_window(w, e, self.window))
        self.parent = parent
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(5, 5, 10, 10)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)

        self.window.window_frame.pack_start(self.titlebar.box, False, False)
        self.window.window_frame.pack_start(main_align)
        
    def show_window(self):    
        if self.parent:
            parent_rect = self.parent.get_toplevel().get_allocation()
            self.window.move(parent_rect.x + parent_rect.width / 2, parent_rect.y + parent_rect.height / 2)
        else:    
            self.window.show_all()
            
    def hide_window(self):        
        self.window.hide_all()
        
class SongView(ListView):
    ''' song view. '''
    def __init__(self):
        
        ListView.__init__(self)
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2), ("text/plain", 0, 3)]
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_HIGHLIGHT | gtk.DEST_DEFAULT_DROP,
                           targets, gtk.gdk.ACTION_COPY)
        
        self.background_pixbuf = app_theme.get_pixbuf("skin/main.png")
        self.pl = None
        self.add_song_cache = []
        sort_key = ["sort_album", "sort_genre", "sort_artist", "sort_title", "#playcount", "#added"]
        self.sort_reverse = {key : False for key in sort_key }
        self.connect_after("drag-data-received", self.on_drag_data_received)
        self.connect("double-click-item", self.double_click_item_cb)
        # self.connect("right-press-items", self.popup_menu)
        
    def double_click_item_cb(self, widget, item, colume, x, y):    
        if item:
            self.set_highlight(item)
            Player.play_new(item.get_song())
            if Player.get_source() != self:
                Player.set_source(self)
        
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
                    highlight_item = self.items[prev_index]    
            else:        
                highlight_item = self.items[0]
            self.set_highlight(highlight_item)    
            return highlight_item.get_song()
    
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
                            highlight_item = self.items[next_index]    
                            self.set_highlight(highlight_item)
                            return highlight_item.get_song()
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
                highlight_item = self.items[next_index]    
        else:        
            highlight_item = self.items[0]
        self.set_highlight(highlight_item)    
        return highlight_item.get_song()
    
    def get_random_song(self):
        if self.highlight_item in self.items:
            current_index = [self.items.index(self.highlight_item)]
        else:    
            current_index = [-1]
        items_index = set(range(len(self.items)))
        remaining = items_index.difference(current_index)
        highlight_item = self.items[random.choice(list(remaining))]
        self.set_highlight(highlight_item)
        return highlight_item.get_song()

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
        utils.ThreadLoad(self.load_taginfo, uris, pos, sort).start()
    
    def load_taginfo(self, uris, pos=None, sort=True):
        start = time.time()
        if pos is None:
            pos = len(self.items)
        for uri in uris:    
            song = MediaDB.get_song(uri)
            self.add_song_cache.append(song)
            end = time.time()
            if end - start > 0.2:
                self.render_song(self.add_song_cache, pos, sort)
                pos += len(self.add_song_cache)
                del self.add_song_cache[:]
                start = time.time()
            else:    
                end = time.time()

        if self.add_song_cache:
            self.render_song(self.add_song_cache, pos, sort)
            del self.add_song_cache[:]
                
    @post_gui
    def render_song(self, songs, pos, sort):    
        if songs:
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
            self.visible_highlight()
            self.queue_draw()
        
    def play_select_item(self):    
        if len(self.select_rows) > 0:
            self.highlight_item = self.items[self.select_rows[0]]
            Player.play_new(self.highlight_item.get_song())
        return True    
    
    def remove_select_items(self):
        self.delete_select_items()
        return True
    
    def erase_items(self):
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
        # self.get_toplevel().window.set_cursor(gtk.gdk.Cursor(gtk.gdk.WATCH))
        root_y = widget.allocation.y + y
        try:
            pos = self.get_coordinate_row(root_y)
        except:    
            pos = None
            
        if selection.target in ["text/uri-list", "text/plain", "text/deepin-songs"]:
            if selection.target == "text/deepin-songs" and selection.data:
                self.add_uris(selection.data.splitlines(), pos, False)
            elif selection.target == "text/uri-list":    
                utils.async_parse_uris(selection.get_uris(), True, True, self.add_uris, pos)
            elif selection.target == "text/plain":    
                raw_path = selection.data
                path = eval("u" + repr(raw_path).replace("\\\\", "\\"))
                utils.async_get_uris_from_plain_text(path, self.add_uris, pos)
    
    def set_sort_keyword(self, keyword, reverse=False):
        with self.keep_select_status():
            reverse = self.sort_reverse[keyword]
            self.items = sorted(self.items, 
                                key=lambda item: item.get_song().get(keyword),
                                reverse=reverse)
            self.sort_reverse[keyword] = not reverse
            self.update_item_index()
            if self.highlight_item != None:
                self.visible_highlight()
            self.queue_draw()
    
    def get_playmode_menu(self, pos=[], align=False):
        mode_dict = OrderedDict()

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
        if align:    
            play_mode_menu = Menu(mode_items, MENU_POS_TOP_LEFT)
        else:    
            play_mode_menu = Menu(mode_items)
        if pos:
            play_mode_menu.show((pos[0], pos[1]))
        else:    
            return play_mode_menu

    def popup_delete_menu(self, x, y):    
        items = [(None, "删除", self.remove_select_items),
                 (None, "从本地删除", self.move_to_trash),
                 (None, "清空列表", self.__clear_items)]
        Menu(items).show((int(x), int(y)))
        
    def popup_add_menu(self, x, y):
        menu_items = [
            (None, "添加歌曲", self.__add_file),
            (None, "添加歌曲目录", self.__add_dir),
            ]
        return Menu(menu_items).show((x, y))

    
    def __add_file(self):
        uri = WinFile().run()        
        if uri and utils.file_is_supported(utils.get_path_from_uri(uri)):
            tags = {"uri": uri}
            try:
                song = MediaDB.get_or_create_song(tags, "local", read_from_file=True)
            except:    
                pass
            else:
                self.add_songs(song)
                
    def __add_dir(self):            
        select_dir = WinDir().run()
        if select_dir:
            utils.async_parse_uris([select_dir], True, False, self.add_uris)
            
    def async_add_uris(self, uris, follow_folder=False):        
        if not isinstance(uris, (list, tuple, set)):
            uris = [ uris ]
        utils.async_parse_uris(uris, follow_folder, True, self.add_uris)
