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
import random
import time

from collections import OrderedDict
from dtk.ui.listview import ListView
from dtk.ui.utils import alpha_color_hex_to_cairo
from dtk.ui.menu import Menu
from dtk.ui.threads import post_gui
from dtk.ui.draw import draw_vlinear

import utils
import common
from config import config
from player import Player
from helper import Dispatcher
from library import MediaDB
from widget.dialog import WinFile, WinDir
from widget.song_item import SongItem
from widget.skin import app_theme
from widget.song_editor import SongEditor
from source.local import ImportPlaylistJob

class SongView(ListView):
    ''' song view. '''
    def __init__(self):
        
        ListView.__init__(self)
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2), ("text/plain", 0, 3)]
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_HIGHLIGHT | gtk.DEST_DEFAULT_DROP,
                           targets, gtk.gdk.ACTION_COPY)
        
        self.pl = None
        self.add_song_cache = []
        sort_key = ["album", "genre", "artist", "title", "#playcount", "#added"]
        self.sort_reverse = {key : False for key in sort_key }
        self.connect_after("drag-data-received", self.on_drag_data_received)
        self.connect("double-click-item", self.double_click_item_cb)
        self.connect("button-press-event", self.button_press_cb)
        
        MediaDB.connect("removed", self.__remove_songs)
        
        
    def double_click_item_cb(self, widget, item, colume, x, y):    
        if item:
            self.set_highlight(item)
            Player.play_new(item.get_song())
            if Player.get_source() != self:
                Player.set_source(self)
                
    def draw_mask(self, cr, x, y, width, height):            
        color_info = app_theme.get_alpha_color("playlistMiddle").get_color_info()
        cr.set_source_rgba(*alpha_color_hex_to_cairo(color_info))
        cr.rectangle(x, y, width, height)
        cr.fill()
        
    def draw_item_hover(self, cr, x, y, w, h):
        draw_vlinear(cr, x, y, w, h, app_theme.get_shadow_color("songviewHover").get_color_info())
        
    def draw_item_select(self, cr, x, y, w, h):    
        draw_vlinear(cr, x, y, w, h, app_theme.get_shadow_color("songviewSelect").get_color_info())
        
    def draw_item_highlight(self, cr, x, y, w, h):    
        draw_vlinear(cr, x, y, w, h, app_theme.get_shadow_color("songviewHighlight").get_color_info())
        
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
        
        if len(songs) >= 1 and play:
            self.set_highlight_song(songs[0])
            gobject.idle_add(Player.play_new, self.highlight_item.get_song())
            
    def play_uris(self, uris, pos=None, sort=True):        
        self.get_toplevel().window.set_cursor(None)
        songs = []
        
        for uri in uris:
            song = MediaDB.get_song(uri)
            if song:
                songs.append(song)
                
        if not songs:        
            return
        if sort: songs.sort()
        self.add_songs(song, pos, sort, True)
            
    def add_uris(self, uris, pos=None, sort=True):
        if uris == None:
            return
        if not isinstance(uris, (tuple, list)):
            uris = [ uris ]

        uris = [ utils.get_uri_from_path(uri) for uri in uris ]    
        utils.ThreadLoad(self.load_taginfo, uris, pos, sort).start()
        # self.load_taginfo(uris, pos, sort)
    
    def load_taginfo(self, uris, pos=None, sort=True):
        start = time.time()
        if pos is None:
            pos = len(self.items)
        for uri in uris:    
            song = MediaDB.get_song(uri)
            if not song:
                continue
            self.add_song_cache.append(song)
            end = time.time()
            if end - start > 0.2:
                self.render_song(self.add_song_cache, pos, sort)
                pos += len(self.add_song_cache)
                del self.add_song_cache[:]
                start = time.time()

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
            utils.open_file_directory(song.get_path())
    
    def open_song_editor(self):
        if len(self.select_rows) > 0:
            index = self.select_rows[0]
            SongEditor(self.get_songs(), index).show_all()
    
    def move_to_trash(self):
        flag = False
        if len(self.select_rows) > 0:
            songs = [ self.items[self.select_rows[index]].get_song() for index in range(0, len(self.select_rows))]
            if self.highlight_item and self.highlight_item.get_song() in songs:
                Player.stop()
                self.highlight_item = None
                flag = True
            MediaDB.remove(songs)    
            [ utils.move_to_trash(song.get("uri")) for song in songs ]
            self.delete_select_items()            
            if flag:
                Player.next()
        return True    
        
    def on_drag_data_received(self, widget, context, x, y, selection, info, timestamp):    
        root_y = widget.allocation.y + y
        try:
            pos = self.get_coordinate_row(root_y)
        except:    
            pos = None
            
        if pos == None:    
            pos = len(self.items)
            
        if selection.target in ["text/uri-list", "text/plain", "text/deepin-songs"]:
            if selection.target == "text/deepin-songs" and selection.data:
                self.add_uris(selection.data.splitlines(), pos, False)
            elif selection.target == "text/uri-list":    
                utils.async_parse_uris(selection.get_uris(), True, False, self.add_uris, pos)
            elif selection.target == "text/plain":    
                raw_path = selection.data
                path = eval("u" + repr(raw_path).replace("\\\\", "\\"))
                utils.async_get_uris_from_plain_text(path, self.add_uris, pos)
    
    def set_sort_keyword(self, keyword, reverse=False):
        with self.keep_select_status():
            reverse = self.sort_reverse[keyword]
            self.items = sorted(self.items, 
                                key=lambda item: item.get_song().get_sortable(keyword),
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
                tick = app_theme.get_pixbuf("menu/tick.png")
                mode_items.append(((tick, None), value, self.set_loop_mode, key))                    
            else:    
                tick = None
                mode_items.append((None, value, self.set_loop_mode, key))                    

        if pos:
            Menu(mode_items, True).show((pos[0], pos[1]))
        else:    
            return Menu(mode_items)
        
    def button_press_cb(self, widget, event):    
        if event.button == 3 and not self.items:
            self.popup_add_menu(int(event.x_root), int(event.y_root))

    def popup_delete_menu(self, x, y):    
        items = [(None, "删除", self.remove_select_items),
                 (None, "从本地删除", self.move_to_trash),
                 (None, "清空列表", self.erase_items)]
        Menu(items, True).show((int(x), int(y)))
        
    def popup_add_menu(self, x, y):
        menu_items = [
            (None, "文件", self.add_file),
            (None, "文件夹(包含子目录)", self.recursion_add_dir),
            (None, "文件夹", self.add_dir),
            ]
        Menu(menu_items, True).show((x, y))
        
    def get_add_menu(self):    
        menu_items = [
            (None, "文件", self.add_file),
            (None, "文件夹(包含子目录)", self.recursion_add_dir),
            (None, "文件夹", self.add_dir),
            ]
        return Menu(menu_items)
    
    def add_file(self, filename=None, play=False):
        if filename is None:
            uri = WinFile().run()        
        else:    
            uri = utils.get_uri_from_path(filename)
            
        if uri and common.file_is_supported(utils.get_path_from_uri(uri)):
            tags = {"uri": uri}
            try:
                song = MediaDB.get_or_create_song(tags, "local", read_from_file=True)
            except:    
                pass
            else:
                self.add_songs(song, play=play)
                
    def add_dir(self):            
        select_dir = WinDir().run()
        if select_dir:
            utils.async_parse_uris([select_dir], True, False, self.add_uris)
            
    def recursion_add_dir(self):        
        pos = len(self.items)
        ImportPlaylistJob(None, self.add_songs, pos)
            
    def async_add_uris(self, uris, follow_folder=True):        
        if not isinstance(uris, (list, tuple, set)):
            uris = [ uris ]
        print uris    
        utils.async_parse_uris(uris, follow_folder, True, self.add_uris)
        
    def __remove_songs(self, db, song_type, songs):    
        indexs = []
        flag = False
        view_songs = self.get_songs()
        
        if self.highlight_item and self.highlight_item.get_song() in songs:
            Player.stop()
            self.highlight_item = None
            flag = True
        
        for song in songs:
            if song in view_songs:
                indexs.append(view_songs.index(song))
        if indexs:        
            for index in indexs:
                del self.items[index]
            self.update_item_index()
            self.update_vadjustment()        
            self.queue_draw()    
            
        if flag:    
            if len(self.items) > 0:
                item = self.items[0]
                self.set_highlight(item)
                Player.play_new(item.get_song())

class MultiDragSongView(ListView):        
    def __init__(self, *args, **kward):
        ListView.__init__(self, *args, **kward)
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        self.sorts = [
            (lambda item: item.get_song().get_sortable("title"), cmp),
            (lambda item: item.get_song().get_sortable("artist"), cmp),
            (lambda item: item.get_song().get_sortable("album"), cmp),
            (lambda item: item.get_song().get_sortable("#added"), cmp),
            ]
        
        sort_key = ["album", "genre", "artist", "title", "#playcount", "#added"]
        self.sort_reverse = {key : False for key in sort_key }
        
        self.set_expand_column(0)
        self.connect("drag-data-get", self.__on_drag_data_get) 
        self.connect("double-click-item", self.__on_double_click_item)
        self.connect("right-press-items", self.popup_right_menu)
        
    def get_selected_songs(self):    
        songs = []
        if len(self.select_rows) > 0:
            songs = [ self.items[index].get_song() for index in self.select_rows ]
        return songs
            
    def __on_drag_data_get(self, widget, context, selection, info, timestamp):    
        songs = self.get_selected_songs()
        if not songs:
            return
        songs.sort()
        list_uris = list([ song.get("uri") for song in songs])
        selection.set("text/deepin-songs", 8, "\n".join(list_uris))
        selection.set_uris(list_uris)
        
    def __on_double_click_item(self, widget, item, colume, x, y):    
        if item:
            Dispatcher.play_and_add_song(item.get_song())
        
    def add_songs(self, songs, pos=None, sort=False):        
        if not songs:
            return 
        if not isinstance(songs, (list, tuple, set)):
            songs = [ songs ]
        song_items = [ SongItem(song, True) for song in songs if song not in self.get_songs() ]    
        
        if song_items:
            self.add_items(song_items, pos, sort)

    def get_songs(self):        
        songs = []
        for song_item in self.items:
            songs.append(song_item.get_song())
        return songs    
            
    def is_empty(self):        
        return len(self.items) == 0
    
    def set_sort_keyword(self, keyword, reverse_able=False):
        with self.keep_select_status():
            reverse = self.sort_reverse[keyword]
            self.items = sorted(self.items, 
                                key=lambda item: item.get_song().get_sortable(keyword),
                                reverse=reverse)
            if reverse_able:
                self.sort_reverse[keyword] = not reverse
            self.update_item_index()
            self.queue_draw()
            
    def open_song_dir(self):
        if len(self.select_rows) > 0:
            song = self.items[self.select_rows[0]].get_song()
            utils.open_file_directory(song.get_path())
        return True    
    
    def emit_to_playlist(self):
        if len(self.select_rows) > 0:
            songs = [self.items[index].get_song() for index in self.select_rows]
            Dispatcher.add_songs(songs)
    
    def play_song(self):
        if len(self.select_rows) > 0:
            song = self.items[self.select_rows[0]].get_song()
            Dispatcher.play_and_add_song(song)
        return True    
    
    def open_song_editor(self):
        if len(self.select_rows) > 0:
            index = self.select_rows[0]
            SongEditor(self.get_songs(), index).show_all()
            
    def remove_songs(self, fully=False):
        if len(self.select_rows) > 0:
            songs = [ self.items[self.select_rows[index]].get_song() for index in range(0, len(self.select_rows))]
            MediaDB.remove(songs)    
            if fully:
                [ utils.move_to_trash(song.get("uri")) for song in songs ]
            self.delete_select_items()            
        return True    

    def popup_right_menu(self, widget, x, y, item, select_items):
        menu_items = [
            (None, "播放", self.play_song),
            (None, "添加到播放列表", self.emit_to_playlist),
            None,
            (None, "从歌曲库删除", self.remove_songs),
            (None, "从磁盘删除", self.remove_songs, True),
            None,
            (None, "打开歌曲所在目录", self.open_song_dir),
            (None, "属性", self.open_song_editor)
                        ]
            
        Menu(menu_items, True).show((int(x), int(y)))
