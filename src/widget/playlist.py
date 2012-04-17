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
from collections import OrderedDict
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.paned import HPaned
from dtk.ui.listview import ListView
from dtk.ui.entry import TextEntry
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.menu import Menu
from dtk.ui.editable_list import EditableList

from library import MediaDB
from widget.ui import SongView, app_theme
from widget.song_item import SongItem
from widget.list_item import PlaylistItem
from widget.dialog import WindowLoadPlaylist, WindowExportPlaylist
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
        
        self.toolbar_box = gtk.HBox(spacing=60)
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
        
        self.new_entry = TextEntry("")
        self.new_entry.connect("focus-out-event", self.__focus_out_entry)
        self.new_entry.set_size(100, 25)
        self.new_entry.set_no_show_all(True)
        new_align = gtk.Alignment()
        new_align.set_padding(2, 2, 8, 8)
                
        self.category_vbox = gtk.VBox()
        self.category_vbox.pack_start(self.category_list, True, True)
        self.category_vbox.pack_start(self.new_entry, False, False)
        category_scrolled_window.add_child(self.category_vbox)
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
        
        if MediaDB.isloaded():
            self.__on_db_loaded(MediaDB)
        else:    
            MediaDB.connect("loaded", self.__on_db_loaded)
            
        Player.connect("loaded", self.__on_player_loaded)    
            
            
    def __on_db_loaded(self, db):        
        if not MediaDB.get_playlists():
            MediaDB.create_playlist("local", "[默认列表]")            
            MediaDB.create_playlist("local", "[流行歌曲]")            
            MediaDB.create_playlist("local", "[我的最爱]")
            
        init_items = [ PlaylistItem(pl) for pl in MediaDB.get_playlists()]
        self.category_list.add_items(init_items)
        self.current_item = self.category_list.items[self.get_current_index()]
        self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.current_item.song_view.connect("drag_data_received", self.parser_drag_event)
        if self.current_item in self.category_list.items:
            index = self.category_list.items.index(self.current_item)
        else:    
            index = 0
        # self.category_list.select_rows.append(index)    
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

    def __focus_out_entry(self, widget, event):
        '''focus out entry'''
        pass
        
    def popup_list_menu(self, widget, event):    
        menu_items = [(None, "新建列表", None),
                      (None, "导入列表", self.leading_in_list),
                      (None, "打开列表", self.add_to_list),
                      (None, "导出列表", self.leading_out_list),
                      (None, "删除列表", self.delete_current_list),
                      None,
                      (None, "保存所有列表", None)]
        Menu(menu_items).show((int(event.x_root), int(event.y_root)))
        
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
        pass
        
    
    def popup_sort_menu(self, widget, event):
        self.current_item.song_view.get_playmode_menu([int(event.x_root), int(event.y_root)])
    
    def popup_delete_menu(self, widget, event):
        self.current_item.song_view.popup_delete_menu(int(event.x_root), int(event.y_root))
        
    def get_current_index(self):    
        index = config.getint("playlist", "current_index")
        if index <= len(self.category_list.items) - 1:
            return index
        return 0
        
    def list_button_press(self, widget, item, column, x, y):        
        self.reset_search_entry()

        self.current_item = item
        self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.current_item.song_view.connect("drag_data_received", self.parser_drag_event)

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
