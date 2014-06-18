#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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
import os

import dtk.ui.tooltip as Tooltip
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.iconview import IconView
from dtk.ui.paned import HPaned
from dtk.ui.menu import Menu
from dtk.ui.dialog import ConfirmDialog
 
from library import MediaDB, DBQuery
from helper import SignalContainer, Dispatcher
from widget.skin import app_theme
from widget.ui import SearchEntry, BackButton
from widget.song_view import MultiDragSongView
from widget.ui_utils import (switch_tab, draw_alpha_mask, 
                             create_right_align, create_separator_box)
from widget.outlookbar import OptionBar, SongImportBar
from widget.local_item import LocalItem
from widget.dialog import WinFile

from source.local import ImportFolderJob, ReloadDBJob
from widget.combo import  PromptButton
from nls import _

# from widget.download_view import TaskView

import utils

class Browser(gtk.VBox, SignalContainer):        
    
    def __init__(self, db_query):
        
        # Init.
        gtk.VBox.__init__(self)
        SignalContainer.__init__(self)
        
        self.__db_query = db_query
        self.update_interval = 3000 # 3000 millisecond.
        self.reload_flag = False
        self.search_flag = False
        self.cache_search_list = []
        self.cache_search_view = []
        self.__selected_tag = {"album": [], "artist": [], "genre": []}
        Dispatcher.connect("reload-browser", self.on_dispatcher_reload_browser)
        gobject.timeout_add(self.update_interval, self.on_interval_loaded_view)
        
        # The saving song Classification presented to the user.
        self.artists_view, self.artists_sw  = self.get_icon_view(14)
        self.albums_view,  self.albums_sw   = self.get_icon_view(14)
        self.genres_view,  self.genres_sw   = self.get_icon_view(14)
        self.folders_view, self.folders_sw  = self.get_icon_view(20)
        self.folders_view.connect("motion-notify-item", self.on_folders_view_motion_notify)
        
        # Song list for temporarily storing
        # self.songs_view, self.songs_view_sw = self.get_song_view()
        self.songs_view = self.get_song_view()
        
        # Classification navigation bar.
        self.filterbar = OptionBar(
            [(app_theme.get_pixbuf("filter/artist_normal.png"), app_theme.get_pixbuf("filter/artist_press.png"),
              _("Artist"), lambda : self.switch_filter_view("artist")),
             (app_theme.get_pixbuf("filter/album_normal.png"), app_theme.get_pixbuf("filter/album_press.png"),
              _("Album"), lambda : self.switch_filter_view("album")),
             (app_theme.get_pixbuf("filter/genre_normal.png"), app_theme.get_pixbuf("filter/genre_press.png"),
              _("Genre"), lambda : self.switch_filter_view("genre")),
             (app_theme.get_pixbuf("filter/local_normal.png"), app_theme.get_pixbuf("filter/local_press.png"),
              _("Folder"), lambda : self.switch_filter_view("folder"))
             ])
        
        # Manage the media library (import, refresh)
        self.importbar = SongImportBar()
        self.importbar.reload_items(
            [
             (_("Quick Scan"), lambda : ImportFolderJob([os.path.expanduser("~")])),                
             (_("Scan"), lambda : ImportFolderJob()),
             (_("Refresh"), lambda : ReloadDBJob())
             ])
        
        # Left box
        invailed_box = gtk.VBox()
        invailed_box.set_size_request(-1, 10)
        left_vbox = gtk.VBox(spacing=10)
        left_vbox.set_size_request(121, -1)
        # left_vbox.pack_start(invailed_box, False, True)
        left_vbox.pack_start(self.filterbar, False, False)
        left_vbox.pack_start(create_separator_box(), False, False)
        left_vbox.pack_start(self.importbar, False, False)
        
        # Used to switch songs category view, in the right side of the layout.
        self.switch_view_box = gtk.VBox()
        self.switch_view_box.add(self.artists_sw)
        # self.switch_view_box.add(TaskView())
        
        # Control back on a view.
        self.back_hbox = gtk.HBox()
        self.back_hbox.set_size_request(-1, 26)
        self.back_hbox.set_no_show_all(True)
        self.back_button = BackButton()
        self.back_button.connect("clicked", self.on_back_button_clicked)
        self.prompt_button = PromptButton()
        self.back_hbox.pack_start(self.back_button, False, False, 5)
        self.back_hbox.pack_start(self.prompt_button, False, False)
        
        # searchbar 
        self.search_entry, search_align = self.get_search_entry()
        search_hbox = gtk.HBox()
        search_hbox.pack_start(self.back_hbox, False, False)
        search_hbox.pack_start(create_right_align(), True, True)
        # search_hbox.pack_start(search_align, False, False)
        search_hbox_align = gtk.Alignment()
        search_hbox_align.set_padding(5, 0, 0, 0)
        search_hbox_align.set(0, 0, 1, 1)
        search_hbox_align.add(search_hbox)
        
        # Layout on the right.
        content_box = gtk.VBox(spacing=5)
        content_box.pack_start(search_hbox_align, False, False)
        content_box.pack_start(self.switch_view_box, True, True)
        
        body_paned = HPaned(handle_color=app_theme.get_color("panedHandler"), enable_drag=True)
        body_paned.add1(left_vbox)
        body_paned.add2(content_box)
        
        self.pack_start(body_paned, True, True)
        
        
    def get_search_entry(self):    
        search_entry = SearchEntry("")
        search_entry.set_size(155, 22)
        
        align = gtk.Alignment()
        # align.set(0.5, 0.5, 0, 0)
        align.set_padding(2, 0, 0, 10)
        align.add(search_entry)
        search_entry.entry.connect("changed", self.on_search_entry_changed)
        return search_entry, align
        
    def get_icon_view(self, padding_x=0, padding_y=10):    
        ''' Draggable IconView '''
        icon_view = IconView(padding_x, padding_y)
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        icon_view.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        icon_view.connect("drag-data-get", self.__on_drag_data_get) 
        icon_view.connect("double-click-item", self.__on_double_click_item)
        icon_view.connect("single-click-item", self.__on_single_click_item)
        icon_view.connect("right-click-item", self.__on_right_click_item)
        
        icon_view.draw_mask  = self.on_iconview_draw_mask
        scrolled_window = ScrolledWindow()
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(icon_view)
        return icon_view, scrolled_window
        
    def get_song_view(self):
        song_view = MultiDragSongView(has_title=True)
        song_view.keymap.update({"BackSpace" : self.on_songview_backspace_press})        
        # song_view.add_titles([_("Title"), _("Artist"), _("Album"), _("Added time")])
        # scrolled_window = song_view.get_scrolled_window()
        return song_view
        # return song_view, scrolled_window
    
    def connect_to_db(self):
        self.autoconnect(self.__db_query, "added", self.__on_added_songs)
        self.autoconnect(self.__db_query, "removed", self.__on_removed_songs)
        self.autoconnect(self.__db_query, "update-tag", self.__on_update_tags)
        self.autoconnect(self.__db_query, "full-update", self.__on_full_update)
        self.autoconnect(self.__db_query, "quick-update", self.__on_quick_update)
        self.__db_query.set_query("")
        
    def __on_added_songs(self, db_query, songs):    
        self.reload_flag = True
    
    def __on_removed_songs(self, db_query, songs):
        self.reload_flag = True
    
    def __on_update_tags(self, db_query, infos, songs):
        self.reload_flag = True
    
    def __on_full_update(self, db_query):
        for tag in ["artist", "album", "genre", "folder"]:
            self.load_view(tag)
    
    def __on_quick_update(self, db_query, songs):
        pass
    
    def on_interval_loaded_view(self):
        if self.reload_flag and not self.search_flag:
            for tag in ["artist", "album", "genre", "folder"]:
                self.load_view(tag)
            self.reload_flag = False    
        return True    
    
    def on_dispatcher_reload_browser(self, obj, infos):
        if infos: self.reload_flag = True
    
    def on_folders_view_motion_notify(self, widget, item, x, y):
        Tooltip.text(widget, item.value_name)
        
    def on_songview_backspace_press(self):    
        self.on_back_button_clicked(None)
        
    def on_search_entry_changed(self, widget, text):    
        songs_view_mode = self.back_hbox.get_visible()
        if songs_view_mode:
            if not self.search_flag:
                self.cache_search_list = self.songs_view.items[:]
            # Clear songs view select status.    
            self.songs_view.select_rows = []
            if text != "":
                self.search_flag = True
                results = filter(lambda item: text.lower().replace(" ", "") in item.get_song().get("search", ""), 
                                 self.cache_search_list)
                self.songs_view.items = results
                self.songs_view.update_item_index()
                self.songs_view.update_vadjustment()
            else:    
                self.search_flag = False
                self.songs_view.items = self.cache_search_list
                self.songs_view.update_item_index()
                self.songs_view.update_vadjustment()
            self.songs_view.queue_draw()    
        else:    
            view_widget = self.get_current_view()
            if not view_widget: return 
            if not self.search_flag:
                self.cache_search_view = view_widget.items[:]
            if text != "":    
                self.search_flag = True
                results = filter(lambda item: text.lower().replace(" ", "") in item.retrieve, self.cache_search_view)
                view_widget.items = results
            else:    
                self.search_flag = False
                view_widget.items = self.cache_search_view
            view_widget.queue_draw()    
    
    def __on_drag_data_get(self, widget, context, selection, info, timestamp):
        item = widget.highlight_item
        if not item: return
        
        songs = self.get_item_songs(item)
        songs = list(songs)
        songs.sort()
        song_uris = [song.get("uri") for song in songs ]
        selection.set("text/deepin-songs", 8, "\n".join(song_uris))
        selection.set_uris(song_uris)
    
    def __on_double_click_item(self, widget, item, x, y):
        self.search_entry.entry.set_text("")        
        
        songs = self.get_item_songs(item)
        self.songs_view.clear()        
        self.songs_view.add_songs(songs)
        
        # todo: switch view mode fixed the back.
        switch_tab(self.switch_view_box, self.songs_view)
        
        # show back button.
        self.prompt_button.set_infos((item.pixbuf, item.name_label))
        self.back_hbox.set_no_show_all(False)
        self.back_hbox.show_all()
        
    def get_current_view(self):    
        index = self.filterbar.get_index()
        widget = None
        if index   == 0: widget = self.artists_view
        elif index == 1: widget = self.albums_view   
        elif index == 2: widget = self.genres_view
        elif index == 3: widget = self.folders_view
        return widget
    
    def get_current_view_sw(self):
        index = self.filterbar.get_index()
        widget = None
        if index   == 0: widget = self.artists_sw
        elif index == 1: widget = self.albums_sw
        elif index == 2: widget = self.genres_sw
        elif index == 3: widget = self.folders_sw
        return widget
    
    def on_back_button_clicked(self, obj):    
        # clear entry text.
        self.search_entry.entry.set_text("")
        
        widget = self.get_current_view_sw()
        if widget: switch_tab(self.switch_view_box, widget)
        
        # hide backhbox.
        self.back_hbox.hide_all()
        self.back_hbox.set_no_show_all(True)
        
    def __on_single_click_item(self, widget, item, x, y):    
        if item.pointer_in_play_rect(x, y):
            self.play_item(item)
            
    def play_item(self, item):        
        songs = self.get_item_songs(item)
        songs = list(songs)
        songs.sort()
        Dispatcher.play_and_add_song(songs)
        
    def emit_to_list(self, item):    
        songs = self.get_item_songs(item)
        Dispatcher.add_songs(songs)
        
    def real_remove_item(self, item, fully=False):    
        songs = self.get_item_songs(item)
        MediaDB.remove(songs)
        
        if fully:
            try:
                [ utils.move_to_trash(song.get("uri")) for song in songs if song.get_type() != "cue" ]
            except: pass    
           
    def try_move_trash(self, item):        
        ConfirmDialog(_("Prompt"), _("Are you sure to delete?"), 
                      confirm_callback=lambda : self.real_remove_item(item, True)).show_all()
        
    def change_item_cover(self, item):    
        new_cover_path = WinFile(False, _("Select image")).run()
        if new_cover_path:
            item.change_cover_pixbuf(new_cover_path)
            
    def __on_right_click_item(self, widget, item, x, y):
        if not item: return
        menu_items = [
            (None, _("Play All"), lambda : self.play_item(item)),
            (None, _("Add to List"), lambda : self.emit_to_list(item)),
            None,
            (None, _("Remove from Library"), lambda : self.real_remove_item(item)),
            (None, _("Move to Trash"), lambda : self.try_move_trash(item)),
            None,
            ]
        
        if item.tag == "folder":
            menu_items.append((None, _("Open Directory"), lambda : utils.open_directory(item.value_name)))
        elif item.tag in ["artist", "album"] and item.key_name != "deepin-all-songs":    
            menu_items.append((None, _("Change Cover"), lambda : self.change_item_cover(item)))
        else:    
            menu_items.pop()
            
        Menu(menu_items, True).show((int(x), int(y)))    
        
    def get_item_songs(self, item):    
        if item.tag == "folder":
            songs = self.__db_query.get_attr_songs(item.value_name)
        else:    
            if item.key_name == "deepin-all-songs":
                songs = self.__db_query.get_all_songs()
            else:    
                self.__selected_tag[item.tag] = [item.key_name]
                songs = self.__get_selected_songs(item.tag)
        return songs        

                
    def __get_selected_songs(self, tag="artist"):
        artists = []
        albums = []
        genres = []
        if   tag == "artist": artists = self.__selected_tag["artist"]
        elif tag == "album": albums = self.__selected_tag["album"]
        elif tag == "genre": genres = self.__selected_tag["genre"]
        return self.__db_query.get_songs(genres, artists, albums)    
    
    def get_infos_from_db(self, tag, values=None):
        genres = []
        artists = []
        extened = False
        
        return self.__db_query.get_info(tag, genres, artists, values, extened)
    
    def load_view(self, tag="artist", switch=False):
        items = self.get_info_items(tag)
        
        if  tag == "artist": 
            self.artists_view.clear()
            self.artists_view.add_items(items)    
        elif tag == "album" :
            self.albums_view.clear()
            self.albums_view.add_items(items)
        elif tag == "genre" :
            self.genres_view.clear()
            self.genres_view.add_items(items)
        elif tag == "folder":
            self.folders_view.clear()
            self.folders_view.add_items(items)
        
    def get_info_items(self, tag):    
        if tag == "folder":
            infos = self.__db_query.get_attr_infos()
            return [ LocalItem(info) for info in infos ]
        
        _dict = self.get_infos_from_db(tag)
        keys = _dict.keys()
        keys.sort()
        items = []
        all_nb = len(self.__db_query.get_all_songs())
        items.append(LocalItem(("deepin-all-songs", "deepin-all-songs", all_nb, tag)))
        
        for key in keys:
            value, nb = _dict[key]
            items.append(LocalItem((key, value, nb, tag)))
        return items    
    
    def switch_filter_view(self, tag):    
        
        # clear search entry
        self.search_entry.entry.set_text("")
        
        widget = None
        if tag == "artist" : widget = self.artists_sw
        elif tag == "album": widget = self.albums_sw
        elif tag == "genre": widget = self.genres_sw
        elif tag == "folder" : widget = self.folders_sw
            
        if widget:    
            self.back_hbox.hide_all()
            self.back_hbox.set_no_show_all(True)
            switch_tab(self.switch_view_box, widget)
    
    def on_iconview_draw_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, "layoutLast")
        
class SimpleBrowser(Browser):    
    _type = ["local", "cue"]
    
    def __init__(self):
        Browser.__init__(self, DBQuery(self._type))
        if MediaDB.isloaded():
            self.__on_db_loaded(MediaDB)
        else:    
            self.autoconnect(MediaDB, "loaded", self.__on_db_loaded)
            
    def __on_db_loaded(self, db):        
        self.connect_to_db()

local_browser = SimpleBrowser()        
