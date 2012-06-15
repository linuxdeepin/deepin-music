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
from collections import OrderedDict, namedtuple
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.menu import Menu
from dtk.ui.draw import draw_vlinear
from dtk.ui.utils import alpha_color_hex_to_cairo
from dtk.ui.treeview import TreeView
from dtk.ui.dialog import InputDialog

from library import MediaDB, Playlist
from helper import Dispatcher
from widget.skin import app_theme
from widget.ui import SearchEntry
from widget.song_item import SongItem
from widget.list_item import PlaylistItem
from widget.dialog import WindowLoadPlaylist, WindowExportPlaylist, WinDir
from widget.ui_utils import container_remove_all
from config import config
from player import Player
import utils


class PlaylistUI(gtk.VBox):
    '''Playlist UI.'''
	
    def __init__(self):
        '''Init.'''
        super(PlaylistUI, self).__init__()

        self.category_list = TreeView(font_size=9, height=26, font_x_padding=15)
        self.category_list.draw_mask = self.draw_single_mask
        self.category_list.connect("single-click-item", self.category_single_click_cb)
        self.category_list.connect("right-press-item", self.category_right_press_cb)
        self.category_list.connect("button-press-event", self.category_button_press_cb)
        self.category_list.set_size_request(98, -1)
        self.search_time_source = 0
        
        self.entry_box = SearchEntry("")
        self.entry_box.entry.connect("changed", self.search_cb)
        self.entry_box.set_no_show_all(True)
        entry_align = gtk.Alignment()
        entry_align.set(0, 0, 1, 1)
        entry_align.set_padding(2, 0, 10, 10)
        entry_align.add(self.entry_box)
        entry_align.connect("expose-event", self.expose_entry_mask)
        
        self.toolbar_box = gtk.HBox(spacing=50)
        self.search_button = self.__create_simple_toggle_button("search", self.show_text_entry)
        self.__create_simple_button("list", self.popup_list_menu)
        self.__create_simple_button("add", self.popup_add_menu)
        self.__create_simple_button("sort", self.popup_sort_menu)
        self.__create_simple_button("delete", self.popup_delete_menu)
        
        toolbar_align = gtk.Alignment()
        toolbar_align.set_padding(6, 6, 24, 24)
        toolbar_align.add(self.toolbar_box)
        toolbar_align.connect("expose-event", self.expose_toolbar_mask)
                
        category_scrolled_window = ScrolledWindow()
        category_scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        category_scrolled_window.add_child(self.category_list)
        
        self.right_box = gtk.VBox()
        self.list_box = gtk.HBox()
        self.list_box.pack_start(category_scrolled_window, False, False)
        self.list_box.pack_start(self.right_box, True, True)
        
        list_align = gtk.Alignment()
        list_align.set_padding(0, 0, 2, 0)
        list_align.set(0, 0, 1, 1)
        list_align.add(self.list_box)
        
        bottom_box = gtk.VBox()
        bottom_box.set_size_request(-1, 22)
        
        self.pack_start(list_align, True, True)            
        self.pack_start(entry_align, False, False)            
        self.pack_start(toolbar_align, False, True)            
        
        # Current
        self.current_playlist = None
        self.current_item = None
        self.search_flag = False
        self.cache_items = None
        self.delete_source_id = None
        self.drag_source_id = None
        self.menu_source_id = None
        self.detail_menu = None
        
        if MediaDB.isloaded():
            self.__on_db_loaded(MediaDB)
        else:    
            MediaDB.connect("loaded", self.__on_db_loaded)
            
        Player.connect("loaded", self.__on_player_loaded)    
        Dispatcher.connect("play-song", self.__play_and_add)
        Dispatcher.connect("add-songs", self.__add_songs_to_list)
        
    def expose_toolbar_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x + 2, rect.y, rect.width - 2, rect.height,
                     app_theme.get_shadow_color("playlistToolbar").get_color_info()
                     )
        return False
    
    def expose_entry_mask(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        color_info = app_theme.get_alpha_color("toolbarEntry").get_color_info()
        cr.set_source_rgba(*alpha_color_hex_to_cairo(color_info))
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
    def draw_single_mask(self, cr, x, y, width, height):
        color_info = app_theme.get_alpha_color("playlistLeft").get_color_info()
        cr.set_source_rgba(*alpha_color_hex_to_cairo(color_info))
        cr.rectangle(x, y, width, height)
        cr.fill()
        
    def draw_item_mask(self, cr, x, y, width, height):    
        draw_vlinear(cr, x, y, width, height,
                     app_theme.get_shadow_color("editlistItemPress").get_color_info())        
        
    def __on_db_loaded(self, db):        
        if not MediaDB.get_playlists():
            MediaDB.create_playlist("local", "[默认列表]")            
            
        # From MediaDB loaded playlists.    
        init_items = [PlaylistItem(pl) for pl in MediaDB.get_playlists()]    
        self.category_list.add_items(None, init_items)
        
        # Init Category_list.
        self.category_list.set_highlight_index(self.get_save_item_index())
        self.current_item = self.category_list.get_highlight_item()
        
        self.delete_source_id = self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.drag_source_id = self.current_item.song_view.connect("drag-data-received", self.parser_drag_event)
        self.menu_source_id = self.current_item.song_view.connect("right-press-items", self.popup_detail_menu)

        Player.set_source(self.current_item.song_view)
        self.right_box.pack_start(self.current_item.get_list_widget(), True, True)
        self.list_box.show_all()
        
    def __on_player_loaded(self, player):   
        if self.current_item:
            self.current_item.song_view.set_highlight_song(Player.song)
        
    def __play_and_add(self, widget, song):    
        self.current_item.song_view.add_songs(song, play=True)
        
    def __add_songs_to_list(self, widget, songs):
        if songs and self.current_item.song_view:
            self.current_item.song_view.add_songs(songs)
        
    def get_selected_song_view(self):    
        return self.current_item.song_view
            
    def search_cb(self, widget, text):        
        if not self.search_flag:
            self.cache_items = self.current_item.song_view.items[:]
        
        # Clear song_view select status    
        self.current_item.song_view.clear_highlight()
        self.current_item.song_view.select_rows = []
        
        if text != "":
            self.search_flag = True
            
            results = filter(lambda item: text.lower().replace(" ", "") in item.get_song().get("search", ""), self.cache_items)
            self.current_item.song_view.items = results
            self.current_item.song_view.update_item_index()
            self.current_item.song_view.update_vadjustment()        
            
        else:    
            self.search_flag = False
            self.current_item.song_view.items = self.cache_items
            if Player.song:
                played_item = SongItem(Player.song)
                if played_item in self.current_item.song_view.items:
                    index = self.current_item.song_view.items.index(played_item)
                    self.current_item.song_view.set_highlight(self.current_item.song_view.items[index])
            self.current_item.song_view.update_item_index()
            self.current_item.song_view.update_vadjustment()        
            if self.current_item.song_view.highlight_item != None:
                self.current_item.song_view.visible_highlight()
                
        self.current_item.song_view.queue_draw()
        
    def parser_delete_items(self, widget, items):    
        if self.search_flag:
            if self.cache_items != None:
                [self.cache_items.remove(item) for item in items if item in self.cache_items]
        
    def parser_drag_event(self, widget, context, x, y, selection, info, timestamp):
        if self.search_flag:
            self.reset_search_entry()
            
    def reset_search_entry(self):        
        self.search_button.set_active(False)
            
    def __create_simple_toggle_button(self, name, callback):        
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("toolbar/%s_normal.png" % name),
            app_theme.get_pixbuf("toolbar/%s_press.png" % name),
            )
        toggle_button.connect("toggled", callback)
        self.toolbar_box.pack_start(toggle_button, False, False)
        return toggle_button
            
    def __create_simple_button(self, name, callback):        
        button = ImageButton(
            app_theme.get_pixbuf("toolbar/%s_normal.png" % name),
            app_theme.get_pixbuf("toolbar/%s_hover.png" % name),
            app_theme.get_pixbuf("toolbar/%s_press.png" % name),
            )
        button.connect("button-press-event", callback)
        self.toolbar_box.pack_start(button, False, False)
        return button
                                                        
    def popup_add_menu(self, widget, event):
        self.current_item.song_view.popup_add_menu(int(event.x_root), int(event.y_root))
        
        
    def popup_list_menu(self, widget, event):    
        menu_items = [(None, "新建列表", self.new_list),
                      (None, "导入列表", self.leading_in_list),
                      (None, "打开列表", self.add_to_list),
                      (None, "导出列表", self.leading_out_list),
                      (None, "删除列表", self.delete_current_list),
                      None,
                      (None, "保存所有列表", self.save_all_list)]
        Menu(menu_items, True).show((int(event.x_root), int(event.y_root)))
        
    def new_list(self, items=[]):    
        index = len(self.category_list.get_items(None))
        name = "%s%d" % ("新建列表", index)
        input_dialog = InputDialog("新建列表", name, 300, 100, lambda name : self.create_new_playlist(name, items))
        input_dialog.show_all()
        
    def create_new_playlist(self, name, items):    
        self.category_list.add_items(None, [PlaylistItem(Playlist("local", name, items))])
        
    def get_edit_sub_menu(self, select_items, move=False):    
        sub_menu_items = []
        if len(self.category_list.get_items(None)) > 1:
            other_category_items = self.category_list.get_other_item(self.category_list.get_highlight_index())
            sub_menu_items = [(None, category_item.get_title(), self.edit_list_item, category_item, select_items ,move) for category_item in other_category_items]
        sub_menu_items.extend([None, (app_theme.get_pixbuf("toolbar/add_normal.png"), "新建列表", self.edit_new_list_item, select_items, move)])
        return Menu(sub_menu_items)
    
    def edit_list_item(self, category_item, select_items, move):
        try:
            category_item.song_view.add_items(select_items)
            if move:
                self.current_item.song_view.remove_select_items()
        except:        
            pass
        
    def edit_new_list_item(self, select_items, move):    
        self.new_list([item.get_song().get("uri") for item in select_items])
        if move:
            self.current_item.song_view.remove_select_items()
        
    def leading_in_list(self):    
        uri = WindowLoadPlaylist().run()
        try:
            p_name = utils.get_filename(uri)
            pl = MediaDB.create_playlist("local", p_name, [])
            new_item = PlaylistItem(pl)
            self.category_list.add_items(None, [new_item])
            new_item.song_view.async_add_uris(uri)
        except:    
            pass
        
    def leading_out_list(self):    
        if self.current_item:
            WindowExportPlaylist(self.current_item.get_songs()).run()
        
    def add_to_list(self):    
        uri = WindowLoadPlaylist().run()
        if uri:
            try:
                self.current_item.song_view.async_add_uris(uri)
            except: pass    
    def delete_current_list(self):
        if len(self.category_list.get_items(None)) == 1:
            return
        index = self.get_current_item_index()
        
        self.category_list.del_item_from_index(index)
        
        max_index = len(self.category_list.get_items(None)) - 1
        if index <= max_index: 
            new_index = index
        else:    
            new_index = index- 1
        self.reset_highlight_item(new_index)    
        
    def save_all_list(self):    
        uri = WinDir().run()
        if uri:
            try:
                save_name_dict = {}
                dir_name = utils.get_path_from_uri(uri)
                for item in self.category_list.get_items(None):
                    item_name = item.get_title()
                    save_name_dict[item_name] = save_name_dict.get(item_name, -1) + 1
                    if save_name_dict.get(item_name) > 0:
                        filename = "%s%d.%s" % (os.path.join(dir_name, item_name), save_name_dict.get(item_name), "m3u")
                    else:    
                        filename = "%s.%s" % (os.path.join(dir_name, item_name), "m3u")
                    utils.export_playlist(item.get_songs(), filename, "m3u")
            except:        
                pass
        
    def reset_highlight_item(self, index):    
        self.category_list.set_highlight_index(index)
        self.category_single_click_cb(None, self.category_list.get_item_from_index(index))
        
    def get_current_item_index(self):    
        index = self.category_list.get_highlight_index()
        if index is None:
            return 0
        else:
            return index
    
    def popup_sort_menu(self, widget, event):
        self.current_item.song_view.get_playmode_menu([int(event.x_root), int(event.y_root)])
    
    def popup_delete_menu(self, widget, event):
        self.current_item.song_view.popup_delete_menu(int(event.x_root), int(event.y_root))
        
    def get_save_item_index(self):    
        index = config.getint("playlist", "current_index")
        if index <= len(self.category_list.get_items(None)) - 1:
            return index
        return 0
    
    def category_right_press_cb(self, widget, item, x, y):    
        # new_event = namedtuple("event", "x_root y_root")
        # event = new_event(int(x), int(y))
        # self.popup_list_menu(widget, event)
        pass
        
    def category_button_press_cb(self, widget, event):    
        if event.button == 3:
            self.popup_list_menu(widget, event)
        
    def category_single_click_cb(self, widget, item):        
        self.reset_search_entry()
        if self.drag_source_id != None or self.delete_source_id != None or self.menu_source_id !=None:
            gobject.source_remove(self.drag_source_id)
            gobject.source_remove(self.delete_source_id)
            gobject.source_remove(self.menu_source_id)

        self.current_item = item
        self.delete_source_id = self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.drag_source_id = self.current_item.song_view.connect("drag-data-received", self.parser_drag_event)
        self.menu_source_id = self.current_item.song_view.connect("right-press-items", self.popup_detail_menu)

        container_remove_all(self.right_box)
        self.right_box.pack_start(item.get_list_widget(), True, True)
        self.list_box.show_all()
        
    def show_text_entry(self, widget):        
        if widget.get_active():
            self.entry_box.set_no_show_all(False)
            self.entry_box.show_all()
            self.entry_box.focus_input()
        else:    
            self.entry_box.entry.set_text("")
            self.entry_box.hide_all()
            self.entry_box.set_no_show_all(True)            
            
    def popup_detail_menu(self, widget, x, y, item, select_items):        
        if self.detail_menu != None:
            self.detail_menu.destroy()
        play_mode_menu = self.current_item.song_view.get_playmode_menu(align=True)
        sort_dict = OrderedDict()
        sort_dict["title"] = "按歌曲名"
        sort_dict["artist"] = "按艺术家"        
        sort_dict["album"] = "按专辑" 
        sort_dict["genre"] = "按流派"
        sort_dict["#playcount"] = "按播放次数"
        sort_dict["#added"] = "按添加时间"
        sort_items = [(None, value, self.current_item.song_view.set_sort_keyword, key) for key, value in sort_dict.iteritems()]
        sort_items.append(None)
        sort_items.append((None, "随机排序", self.current_item.song_view.random_reorder))
        sub_sort_menu = Menu(sort_items)
        add_to_list_menu = self.get_edit_sub_menu(select_items)
        move_to_list_menu = self.get_edit_sub_menu(select_items, True)
        self.detail_menu = Menu([(app_theme.get_pixbuf("playlist/play_song.png"), "播放歌曲",  self.current_item.song_view.play_select_item),
                                 (None, "添加到列表", add_to_list_menu),
                                 (None, "移动到列表", move_to_list_menu),
                                 (None, "发送到移动盘", None),
                                 None,
                                 (None, "删除", self.current_item.song_view.remove_select_items),
                                 (app_theme.get_pixbuf("playlist/delete_song.png"), "从本地删除", self.current_item.song_view.move_to_trash),
                                 (None, "清空列表", self.current_item.song_view.erase_items),
                                 None,
                                 (None, "播放模式", play_mode_menu),
                                 (None, "歌曲排序", sub_sort_menu),
                                 (app_theme.get_pixbuf("playlist/open_dir.png"), "打开文件目录", self.current_item.song_view.open_song_dir),
                                 (None, "编辑歌曲信息", self.current_item.song_view.open_song_editor),
                                 ], True)
        self.detail_menu.show((int(x), int(y)))
        
    def save_to_library(self):    
        if self.search_flag:
            self.reset_search_entry()
            
        index = self.get_current_item_index()        
        config.set("playlist","current_index", str(index))
                  
        MediaDB.full_erase_playlists()
        for item in self.category_list.get_items(None):
            songs = item.get_songs()
            name = item.get_title()
            MediaDB.create_playlist("local", name, songs)
