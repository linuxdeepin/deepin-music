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

from dtk.ui.categorybar import Categorybar
from dtk.ui.listview import ListView, render_text
from dtk.ui.draw import draw_pixbuf, draw_font
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.button import ImageButton
from dtk.ui.iconview import IconView

from library import MediaDB, DBQuery
from helper import SignalContainer 
from widget.ui import app_theme, MultiDragSongView, SearchEntry
from widget.ui_utils import switch_tab
from cover_manager import CoverManager


class IconItem(gobject.GObject):
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, _tuple):
        super(IconItem, self).__init__()
        self.name, nums, self.tag = _tuple
        if not self.name:
            self.name_label= "其它"
        elif self.name == "deepin-all-songs":    
            self.name_label = "所有歌曲"
        else:    
            self.name_label = self.name
            
        self.labels = "%d首歌曲" % nums
        self.cell_width = 80
        self.pixbuf = CoverManager.get_pixbuf_from_album(self.name_label, self.cell_width, self.cell_width)
        self.padding_x = 10
        self.padding_y = 15
        self.hover_flag = False
        self.highlight_flag = False
        
       
    def emit_redraw_request(self):    
        self.emit("redraw-request")
       
    def get_width(self):    
        return self.pixbuf.get_width() + self.padding_x * 2
    
    def get_height(self):
        return self.pixbuf.get_height() + self.padding_y * 2 + 20
    
    def render(self, cr, rect):
        border_size = 2
        
        if self.hover_flag:
            cr.set_source_rgb(1, 0, 0)
        elif self.highlight_flag:
            cr.set_source_rgb(0, 1, 0)
        else:    
            cr.set_source_rgb(1, 1, 1)
            
        cr.rectangle(
            rect.x + (rect.width - self.pixbuf.get_width()) / 2  - border_size,
            rect.y + (rect.height - self.pixbuf.get_height()) / 2 - 10 - border_size,
            self.pixbuf.get_width() + border_size * 2,
            self.pixbuf.get_height() + border_size * 2)
        cr.fill()
        
        # Draw cover
        
        # side_pixbuf = app_theme.get_pixbuf("cover/side.png").get_pixbuf()
        # draw_pixbuf(cr, side_pixbuf, rect.x + self.padding_x, rect.y + self.padding_y)
                
        draw_pixbuf(cr, self.pixbuf, 
                    rect.x + self.padding_x,
                    rect.y + self.padding_y)
        name_rect = gtk.gdk.Rectangle(rect.x + self.padding_x , rect.y + self.pixbuf.get_height() + self.padding_y + 8 , self.cell_width, 10)
        num_rect = gtk.gdk.Rectangle(name_rect.x, name_rect.y + 14, name_rect.width, name_rect.height)
        render_text(cr, name_rect, self.name_label, 2, font_size=12)
        render_text(cr, num_rect, self.labels, 2, font_size=9)
        
        
    def icon_item_motion_notify(self, x, y):    
        self.hover_flag = True
        self.emit_redraw_request()
        
    def icon_item_lost_focus(self):    
        self.hover_flag = False
        self.emit_redraw_request()
        
    def icon_item_highlight(self):    
        self.highlight_flag = True
        self.emit_redraw_request()
        
    def icon_item_normal(self):    
        self.highlight_flag = False
        self.emit_redraw_request()
        
gobject.type_register(IconItem)        

