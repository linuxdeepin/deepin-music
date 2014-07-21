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
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.menu import Menu
from dtk.ui.draw import draw_vlinear
from dtk.ui.treeview import TreeView
from dtk.ui.dialog import InputDialog
from dtk.ui.paned import HPaned
import dtk.ui.tooltip as Tooltip

from library import MediaDB, Playlist, CDPlaylist
from helper import Dispatcher
from widget.skin import app_theme
from widget.ui import SearchEntry, PlaymodeButton
from widget.ui_utils import draw_line
from widget.list_item import ListTreeItem
from widget.dialog import WindowLoadPlaylist, WindowExportPlaylist, WinDir
from widget.ui_utils import container_remove_all, draw_alpha_mask
from config import config
from player import Player
from nls import _
from constant import CATEGROYLIST_WIDTH, HIDE_PLAYLIST_WIDTH
import utils

class CategoryView(TreeView):
    
    def add_items(self, items, insert_pos=None, clear_first=False):
        for item in items:
            song_view = getattr(item, "song_view", None)
            if song_view:
                setattr(song_view, "category_view", self)
        TreeView.add_items(self, items, insert_pos, clear_first)        
        
    items = property(lambda self: self.visible_items)    
    
    def check_view_status(self):
        for item in self.items:
            item.song_view.async_reset_error_items()

