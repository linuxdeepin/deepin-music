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

from dtk.ui.listview import  render_text
from dtk.ui.draw import draw_pixbuf
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.button import ImageButton
from dtk.ui.iconview import IconView
from dtk.ui.utils import alpha_color_hex_to_cairo
from dtk.ui.line import HSeparator

from library import MediaDB, DBQuery
from helper import SignalContainer, Dispatcher
from widget.ui import app_theme, MultiDragSongView, SearchEntry
from widget.ui_utils import switch_tab
from widget.outlookbar import OptionBar, OptionTitleBar
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
        self.cell_width = 100
        self.pixbuf = CoverManager.get_pixbuf_from_album(self.name_label, self.cell_width, self.cell_width)
        self.padding_x = 4
        self.padding_y = 4
        self.hover_flag = False
        self.highlight_flag = False
        self.__draw_play_hover_flag = False
        self.__draw_play_press_flag = False
        self.__normal_side_pixbuf =  app_theme.get_pixbuf("filter/side_normal.png").get_pixbuf()
        self.__normal_play_pixbuf =  app_theme.get_pixbuf("filter/play_normal.png").get_pixbuf()
        
        self.play_rect = gtk.gdk.Rectangle(
            self.__normal_side_pixbuf.get_width() - self.__normal_play_pixbuf.get_width() - 2 - 4,
            self.__normal_side_pixbuf.get_height() - self.__normal_play_pixbuf.get_height() - 2 - 4,
            self.__normal_play_pixbuf.get_width(),
            self.__normal_play_pixbuf.get_height()
            )
        
    def pointer_in_play_rect(self, x, y):    
        if self.play_rect.x < x < self.play_rect.x + self.play_rect.width and self.play_rect.y < y < self.play_rect.y + self.play_rect.height:
            return True
        else:
            return False
        
    def emit_redraw_request(self):    
        self.emit("redraw-request")
       
    def get_width(self):    
        return self.__normal_side_pixbuf.get_width() + self.padding_x * 2
    
    def get_height(self):
        return self.__normal_side_pixbuf.get_height() + self.padding_y * 2 + 20
    
    def render(self, cr, rect):
        
        if self.hover_flag:
            side_pixbuf = app_theme.get_pixbuf("filter/side_hover.png").get_pixbuf()

            
        elif self.highlight_flag:
            side_pixbuf = app_theme.get_pixbuf("filter/side_hover.png").get_pixbuf()
            
        else:    
            side_pixbuf = self.__normal_side_pixbuf
            
        # Draw cover.
        draw_pixbuf(cr, self.pixbuf, 
                    rect.x + self.padding_x,
                    rect.y + self.padding_y)

        draw_pixbuf(cr, side_pixbuf, rect.x, rect.y )       
        
        if self.hover_flag:
            if self.__draw_play_hover_flag:
                play_pixbuf = app_theme.get_pixbuf("filter/play_hover.png").get_pixbuf()
            elif self.__draw_play_press_flag:    
                play_pixbuf = app_theme.get_pixbuf("filter/play_press.png").get_pixbuf()
            else:    
                play_pixbuf = self.__normal_play_pixbuf
                
            draw_pixbuf(cr, play_pixbuf, rect.x + self.play_rect.x, rect.y + self.play_rect.y)        
            
        # Draw text.    
        name_rect = gtk.gdk.Rectangle(rect.x + self.padding_x , 
                                      rect.y + self.__normal_side_pixbuf.get_height() + 2,
                                      self.cell_width, 10)
        num_rect = gtk.gdk.Rectangle(name_rect.x, name_rect.y + 14, name_rect.width, name_rect.height)
        
        render_text(cr, name_rect, self.name_label, 2, font_size=12)
        render_text(cr, num_rect, self.labels, 2, font_size=9)
        
    def icon_item_motion_notify(self, x, y):    
        self.hover_flag = True
        if self.pointer_in_play_rect(x, y):
            if self.__draw_play_press_flag:
                self.__draw_play_hover_flag =  False
            else:    
                self.__draw_play_hover_flag = True
        else:    
            self.__draw_play_hover_flag = False
        self.emit_redraw_request()
        
    def icon_item_lost_focus(self):    
        self.__draw_play_flag = False
        self.hover_flag = False
        self.emit_redraw_request()
        
    def icon_item_highlight(self):    
        self.highlight_flag = True
        self.emit_redraw_request()
        
    def icon_item_normal(self):    
        self.highlight_flag = False
        self.emit_redraw_request()
        
    def icon_item_button_press(self, x, y):    
        if self.pointer_in_play_rect(x, y):
            self.__draw_play_hover_flag =  False
            self.__draw_play_press_flag = True
        else:    
            self.__draw_play_press_flag = False
        self.emit_redraw_request()
    
    def icon_item_button_release(self, x, y):
        self.__draw_play_press_flag = False
        self.emit_redraw_request()
    
    def icon_item_single_click(self, x, y):
        pass
    
    def icon_item_double_click(self, x, y):
        pass
        
gobject.type_register(IconItem)        

FILTER_VIEW, SONG_VIEW = 1, 2

