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
from dtk.ui.paned import HPaned
from dtk.ui.entry import TextEntry
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.menu import Menu
from dtk.ui.editable_list import EditableList

from library import MediaDB, Playlist
from widget.ui import app_theme
from widget.song_item import SongItem
from widget.list_item import PlaylistItem
from widget.dialog import WindowLoadPlaylist, WindowExportPlaylist, WinDir
from config import config
from player import Player
import utils

class PlaylistUI(gtk.VBox):
    '''Playlist UI.'''
	
    def __init__(self):
        '''Init.'''
        super(PlaylistUI, self).__init__()
        self.list_paned = HPaned(80)
        self.category_list = EditableList(background_pixbuf=app_theme.get_pixbuf("skin/main.png"))
        self.category_list.connect("active", self.category_button_press)
        self.category_list.connect("right-press", self.category_right_press)
        self.search_time_source = 0
        
        entry_button = ImageButton(
            app_theme.get_pixbuf("entry/search_normal.png"),
            app_theme.get_pixbuf("entry/search_hover.png"),
            app_theme.get_pixbuf("entry/search_press.png")
            )
        
        self.entry_box = TextEntry("", entry_button)
        self.entry_box.entry.connect("changed", self.search_cb)
        self.entry_box.set_size(300, 25)
        self.entry_box.set_no_show_all(True)
        entry_align = gtk.Alignment()
        entry_align.set_padding(0, 4, 5, 5)
        entry_align.add(self.entry_box)
        
        paned_align = gtk.Alignment()
        paned_align.set_padding(2, 4, 5, 5)
        paned_align.set(0, 0, 1, 1)
        paned_align.add(self.list_paned)
        
        self.toolbar_box = gtk.HBox(spacing=55)
        self.search_button = self.__create_simple_toggle_button("search", self.show_text_entry)
        self.__create_simple_button("list", self.popup_list_menu)
        self.__create_simple_button("add", self.popup_add_menu)
        self.__create_simple_button("sort", self.popup_sort_menu)
        self.__create_simple_button("delete", self.popup_delete_menu)
        toolbar_align = gtk.Alignment()
        toolbar_align.set_padding(2, 4, 10, 5)
        toolbar_align.add(self.toolbar_box)
                
        category_scrolled_window = ScrolledWindow(app_theme.get_pixbuf("skin/main.png"))
        category_scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        
        category_scrolled_window.add_child(self.category_list)
        self.right_box = gtk.VBox()
        self.list_paned.pack1(category_scrolled_window)
        self.list_paned.pack2(self.right_box)
        self.pack_start(paned_align, True, True)            
        self.pack_start(entry_align, False, False)            
        self.pack_start(toolbar_align, False, False)            
        
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
            
            
    def __on_db_loaded(self, db):        
        if not MediaDB.get_playlists():
            MediaDB.create_playlist("local", "[默认列表]")            
            
        init_items = [PlaylistItem(pl) for pl in MediaDB.get_playlists()]    
        self.category_list.add_items(init_items)
        self.current_item = self.category_list.items[self.get_save_item_index()]
        self.delete_source_id = self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.drag_source_id = self.current_item.song_view.connect("drag-data-received", self.parser_drag_event)
        self.menu_source_id = self.current_item.song_view.connect("right-press-items", self.popup_detail_menu)
        
        if self.current_item in self.category_list.items:
            index = self.category_list.items.index(self.current_item)
        else:    
            index = 0
        self.category_list.highlight_item(self.category_list.items[index])
        Player.set_source(self.current_item.song_view)
        self.right_box.pack_start(self.current_item.get_list_widget(), True, True)
        self.list_paned.show_all()
        
    def __on_player_loaded(self, player):   
        self.current_item.song_view.set_highlight_song(Player.song)
            
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
        
    def category_right_press(self, widget, item, x, y):    
        new_event = namedtuple("event", "x_root y_root")
        event = new_event(int(x), int(y))
        self.popup_list_menu(widget, event)
        
    def popup_list_menu(self, widget, event):    
        menu_items = [(None, "新建列表", self.new_list),
                      (None, "导入列表", self.leading_in_list),
                      (None, "打开列表", self.add_to_list),
                      (None, "导出列表", self.leading_out_list),
                      (None, "删除列表", self.delete_current_list),
                      None,
                      (None, "保存所有列表", self.save_all_list)]
        Menu(menu_items).show((int(event.x_root), int(event.y_root)))
        
    def new_list(self, items=[]):    
        index = len(self.category_list.items)
        self.category_list.new_item(PlaylistItem(Playlist("local", "%s%d" %  ("新建列表", index), items)))
        
        
    def get_edit_sub_menu(self, select_items, move=False):    
        sub_menu_items = []
        if len(self.category_list.items) > 1:
            other_obj = OrderedDict({index: item.get_name() for index, item in enumerate(self.category_list.items) if index != self.get_current_item_index()})
            sub_menu_items = [(None, value, self.edit_list_item, key, select_items ,move) for key, value in other_obj.items()]
        sub_menu_items.extend([None, (app_theme.get_pixbuf("toolbar/add_normal.png"), "新建列表", self.edit_new_list_item, select_items, move)])
        return Menu(sub_menu_items)
    
    def edit_list_item(self, index, select_items, move):
        try:
            other_item = self.category_list.items[index]
            other_item.song_view.add_items(select_items)
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
            self.category_list.add_items([new_item])
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
        if len(self.category_list.items) == 1:
            return
        index = self.get_current_item_index()
        self.category_list.delete_item(self.current_item)
        max_index = len(self.category_list.items) - 1
        if index <= max_index: 
            new_item = self.category_list.items[index]
        else:    
            new_item = self.category_list.items[index - 1]
        self.reset_highlight_item(new_item)
        
    def save_all_list(self):    
        uri = WinDir().run()
        if uri:
            try:
                save_name_dict = {}
                dir_name = utils.get_path_from_uri(uri)
                for item in self.category_list.items:
                    item_name = item.get_name()
                    save_name_dict[item_name] = save_name_dict.get(item_name, -1) + 1
                    if save_name_dict.get(item_name) > 0:
                        filename = "%s%d.%s" % (os.path.join(dir_name, item_name), save_name_dict.get(item_name), "m3u")
                    else:    
                        filename = "%s.%s" % (os.path.join(dir_name, item_name), "m3u")
                    utils.export_playlist(item.get_songs(), filename, "m3u")
            except:        
                pass
        
    def reset_highlight_item(self, item):    
        self.category_button_press(None, item)
        self.category_list.highlight_item(item)        
        
    def get_current_item_index(self):    
        try:
            index = self.category_list.items.index(self.current_item)
        except: index = 0    
        return index
    
    def popup_sort_menu(self, widget, event):
        self.current_item.song_view.get_playmode_menu([int(event.x_root), int(event.y_root)])
    
    def popup_delete_menu(self, widget, event):
        self.current_item.song_view.popup_delete_menu(int(event.x_root), int(event.y_root))
        
    def get_save_item_index(self):    
        index = config.getint("playlist", "current_index")
        if index <= len(self.category_list.items) - 1:
            return index
        return 0
        
    def category_button_press(self, widget, item):        
        self.reset_search_entry()
        if self.current_item == item:
            return 
        if self.drag_source_id != None or self.delete_source_id != None or self.menu_source_id !=None:
            gobject.source_remove(self.drag_source_id)
            gobject.source_remove(self.delete_source_id)
            gobject.source_remove(self.menu_source_id)

        self.current_item = item
        self.delete_source_id = self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.drag_source_id = self.current_item.song_view.connect("drag-data-received", self.parser_drag_event)
        self.menu_source_id = self.current_item.song_view.connect("right-press-items", self.popup_detail_menu)

        utils.container_remove_all(self.right_box)
        self.right_box.pack_start(item.get_list_widget(), True, True)
        self.list_paned.show_all()
        
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
            self.detail_menu.menu_window.destroy()
        play_mode_menu = self.current_item.song_view.get_playmode_menu(align=True)
        sort_dict = OrderedDict()
        sort_dict["sort_title"] = "按歌曲名"
        sort_dict["sort_artist"] = "按艺术家"        
        sort_dict["sort_album"] = "按专辑" 
        sort_dict["sort_genre"] = "按流派"
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
                                 (None, "编辑歌曲信息", None),
                                 ], opacity=1.0, menu_pos=1)
        self.detail_menu.show((int(x), int(y)))
        
        
    def save_to_library(self):    
        if self.search_flag:
            self.reset_search_entry()
            
        try:    
            index = self.category_list.items.index(self.current_item)
        except:    
            index = 0
            
        config.set("playlist","current_index", str(index))
                  
        MediaDB.full_erase_playlists()
        for item in self.category_list.items:
            songs = item.get_songs()
            name = item.get_name()
            MediaDB.create_playlist("local", name, songs)