class PlaylistUI(gtk.VBox):
    '''Playlist UI.'''
	
    def __init__(self):
        '''Init.'''
        gtk.VBox.__init__(self)

        # Init catagory list.
        self.category_list = CategoryView()
        setattr(self.category_list, "save_to_library", self.save_to_library)        
        self.category_list.draw_mask = self.draw_category_list_mask
        self.category_list.connect("single-click-item", self.on_category_single_click)
        self.category_list.connect("right-press-items", self.on_category_right_press)
        self.category_list.set_size_request(CATEGROYLIST_WIDTH, -1)

        del self.category_list.keymap["Delete"]
        
        # Init SearchEntry.
        self.entry_box = SearchEntry("")
        self.entry_box.entry.connect("changed", self.search_cb)
        self.entry_box.set_no_show_all(True)
        entry_align = gtk.Alignment()
        entry_align.set(0, 0, 1, 1)
        entry_align.set_padding(2, 0, 10, 10)
        entry_align.add(self.entry_box)
        entry_align.connect("expose-event", self.expose_entry_mask)
        
        # Init toolbar.
        self.toolbar_box = gtk.HBox(spacing=45)
        self.search_button = self.__create_simple_toggle_button("search", self.show_text_entry, 
                                                                _("Search in Current Playlist"))
        
        self.__create_simple_button("list", self.popup_list_menu, _("Playlist Operations"))
        self.__create_simple_button("add", self.popup_add_menu, _("Add"))
        
        self.playmode_button = PlaymodeButton(config.get("setting", "loop_mode", "list_mode").split("_")[0])
        Tooltip.text(self.playmode_button, _("Playback mode"))
        self.playmode_button.connect("button-press-event", self.popup_sort_menu)
        self.toolbar_box.pack_start(self.playmode_button, False, False)
        self.__create_simple_button("delete", self.popup_delete_menu, _("Delete"))
        toolbar_align = gtk.Alignment()
        toolbar_align.set_padding(6, 6, 28, 0)
        toolbar_align.add(self.toolbar_box)
        toolbar_align.connect("expose-event", self.expose_toolbar_mask)
                
        self.right_box = gtk.VBox()
        self.right_box.connect("size-allocate", self.on_right_box_size_allocate)
        self.list_paned = HPaned(handle_color=app_theme.get_color("panedHandler"), enable_drag=True)
        self.list_paned.pack1(self.category_list, True, True)
        self.list_paned.pack2(self.right_box, True, False)
        bottom_box = gtk.VBox()
        bottom_box.set_size_request(-1, 22)
        self.pack_start(self.list_paned, True, True)            
        self.pack_start(entry_align, False, False)            
        self.pack_start(toolbar_align, False, True)            
        
        # Current
        self.current_playlist = None
        self.search_time_source = 0        
        self.current_item = None
        self.search_flag = False
        self.cache_items = None
        self.delete_source_id = None
        self.drag_source_id = None
        self.menu_source_id = None
        self.song_notify_id = None
        self.detail_menu = None
        self.save_flag = False
        
        if MediaDB.isloaded():
            self.__on_db_loaded(MediaDB)
        else:    
            MediaDB.connect("loaded", self.__on_db_loaded)
            
        Player.connect("loaded", self.__on_player_loaded)    
        Dispatcher.connect("play-song", self.__play_and_add)
        Dispatcher.connect("add-songs", self.__add_songs_to_list)
        Dispatcher.connect("new-cd-playlist", self.__new_audiocd_playlist)
        Dispatcher.connect("del-cd-playlist", self.delete_audiocd_list)
        Dispatcher.connect("save-current-list", self.save_current_playlist)
        config.connect("config-changed", self.on_config_changed)
        
    def on_config_changed(self, config, section, option, value):    
        if section == "setting" and option == "loop_mode":
            icon_name = value.split("_")[0]
            self.playmode_button.update_dpixbufs(icon_name, True)
        
    def on_right_box_size_allocate(self, widget, rect):    
        if self.current_item:
            if rect.width > HIDE_PLAYLIST_WIDTH:
                self.current_item.song_view.set_hide_columns(None)
            else:    
                self.current_item.song_view.set_hide_columns([1])
        
    def expose_toolbar_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        cr.set_source_rgba(1, 1, 1, 0.95)
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
        draw_line(cr, (rect.x, rect.y + 1), 
                  (rect.x + rect.width, rect.y + 1), "#b0b0b0")
        return False
    
    def expose_entry_mask(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x , rect.y, rect.width, rect.height, "toolbarEntry")
        
    def draw_category_list_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, "layoutLeft")
        
    def draw_item_mask(self, cr, x, y, width, height):    
        draw_vlinear(cr, x, y, width, height,
                     app_theme.get_shadow_color("editlistItemPress").get_color_info())        
        
    def __on_db_loaded(self, db):        
        if not MediaDB.get_playlists():
            MediaDB.create_playlist("local", _("Default List"))            
            
        # From MediaDB loaded playlists.    
        init_items = [ListTreeItem(pl) for pl in MediaDB.get_playlists()]    
        self.category_list.add_items(init_items)
        
        # Init Category_list.
        self.category_list.set_highlight_item(self.get_categroy_item_by_index(self.get_save_item_index()))
        self.current_item = self.category_list.get_highlight_item()
        
        self.delete_source_id = self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.drag_source_id = self.current_item.song_view.connect("drag-data-received", self.parser_drag_event)
        self.menu_source_id = self.current_item.song_view.connect("right-press-items", self.popup_detail_menu)

        Player.set_source(self.current_item.song_view)
        self.right_box.add(self.current_item.get_list_widget())
        
        self.category_list.check_view_status()
        self.list_paned.show_all()
        
    def __on_player_loaded(self, player):   
        if self.current_item:
            self.current_item.song_view.reset_error_items()
            self.current_item.song_view.set_highlight_song(Player.song)
            
    def restore_status(self):        
        uri = config.get("player", "uri")
        seek = int(config.get("player", "seek"))
        state = config.get("player", "state")
        play = False
        
        if config.getboolean("player", "play_on_startup") and state == "playing":
            play = True
            
        if uri and self.current_item:    
            song = MediaDB.get_song(uri)
            if song.get_type() == "cue":
                seek = seek + song.get("seek", 0)
                
            if song and song.exists():
                if not config.getboolean("player", "resume_last_progress") or not play:
                    if song.get_type() == "cue":
                        seek = song.get("seek", 0)
                    else:    
                        seek = None
                        
                self.current_item.song_view.play_song(song, play, seek)
        
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
            self.cache_items = self.current_item.song_view.get_items()
        
        # Clear song_view select status    
        self.current_item.song_view.clear_highlight()
        self.current_item.song_view.select_rows = []
        
        if text != "":
            self.search_flag = True
            results = filter(lambda item: text.lower().replace(" ", "") in item.get_song().get("search", ""), self.cache_items)
            self.current_item.song_view.set_song_items(results)
        else:    
            self.search_flag = False
            self.current_item.song_view.set_song_items(self.cache_items)
            
            if Player.song:
                self.current_item.song_view.set_highlight_song(Player.song)
        
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
        menu_items = [(None, _("New List"), self.new_list),
                      (None, _("Import List"), self.leading_in_list),
                      (None, _("Open List"), self.add_to_list),
                      (None, _("Export List"), self.leading_out_list),
                      (None, _("Remove List"), self.delete_item_list),
                      None,
                      (None, _("Save all Lists"), self.save_all_list)]
        Menu(menu_items, True).show((int(event.x_root), int(event.y_root)))
        
    def new_list(self, items=[], name=None):    
        index = len(self.category_list.get_items())
        if name is None:
            name = "%s%d" % (_("New List"), index)
        input_dialog = InputDialog(_("New List"), name, 300, 100, lambda name : self.create_new_playlist(name, items))
        input_dialog.show_all()
        
    def create_new_playlist(self, name, items):    
        self.category_list.add_items([ListTreeItem(Playlist("local", name, items))])
        
    def __new_audiocd_playlist(self, obj, name, songs, udi):
        self.category_list.add_items([ListTreeItem(CDPlaylist("audiocd", name, songs), udi=udi)]) 
        
    def get_categroy_other_items(self):    
        other_items = []
        highlight_item = self.category_list.get_highlight_item()
        for item in self.category_list.get_items():
            if highlight_item == item:
                continue
            other_items.append(item)
        return other_items    
    
    def get_categroy_index_by_item(self, item):
        index = -1
        for each_index, each_item in enumerate(self.category_list.get_items()):
            if item == each_item: 
                index = each_index
                break
        return index    
    
    def get_categroy_item_by_index(self, index):
        try:
            return self.category_list.get_items()[index]
        except:
            return None
        
    def get_edit_sub_menu(self, select_items, move=False):    
        sub_menu_items = []
        if len(self.category_list.get_items()) > 1:
            other_category_items = self.get_categroy_other_items()
            sub_menu_items = [(None, category_item.get_title(), 
                               self.edit_list_item, category_item, select_items ,move) for category_item in other_category_items]
        if sub_menu_items:    
            sub_menu_items.extend([None, ((app_theme.get_pixbuf("toolbar/add_normal.png"), None, None),
                                          _("New List"), self.edit_new_list_item, select_items, move)])
        else:    
            sub_menu_items.extend([((app_theme.get_pixbuf("toolbar/add_normal.png"), None, None),
                                    _("New List"), self.edit_new_list_item, select_items, move)])
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
            new_item = ListTreeItem(pl)
            self.category_list.add_items([new_item])
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
            
    def delete_audiocd_list(self, obj, udi):
        reset = False
        for item in self.category_list.get_items():
            if item.udi == udi:
                reset = True
                self.category_list.delete_items([item])
        if reset:        
            self.reset_highlight_item(self.category_list.get_items()[-1])    
            
    def delete_item_list(self, item):
        if len(self.category_list.get_items()) == 1:
            return
        
        index = self.get_categroy_index_by_item(item)
        self.category_list.delete_items([item])
        
        max_index = len(self.category_list.get_items()) - 1
        if index <= max_index: 
            new_index = index
        else:    
            new_index = index- 1
        self.reset_highlight_item(self.category_list.get_items()[new_index])    
        
    def save_all_list(self):    
        uri = WinDir().run()
        if uri:
            try:
                save_name_dict = {}
                dir_name = utils.get_path_from_uri(uri)
                for item in self.category_list.get_items():
                    item_name = item.get_title()
                    save_name_dict[item_name] = save_name_dict.get(item_name, -1) + 1
                    if save_name_dict.get(item_name) > 0:
                        filename = "%s%d.%s" % (os.path.join(dir_name, item_name), save_name_dict.get(item_name), "m3u")
                    else:    
                        filename = "%s.%s" % (os.path.join(dir_name, item_name), "m3u")
                    utils.export_playlist(item.get_songs(), filename, "m3u")
            except:        
                pass
        
    def reset_highlight_item(self, item):    
        self.category_list.set_highlight_item(item)
        self.on_category_single_click(None, item, None, None, None)
        
        
    def get_current_item_index(self):    
        item = self.category_list.get_highlight_item()
        index = self.get_categroy_index_by_item(item)
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
        if index <= len(self.category_list.get_items()) - 1:
            return index
        return 0
    
    def on_category_right_press(self, widget, x, y, item, column):    
        if not item:
            menu_items = [
                (None, _("New List"), self.new_list),
                (None, _("Import List"), self.leading_in_list),
                None,
                (None, _("Save all Lists"), self.save_all_list)
                ]
        else:    
            menu_items = [
                (None, _("Rename"), lambda : self.rename_item_list(item)),
                (None, _("Remove List"), lambda : self.delete_item_list(item)),
                (None, _("Open List"), lambda : self.add_to_list(item)),
                None,
                (None, _("Save all Lists"), self.save_all_list)
                ]
            
        Menu(menu_items, True).show((x, y))    
            
    def rename_item_list(self, item):        
        input_dialog = InputDialog(_("Rename"), item.get_title(), 300, 100,
                                   lambda name: item.set_title(name))    
        input_dialog.show_all()
        
    def on_category_button_press(self, widget, event):    
        if event.button == 3:
            self.popup_list_menu(widget, event)
        
    def on_category_single_click(self, widget, item, column, x, y):        
        self.reset_search_entry()
        if self.drag_source_id != None or self.delete_source_id != None or self.menu_source_id !=None:
            gobject.source_remove(self.drag_source_id)
            gobject.source_remove(self.delete_source_id)
            gobject.source_remove(self.menu_source_id)

        self.current_item = item
        self.category_list.set_highlight_item(item)
        
        self.delete_source_id = self.current_item.song_view.connect("delete-select-items", self.parser_delete_items)
        self.drag_source_id = self.current_item.song_view.connect("drag-data-received", self.parser_drag_event)
        self.menu_source_id = self.current_item.song_view.connect("right-press-items", self.popup_detail_menu)

        container_remove_all(self.right_box)
        self.right_box.add(item.get_list_widget())
        self.list_paned.show_all()
        
    def show_text_entry(self, widget):        
        if widget.get_active():
            self.entry_box.set_no_show_all(False)
            self.entry_box.show_all()
            self.entry_box.focus_input()
        else:    
            self.entry_box.hide_all()            
            self.entry_box.set_no_show_all(True)                        
            self.entry_box.entry.set_text("")
            
    def popup_detail_menu(self, widget, x, y, item, select_items):        
        if self.detail_menu != None:
            self.detail_menu.destroy()
        play_mode_menu = self.current_item.song_view.get_playmode_menu(align=True)
        sort_dict = OrderedDict()
        sort_dict["file"] = _("By Filename")        
        sort_dict["title"] = _("By Title")
        sort_dict["artist"] = _("By Artist")        
        sort_dict["album"] = _("By Album") 
        sort_dict["genre"] = _("By Genre")
        sort_dict["#track"] = _("By Track")
        sort_dict["#playcount"] = _("By Play Count")
        sort_dict["#added"] = _("By Added Time")

        sort_items = [(None, value, self.current_item.song_view.set_sort_keyword, key) for key, value in sort_dict.iteritems()]
        sort_items.append(None)
        sort_items.append((None, _("Randomize"), self.current_item.song_view.random_reorder))
        sub_sort_menu = Menu(sort_items)
        add_to_list_menu = self.get_edit_sub_menu(select_items)
        move_to_list_menu = self.get_edit_sub_menu(select_items, True)
        self.detail_menu = Menu([(None, _("Play"),  self.current_item.song_view.play_select_item),
                                 (None, _("Add to List"), add_to_list_menu),
                                 (None, _("Move to List"), move_to_list_menu),
                                 None,
                                 (None, _("Remove Track"), self.current_item.song_view.remove_select_items),
                                 (None, _("Move to Trash"), self.current_item.song_view.try_move_trash),
                                 (None, _("Clear List"), self.current_item.song_view.erase_items),
                                 None,
                                 (None, _("Playback mode"), play_mode_menu),
                                 (None, _("Sort"), sub_sort_menu),
                                 (None, _("Converter"), self.current_item.song_view.songs_convert),
                                 (None, _("Open directory"), self.current_item.song_view.open_song_dir),
                                 (None, _("Properties"), self.current_item.song_view.open_song_editor),
                                 ], True)
        
        if item and item.song.get_type() == "cue":
            self.detail_menu.set_menu_item_sensitive_by_index(5, False)
            self.detail_menu.set_menu_item_sensitive_by_index(10, False)
        self.detail_menu.show((int(x), int(y)))
        
        
    def save_current_playlist(self, *args):    
        index = 0
        player_source = Player.get_source()
        for i, item in enumerate(self.category_list.get_items()):
            if item.song_view == player_source:
                index = i
        config.set("playlist","current_index", str(index))
        
    def save_to_library(self):    
        if self.save_flag:
            return 
        
        self.save_flag = True
        if self.search_flag:
            self.reset_search_entry()
                  
        MediaDB.full_erase_playlists()
        for item in self.category_list.get_items():
            if item.udi is not None:
                continue
            songs = item.get_songs()
            name = item.get_title()
            MediaDB.create_playlist("local", name, songs)
        MediaDB.async_save()    
        self.save_flag = False
            
playlist_ui = PlaylistUI()            
