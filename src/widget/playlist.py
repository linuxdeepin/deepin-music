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
from collections import OrderedDict
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.menu import Menu
from dtk.ui.draw import draw_vlinear
from dtk.ui.treeview import TreeView
from dtk.ui.dialog import InputDialog
import dtk.ui.tooltip as Tooltip

from library import MediaDB, Playlist
from helper import Dispatcher
from widget.skin import app_theme
from widget.ui import SearchEntry
from widget.song_item import SongItem
from widget.list_item import PlaylistItem
from widget.dialog import WindowLoadPlaylist, WindowExportPlaylist, WinDir
from widget.ui_utils import container_remove_all, draw_alpha_mask
from config import config
from player import Player
from nls import _
import utils


class PlaylistUI(gtk.VBox):
    '''Playlist UI.'''
	
    def __init__(self):
        '''Init.'''
        super(PlaylistUI, self).__init__()

        self.category_list = TreeView(font_size=9, height=26, font_x_padding=15)
        self.category_list.draw_mask = self.draw_category_list_mask
        self.category_list.connect("single-click-item", self.category_single_click_cb)
        self.category_list.connect("right-press-item", self.category_right_press_cb)
        # self.category_list.connect("button-press-event", self.category_button_press_cb)
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
        self.search_button = self.__create_simple_toggle_button("search", self.show_text_entry, _("Search for tracks"))
        self.__create_simple_button("list", self.popup_list_menu, _("Playlist operations"))
        self.__create_simple_button("add", self.popup_add_menu, _("Add track"))
        self.__create_simple_button("sort", self.popup_sort_menu, _("Play mode"))
        self.__create_simple_button("delete", self.popup_delete_menu, _("Delete"))
        
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
        draw_alpha_mask(cr, rect.x + 2, rect.y, rect.width - 1, rect.height, "toolbarEntry")
        
    def draw_category_list_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, "layoutLeft")
        
    def draw_item_mask(self, cr, x, y, width, height):    
        draw_vlinear(cr, x, y, width, height,
                     app_theme.get_shadow_color("editlistItemPress").get_color_info())        
        
    def __on_db_loaded(self, db):        
        if not MediaDB.get_playlists():
            MediaDB.create_playlist("local", _("[Default list]"))            
            
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
            self.current_item.song_view.reset_error_items()
            self.current_item.song_view.set_highlight_song(Player.song)
        
    def __play_and_add(self, widget, song):    
        self.current_item.song_view.add_songs(song, play=True)
        
    def __add_songs_to_list(self, widget, songs):
        if songs and self.current_item.song_view:
            self.current_item.song_view.add_songs(songs)
        
    def get_selected_song_view(self):    
        if self.current_item:
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
            
    def __create_simple_toggle_button(self, name, callback, tip_msg=""):        
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("toolbar/%s_normal.png" % name),
            app_theme.get_pixbuf("toolbar/%s_press.png" % name),
            )
        toggle_button.connect("toggled", callback)
        
        if tip_msg:
            Tooltip.text(toggle_button, tip_msg)
            
        self.toolbar_box.pack_start(toggle_button, False, False)
        return toggle_button
            
    def __create_simple_button(self, name, callback, tip_msg=""):        
        button = ImageButton(
            app_theme.get_pixbuf("toolbar/%s_normal.png" % name),
            app_theme.get_pixbuf("toolbar/%s_hover.png" % name),
            app_theme.get_pixbuf("toolbar/%s_press.png" % name),
            )
        button.connect("button-press-event", callback)
        if tip_msg:
            Tooltip.text(button, tip_msg)
            
        self.toolbar_box.pack_start(button, False, False)
        return button
                                                        
    def popup_add_menu(self, widget, event):
        self.current_item.song_view.popup_add_menu(int(event.x_root), int(event.y_root))
        
        
    def popup_list_menu(self, widget, event):    
        menu_items = [(None, _("New list"), self.new_list),
                      (None, _("Import list"), self.leading_in_list),
                      (None, _("Open list"), self.add_to_list),
                      (None, _("Export list"), self.leading_out_list),
                      (None, _("Remove list"), self.delete_item_list),
                      None,
                      (None, _("Save all lists"), self.save_all_list)]
        Menu(menu_items, True).show((int(event.x_root), int(event.y_root)))
        
    def new_list(self, items=[]):    
        index = len(self.category_list.get_items(None))
        name = "%s%d" % (_("New list"), index)
        input_dialog = InputDialog(_("New list"), name, 300, 100, lambda name : self.create_new_playlist(name, items))
        input_dialog.show_all()
        
    def create_new_playlist(self, name, items):    
        self.category_list.add_items(None, [PlaylistItem(Playlist("local", name, items))])
        
    def get_edit_sub_menu(self, select_items, move=False):    
        sub_menu_items = []
        if len(self.category_list.get_items(None)) > 1:
            other_category_items = self.category_list.get_other_item(self.category_list.get_highlight_index())
            sub_menu_items = [(None, category_item.get_title(), 
                               self.edit_list_item, category_item, select_items ,move) for category_item in other_category_items]
        if sub_menu_items:    
            sub_menu_items.extend([None, ((app_theme.get_pixbuf("toolbar/add_normal.png"), None, None),
                                          _("New list"), self.edit_new_list_item, select_items, move)])
        else:    
            sub_menu_items.extend([((app_theme.get_pixbuf("toolbar/add_normal.png"), None, None),
                                    _("New list"), self.edit_new_list_item, select_items, move)])
        return Menu(sub_menu_items)
    
    def edit_list_item(self, category_item, select_items, move):
        try:
            category_item.song_view.add_items(select_items)
            category_item.song_view.update_item_index()
            category_item.song_view.update_vadjustment()        
            if move:
                self.current_item.song_view.remove_select_items()
        except:        
            pass
        else:
            self.current_item.song_view.update_item_index()
            self.current_item.song_view.update_vadjustment()
        
    def edit_new_list_item(self, select_items, move):    
        self.new_list([item.get_song().get("uri") for item in select_items])
        if move:
            self.current_item.song_view.remove_select_items()
        self.current_item.song_view.update_item_index()    
        self.current_item.song_view.update_vadjustment()
        
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
        
    def leading_out_list(self, item):    
        if not item:
            item = self.current_item
        WindowExportPlaylist(item.get_songs()).run()
        
    def add_to_list(self, item=None):    
        uri = WindowLoadPlaylist().run()
        if uri:
            try:
                if not item:
                    item = self.current_item
                item.song_view.async_add_uris(uri)
            except: pass    
    def delete_item_list(self, index=None):
        if len(self.category_list.get_items(None)) == 1:
            return
        
        if not index:
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
    
    def category_right_press_cb(self, widget, item, x, y, index):    
        # new_event = namedtuple("event", "x_root y_root")
        # event = new_event(int(x), int(y))
        # self.popup_list_menu(widget, event)
        if index == -1:
            menu_items = [
                (None, _("New list"), self.new_list),
                (None, _("Import list"), self.leading_in_list),
                None,
                (None, _("Save all lists"), self.save_all_list)
                ]
        else:    
            menu_items = [
                (None, _("Rename"), self.rename_item_list, item, index),
                (None, _("Remove list"), self.delete_item_list, index),
                (None, _("Open list"), self.add_to_list, item),
                None,
                (None, _("Save all lists"), self.save_all_list)
                ]
            
        Menu(menu_items, True).show((x, y))    
            
    def rename_item_list(self, item, index):        
        def rename_spec_list(name, spec_item, spec_index):
            self.category_list.set_index_text(spec_index, name)
            spec_item.set_title(name)
        input_dialog = InputDialog(_("Rename"), item.get_title(), 300, 100,
                                   lambda name: rename_spec_list(name, item, index))    
        input_dialog.show_all()
        
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
        sort_dict["title"] = _("by title")
        sort_dict["artist"] = _("by artist")        
        sort_dict["album"] = _("by album") 
        sort_dict["genre"] = _("by genre")
        sort_dict["#playcount"] = _("by playcount")
        sort_dict["#added"] = _("by added")
        sort_items = [(None, value, self.current_item.song_view.set_sort_keyword, key) for key, value in sort_dict.iteritems()]
        sort_items.append(None)
        sort_items.append((None, _("Random order"), self.current_item.song_view.random_reorder))
        sub_sort_menu = Menu(sort_items)
        add_to_list_menu = self.get_edit_sub_menu(select_items)
        move_to_list_menu = self.get_edit_sub_menu(select_items, True)
        self.detail_menu = Menu([(None, _("Play track"),  self.current_item.song_view.play_select_item),
                                 (None, _("Add to list"), add_to_list_menu),
                                 (None, _("move to list"), move_to_list_menu),
                                 None,
                                 (None, _("Remove track"), self.current_item.song_view.remove_select_items),
                                 (None, _("Move to trash"), self.current_item.song_view.move_to_trash),
                                 (None, _("Clear list"), self.current_item.song_view.erase_items),
                                 None,
                                 (None, _("Play mode"), play_mode_menu),
                                 (None, _("Sort tracks"), sub_sort_menu),
                                 (None, _("Format conversion"), self.current_item.song_view.songs_convert),
                                 (None, _("Open directory"), self.current_item.song_view.open_song_dir),
                                 (None, _("Property"), self.current_item.song_view.open_song_editor),
                                 ], True)
        
        if item.song.get_type() == "cue":
            self.detail_menu.set_menu_item_sensitive_by_index(5, False)
            self.detail_menu.set_menu_item_sensitive_by_index(10, False)
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