class Browser(gtk.VBox, SignalContainer):
    
    def __init__(self, db_query):
        
        gtk.VBox.__init__(self)
        SignalContainer.__init__(self)
        self.__db_query = db_query
        self._tree = {}
        self.__selected_tag = {"album": [], "artist": [], "genre": []}
        # self.connect("destroy", self.on_destroy)
        
        self.__labels = {
            "genre" : "流派",
            "artist" : "艺术家",
            "album" : "专辑",
            "genres" : "流派",
            "artists" : "艺术家",
            "albums" : "专辑"
            }
        
        # init widget.
        self.entry_box = SearchEntry("")
        self.entry_box.set_size(155, 25)
        entry_align = gtk.Alignment()
        entry_align.set_padding(5, 5, 5, 5)
        entry_align.set(0, 0, 1, 1)
        upper_align = gtk.Alignment()
        upper_align.set(0, 0, 0, 1)
        self.back_button = self.__create_simple_button("back", self.__switch_to_filter_view)
        back_align = gtk.Alignment()
        back_align.set(0.5, 0.5, 0, 0)
        back_align.set_padding(0, 0, 0, 20)
        back_align.add(self.back_button)
        self.upper_box = gtk.HBox(spacing=5)
        self.upper_box.pack_start(upper_align, True, True)
        self.upper_box.pack_start(back_align, False, False)
        self.upper_box.pack_start(self.entry_box, False, False)
        entry_align.add(self.upper_box)
        
        self.filter_categorybar = Categorybar([
                (app_theme.get_pixbuf("filter/artist_normal.png"), "按歌手", lambda : self.reload_filter_view("artist", True)),
                (app_theme.get_pixbuf("filter/album_normal.png"), "按专辑", lambda : self.reload_filter_view("album", True)),
                (app_theme.get_pixbuf("filter/genre_normal.png"), "按流派", lambda : self.reload_filter_view("genre", True)),
                # (app_theme.get_pixbuf("filter/customer_normal.png"), "自定义", None),
                # (app_theme.get_pixbuf("filter/all_normal.png"), "所有歌曲", None)
                ])
        self.filter_categorybar.set_size_request(-1, 200)
        self.filter_view = IconView(background_pixbuf=app_theme.get_pixbuf("skin/main.png"))
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        self.filter_view.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        self.filter_view.connect("drag-data-get", self.__on_drag_data_get) 
        self.filter_view.connect("double-click-item", self.__on_double_click_item)
        self.filter_scrolled_window = ScrolledWindow(app_theme.get_pixbuf("skin/main.png"))
        self.filter_scrolled_window.add_child(self.filter_view)
        
        self.songs_view = MultiDragSongView(background_pixbuf=app_theme.get_pixbuf("skin/main.png"))
        self.songs_view.add_titles(["歌名", "艺术家", "专辑", "添加时间"])
        self.songs_scrolled_window = ScrolledWindow(app_theme.get_pixbuf("skin/main.png"))
        self.songs_scrolled_window.add_child(self.songs_view)
        
        align = gtk.Alignment()
        align.set(0, 1, 0, 0)
        left_box = gtk.VBox()
        left_box.pack_start(self.filter_categorybar, False, False)
        left_box.pack_start(align, True, True)
        
        self.right_box = gtk.VBox()
        self.right_box.add(self.filter_scrolled_window)
        body_box = gtk.HBox()

        body_box.pack_start(left_box,  False, False)
        body_box.pack_start(self.right_box, True, True)
        self.pack_start(entry_align, False, False)
        self.pack_start(body_box, True, True)
        
    def __create_simple_button(self, name, callback):    
        button = ImageButton(
            app_theme.get_pixbuf("filter/%s_normal.png" % name),
            app_theme.get_pixbuf("filter/%s_normal.png" % name),
            app_theme.get_pixbuf("filter/%s_press.png" % name),
            )
        if callback:
            button.connect("clicked", callback)
        return button    
    
    def __switch_to_filter_view(self, widget):
        self.switch_box(self.right_box, self.filter_scrolled_window)
        
    def reload_filter_view(self, tag="artist", switch=False):    
        if switch:
            self.filter_view.clear()
        
        _dict = self.get_infos_from_db(tag)
        keys = _dict.keys()
        keys.sort()
        items = []
        all_nb = len(self.__db_query.get_all_songs())
        items.append(IconItem(("deepin-all-songs", all_nb, tag)))
                
        for key in keys:
            value, nb = _dict[key]
            items.append(IconItem((value, nb, tag)))
        self.filter_view.add_items(items)    
        if switch:
            self.switch_box(self.right_box, self.filter_scrolled_window)
            
    def get_infos_from_db(self, tag, values=None):
        genres = []
        artists = []
        extened = False
        return self.__db_query.get_info(tag, genres, artists, values, extened)
                
    def connect_to_db(self):    
        self.autoconnect(self.__db_query, "added", self.__added_song_cb)
        self.autoconnect(self.__db_query, "removed", self.__removed_song_cb)
        self.autoconnect(self.__db_query, "update-tag", self.__update_tag_view)
        self.autoconnect(self.__db_query, "full-update", self.__full_update)
        self.autoconnect(self.__db_query, "quick-update", self.__quick_update)
        self.__db_query.set_query("")
        
    def __added_song_cb(self, db_query, songs):
        pass

    def __removed_song_cb(self, db_query, songs):
        pass
    
    def __update_tag_view(self, db_query, tag, values):
        pass
    
    def __quick_update(self, db_query, songs):
        pass
        
    def __full_update(self, db_query):    
        self.reload_filter_view()
    
    def __get_selected_songs(self, tag="artist"):
        artists = []
        albums = []
        genres = []
        
        # if tag in ["genre", "artist", "album"]:
        #     genres = self.__selected_tag["genre"]
        # if tag in ["artist", "album"]:
        #     artists = self.__selected_tag["artist"]
        # if tag in ["album"]:
        #     albums = self.__selected_tag["album"]
        
        if tag == "artist":
            artists = self.__selected_tag["artist"]
        elif tag == "album":    
            albums = self.__selected_tag["album"]
        elif tag == "genre":    
            genres = self.__selected_tag["genre"]
            
        return self.__db_query.get_songs(genres, artists, albums)    
    
    def __on_drag_data_get(self, widget, context, selection, info, timestamp):
        item = self.filter_view.highlight_item
        if not item:
            return 
        del self.__selected_tag[item.tag]
        self.__selected_tag[item.tag] = [item.name]
        songs = self.__get_selected_songs(item.tag)
        if not songs:
            return 
        songs = list(songs)
        songs.sort()
        list_uris = list([ song.get("uri") for song in songs])
        selection.set("text/deepin-songs", 8, "\n".join(list_uris))
        selection.set_uris(list_uris)
    
    def __on_double_click_item(self, widget,  item, x, y):
        self.songs_view.clear()
        
        if item.name == "deepin-all-songs":
            songs = self.__db_query.get_all_songs()
        else:    
            self.__selected_tag[item.tag] = [item.name]
            songs = self.__get_selected_songs(item.tag)
            
        self.songs_view.add_songs(songs)
        self.songs_view.set_sort_keyword(item.tag)
        self.switch_box(self.right_box, self.songs_scrolled_window)
        
    def switch_box(self, parent, child):    
        switch_tab(parent, child)
    
class SimpleBrowser(Browser):    
    _type = "local"
    
    def __init__(self):
        Browser.__init__(self, DBQuery(self._type))
        
        if MediaDB.isloaded():
            self.__on_db_loaded(MediaDB)
        else:    
            self.autoconnect(MediaDB, "loaded", self.__on_db_loaded)
            
    def __on_db_loaded(self, db):        
        self.connect_to_db()