class Browser(gtk.HBox, SignalContainer):
    
    def __init__(self, db_query):
        
        gtk.HBox.__init__(self)
        SignalContainer.__init__(self)
        self.__db_query = db_query
        self._tree = {}
        self.__selected_tag = {"album": [], "artist": [], "genre": []}
        self.view_mode = FILTER_VIEW
        self.__search_flag = False
        self.__song_cache_items = []
        
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
        self.entry_box.entry.connect("changed", self.__search_cb)
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
        
        self.filter_categorybar = OptionBar(
            [(app_theme.get_pixbuf("filter/artist_normal.png"), app_theme.get_pixbuf("filter/artist_press.png"),
              "按歌手", lambda : self.reload_filter_view("artist", True)),
             (app_theme.get_pixbuf("filter/album_normal.png"), app_theme.get_pixbuf("filter/album_press.png"),
              "按专辑", lambda : self.reload_filter_view("album", True)),
             (app_theme.get_pixbuf("filter/genre_normal.png"), app_theme.get_pixbuf("filter/genre_press.png"),
              "按流派", lambda : self.reload_filter_view("genre", True)),]
                                            )
        
        # song path.
        self.path_categorybar = OptionTitleBar(
            (app_theme.get_pixbuf("filter/local_normal.png"), app_theme.get_pixbuf("filter/local_press.png"),
              "本地歌曲", None),
            [(app_theme.get_pixbuf("filter/artist_normal.png"), app_theme.get_pixbuf("filter/artist_press.png"),
              "按歌手", lambda : self.reload_filter_view("artist", True)),
             (app_theme.get_pixbuf("filter/album_normal.png"), app_theme.get_pixbuf("filter/album_press.png"),
              "按专辑", lambda : self.reload_filter_view("album", True)),
             (app_theme.get_pixbuf("filter/genre_normal.png"), app_theme.get_pixbuf("filter/genre_press.png"),
              "按流派", lambda : self.reload_filter_view("genre", True)),] 
            )
        
        # iconview.
        self.filter_view = IconView(background_pixbuf=app_theme.get_pixbuf("skin/main.png"))
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        self.filter_view.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        self.filter_view.connect("drag-data-get", self.__on_drag_data_get) 
        self.filter_view.connect("double-click-item", self.__on_double_click_item)
        self.filter_view.connect("single-click-item", self.__on_single_click_item)
        self.filter_scrolled_window = ScrolledWindow(app_theme.get_pixbuf("skin/main.png"))
        self.filter_scrolled_window.add_child(self.filter_view)
        
        self.songs_view = MultiDragSongView(background_pixbuf=app_theme.get_pixbuf("skin/main.png"))
        self.songs_view.add_titles(["歌名", "艺术家", "专辑", "添加时间"])
        self.songs_scrolled_window = ScrolledWindow(app_theme.get_pixbuf("skin/main.png"))
        self.songs_scrolled_window.add_child(self.songs_view)
        
        align = gtk.Alignment()
        align.set(0, 1, 0, 0)
        left_box = gtk.VBox(spacing=20)
        left_box.pack_start(self.filter_categorybar, False, False)
        left_box.pack_start(self.create_separator_box(), False, False)
        left_box.pack_start(self.path_categorybar, False, False)
        left_box.pack_start(self.create_separator_box(), False, False)
        
        left_box.pack_start(align, True, True)
        left_box.connect("expose-event", self.expose_left_box_mask)
        self.right_box = gtk.VBox()
        self.right_box.add(self.filter_scrolled_window)
        body_box = gtk.VBox()

        body_box.pack_start(entry_align,  False, False)
        body_box.pack_start(self.right_box, True, True)
        self.pack_start(left_box, False, False)
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
    
    def expose_left_box_mask(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        color_info = app_theme.get_alpha_color("playlistRight").get_color_info()
        cr.set_source_rgba(*alpha_color_hex_to_cairo(color_info))
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        return False
    
    def create_separator_box(self, padding_x=0, padding_y=0):
        separator_box = HSeparator(
            app_theme.get_shadow_color("hSeparator").get_color_info(),
            padding_x, padding_y)
        return separator_box
    
    def __switch_to_filter_view(self, widget):
        self.switch_box(self.right_box, self.filter_scrolled_window)
        self.view_mode = FILTER_VIEW
        
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
            self.view_mode = FILTER_VIEW
            
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
        # self.__db_query.get_attr_info()
    
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
        
        if item.name == "deepin-all-songs":
            songs = self.__db_query.get_all_songs()
        else:    
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
        self.entry_box.entry.set_text("")
        
        if item.name == "deepin-all-songs":
            songs = self.__db_query.get_all_songs()
        else:    
            self.__selected_tag[item.tag] = [item.name]
            songs = self.__get_selected_songs(item.tag)
            
        self.songs_view.add_songs(songs)
        self.songs_view.set_sort_keyword(item.tag)
        self.switch_box(self.right_box, self.songs_scrolled_window)
        self.view_mode = SONG_VIEW
        
    def __on_single_click_item(self, widget, item, x, y):    
        if item.pointer_in_play_rect(x, y):
            if item.name == "deepin-all-songs":
                songs = self.__db_query.get_all_songs()
            else:    
                del self.__selected_tag[item.tag]
                self.__selected_tag[item.tag] = [item.name]
                songs = self.__get_selected_songs(item.tag)
            if not songs:
                return 
            songs = list(songs)
            songs.sort()
            if not songs:
                return 
            Dispatcher.play_and_add_song(songs)
        
        
    def __search_cb(self, widget, text):    
        if self.view_mode == SONG_VIEW:
            if not self.__search_flag:
                self.__song_cache_items = self.songs_view.items[:]
                
            # Clear song_view select status    
            self.songs_view.select_rows = []    
            if text != "":
                self.__search_flag = True
                results = filter(lambda item: text.lower().replace(" ", "") in item.get_song().get("search", ""), self.__song_cache_items)
                self.songs_view.items = results
                self.songs_view.update_item_index()
                self.songs_view.update_vadjustment()
            else:    
                self.__search_flag = False
                self.songs_view.items = self.__song_cache_items
                self.songs_view.update_item_index()
                self.songs_view.update_vadjustment()
            self.songs_view.queue_draw()    
        
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
