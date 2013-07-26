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
from dtk.ui.listview import ListView
from dtk.ui.iconview import IconView
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.menu import Menu



from widget.ui_utils import draw_alpha_mask, draw_single_mask, switch_tab
from widget.ui import SearchPrompt
from widget.webcast_item import WebcastIconItem, WebcastListItem, CompositeIconItem
from helper import Dispatcher
from player import Player
from song import Song
from webcast_library import WebcastDB
from xdg_support import get_config_file
from nls import _
import utils

class WebcastView(ListView):
    __gsignals__ = {
        "begin-add-items" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "empty-items" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        }

    def __init__(self, *args, **kwargs):
        ListView.__init__(self, *args, **kwargs)
        targets = [("text/deepin-webcasts", gtk.TARGET_SAME_APP, 1),]
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP, targets, gtk.gdk.ACTION_COPY)

        self.connect_after("drag-data-received", self.on_drag_data_received)
        self.connect("single-click-item", self.__on_single_click_item)
        self.connect("double-click-item", self.on_double_click_item)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("delete-select-items", self.try_emit_empty_signal)
        self.connect("right-press-items", self.on_right_press_items)

        self.set_expand_column(1)
        Dispatcher.connect("play-webcast", self.on_dispatcher_play_webcast)
        Dispatcher.connect("add-webcasts", self.on_dispatcher_add_webcasts)
        self.limit_number = 25
        WebcastDB.connect("changed", self.on_db_update_songs)
        
        self.preview_db_file = get_config_file("preview_webcasts.db")


    def on_db_update_songs(self, db, infos):
        all_items = self.items
        for song, tags, new_tags in infos:
            webcast_item = WebcastListItem(song)
            if webcast_item in all_items:
                all_items[all_items.index(webcast_item)].update_webcast(song)


    def draw_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, "layoutMiddle")

    def draw_item_hover(self, cr, x, y, w, h):
        draw_single_mask(cr, x + 1, y, w - 2, h, "globalItemHover")

    def draw_item_select(self, cr, x, y, w, h):
        draw_single_mask(cr, x + 1, y, w - 2, h, "globalItemSelect")

    def draw_item_highlight(self, cr, x, y, w, h):
        draw_single_mask(cr, x + 1, y, w - 2, h, "globalItemHighlight")


    def try_emit_empty_signal(self, widget, items):
        if len(self.items) <= 0:
            self.emit("empty-items")

    def on_double_click_item(self, widget, item, column, x, y):
        self.play_item(item)
                
    def play_item(self, item):            
        if item:
            self.set_highlight(item)
            Player.play_new(item.get_webcast(), seek=item.get_webcast().get("seek", 0))
            if Player.get_source() != self:
                Player.set_source(self)

    def on_button_press_event(self, widget, event):
        ''' TODO: Popup Menu. '''
        pass
    
    def on_right_press_items(self, widget, x, y, current_item, select_items):
        # if len(select_items) > 1:
        if current_item and select_items:
            if len(select_items) > 1:
                items = [
                    (None, _("Delete"), lambda : self.delete_items(select_items)),
                    (None, _("Clear List"), lambda : self.clear_items()),
                    ]
            else:    
                items = [
                    (None, _("Play"), lambda : self.play_item(current_item)),
                    (None, _("Delete"), lambda : self.delete_items([current_item])),
                    self.get_favorite_menu_item(current_item),
                    None,
                    (None, _("Clear List"), lambda : self.clear_items()),
                    ]
            Menu(items, True).show((int(x), int(y)))
            
    def clear_items(self):        
        self.clear()
        self.emit("empty-items")

    def on_drag_data_received(self, widget, context, x, y, selection, info, timestamp):
        root_y = widget.allocation.y + y
        try:
            pos = self.get_coordinate_row(root_y)
        except: pos = None

        if pos == None:
            pos = len(self.items)

        if selection.target == "text/deepin-webcasts":
            webcasts_data =  selection.data
            webcast_taglists =  eval(webcasts_data)

            webcasts = []
            for tag in webcast_taglists:
                webcast = Song()
                webcast.init_from_dict(tag)
                webcast.set_type("webcast")
                webcasts.append(webcast)
            self.add_webcasts(webcasts)

    def get_webcasts(self):
        return [item.get_webcast() for item in self.items]

    def is_empty(self):
        return len(self.items) == 0

    def get_previous_song(self):
        del self.select_rows[:]
        self.queue_draw()
        if not self.items: return None

        if self.highlight_item != None:
            if self.highlight_item in self.items:
                current_index = self.items.index(self.highlight_item)
                prev_index = current_index - 1
                if prev_index < 0:
                    prev_index = len(self.items) - 1
                highlight_item = self.items[prev_index]
            else:
                highlight_item = self.items[0]
        else:
            highlight_item = self.items[0]
        self.set_highlight(highlight_item)
        return highlight_item.get_webcast()


    def get_next_song(self, maunal=False):
        del self.select_rows[:]
        self.queue_draw()
        if not self.items: return None

        if self.highlight_item:
            if self.highlight_item in self.items:
                current_index = self.items.index(self.highlight_item)
                next_index = current_index + 1
                if next_index > len(self.items) - 1:
                    next_index = 0
                highlight_item = self.items[next_index]
            else:
                highlight_item = self.items[0]

        else:
            highlight_item = self.items[0]
        self.set_highlight(highlight_item)
        return highlight_item.get_webcast()

    def on_dispatcher_play_webcast(self, obj, webcast):
        self.add_webcasts([webcast], play=True, pos=0)
        
    def __on_single_click_item(self, widget, item, column, x, y):
        if column == 2:
            self.toggle_item_collected(item)
            
    def toggle_item_collected(self, item):        
        if not item: return 
        song = item.webcast
        if song.get("collected", False):
            collected = False
        else:
            collected = True
        WebcastDB.set_property(song, {"collected": collected}, emit_update=False)
        item.update_webcast(song)
        Dispatcher.emit("change-webcast", song)
        
    def get_favorite_menu_item(self, item):
        song = item.webcast        
        if song.get("collected", False):
            menu_name = _("Remove from Favorites")
        else:
            menu_name = _("Add to Favorites")
        return (None, menu_name, lambda : self.toggle_item_collected(item))    
    
    def on_dispatcher_add_webcasts(self, widget, items):
        if items:
            self.add_webcasts(items)
        
    def add_webcasts(self, webcasts, pos=None, sort=False, play=False):
        if not webcasts:
            return
        if not isinstance(webcasts, (list, tuple, set)):
            webcasts = [ webcasts ]

        webcast_items = [ WebcastListItem(webcast) for webcast in webcasts if webcast not in self.get_webcasts()]

        if webcast_items:
            if not self.items:
                self.emit_add_signal()
            self.add_items(webcast_items, pos, sort)

            if len(self.items) > self.limit_number:
                being_delete_items = self.items[self.limit_number:]
                if self.highlight_item in being_delete_items:
                    being_delete_items.remove(self.highlight_item)
                self.delete_items(being_delete_items)

        if len(webcasts) >= 1 and play:
            del self.select_rows[:]
            self.queue_draw()
            self.set_highlight_webcast(webcasts[0])
            Player.play_new(self.highlight_item.get_webcast(), seek=self.highlight_item.get_webcast().get("seek", 0))
            if Player.get_source() != self:
                Player.set_source(self)

    def set_highlight_webcast(self, webcast):
        if not webcast: return
        webcast_item = WebcastListItem(webcast)
        if webcast_item in self.items:
            self.set_highlight(self.items[self.items.index(webcast_item)])
            self.visible_highlight()
            self.queue_draw()
        else:
            self.add_webcasts(webcast, pos=0)
            self.set_highlight(self.items[0])
            self.visible_highlight()
            self.queue_draw()

    def emit_add_signal(self):
        self.emit("begin-add-items")

    def load(self):    
        try:
            webcast_uris = utils.load_db(self.preview_db_file)
        except:    
            webcast_uris = None
            
        if webcast_uris:    
            webcasts = [ WebcastDB.get_song(uri) for uri in webcast_uris]
        else:    
            webcasts = None
            
        if webcasts:    
            self.add_webcasts(webcasts)
            
    def save(self):        
        songs = self.get_webcasts()
        uris = [ song.get("uri") for song in songs if song.get("uri")]
        utils.save_db(uris, self.preview_db_file)

class MultiDragWebcastView(ListView):

    def __init__(self):
        targets = [("text/deepin-webcasts", gtk.TARGET_SAME_APP, 1),]
        ListView.__init__(self, drag_data=(targets, gtk.gdk.ACTION_COPY, 1))
        self.set_expand_column(1)

        self.connect("drag-data-get", self.__on_drag_data_get)
        self.connect("double-click-item", self.__on_double_click_item)
        self.connect("single-click-item", self.__on_single_click_item)
        self.connect("right-press-items", self.__on_right_press_items)

    def draw_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, "layoutMiddle")

    def draw_item_hover(self, cr, x, y, w, h):
        draw_single_mask(cr, x + 1, y, w - 2, h, "globalItemHover")

    def draw_item_select(self, cr, x, y, w, h):
        draw_single_mask(cr, x + 1, y, w - 2, h, "globalItemSelect")

    def draw_item_highlight(self, cr, x, y, w, h):
        draw_single_mask(cr, x + 1, y, w - 2, h, "globalItemHighlight")

    def get_selected_webcasts(self):
        webcasts = None
        if len(self.select_rows) > 0:
            webcasts = [ self.items[index].webcast for index in self.select_rows ]
        return webcasts

    def __on_drag_data_get(self, widget, context, selection, info, timestamp):
        webcasts = self.get_selected_webcasts()
        if not webcasts: return
        str_items = str([webcast.get_dict() for webcast in webcasts])
        selection.set("text/deepin-webcasts", 8, str_items)

    def __on_double_click_item(self, widget, item, column, x, y):
        Dispatcher.play_webcast(item.webcast)

    def __on_right_press_items(self, widget, x, y, current_item, select_items):
        if current_item and select_items:
            if len(select_items) > 1:
                items = [
                    (None, _("Add to Playlist"), lambda : self.emit_to_playlist(select_items)),                    
                    # (None, _("Delete"), None),
                    ]
            else:    
                items = [
                    (None, _("Play"), lambda : Dispatcher.play_webcast(current_item.webcast)),
                    (None, _("Add to Playlist"), lambda : self.emit_to_playlist([current_item])),
                    self.get_favorite_menu_item(current_item),
                    # None,
                    # (None, _("Clear List"), None),
                    ]
            Menu(items, True).show((int(x), int(y)))
            
    def emit_to_playlist(self, items):        
        webcasts = [ item.webcast for item in items ]
        Dispatcher.emit("add-webcasts", webcasts)

    def __on_single_click_item(self, widget, item, column, x, y):
        if column == 2:
            self.toggle_item_collected(item)
            
    def toggle_item_collected(self, item):        
        if item:
            song = item.webcast
            if song.get("collected", False):
                collected = False
            else:
                collected = True
            WebcastDB.set_property(song, {"collected": collected})
            item.update_webcast(song)
        
            
    def get_favorite_menu_item(self, item):        
        song = item.webcast        
        if song.get("collected", False):
            menu_name = _("Remove from Favorites")
        else:
            menu_name = _("Add to Favorites")
        return (None, menu_name, lambda : self.toggle_item_collected(item))    

    def get_scrolled_window(self):
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(self)
        return scrolled_window

    def add_webcasts(self, songs):
        items = [ WebcastListItem(song) for song in songs]
        self.add_items(items)
        
    def get_webcasts(self):    
        songs = [item.webcast for item in self.items ]
        return songs
    
    def get_webcast_item(self, song):
        for item in self.items:
            if song == item.webcast:
                return item
        return None    
    
    def get_search_songs(self, keyword):
        self.clear()
        all_songs = WebcastDB.get_all_songs()
        result_songs = filter(lambda song: keyword.lower().replace(" ", "") in song.get("search", ""),
                              all_songs)
        
        return result_songs


class WebcastIconView(IconView):

    def __init__(self, padding_x=14, padding_y=16):
        IconView.__init__(self, padding_x=padding_x, padding_y=padding_y)

        targets = [("text/deepin-webcasts", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        self.connect("drag-data-get", self.__on_drag_data_get)

    def __on_drag_data_get(self, widget, context, selection, info, timestamp):
        # item = widget.highlight_item
        # if not item: return
        # channel_info = str([ item.chl ])
        # selection.set("text/deepin-radios", 8, channel_info)
        pass

    def add_webcast_items(self, items, is_composited=False):
        webcast_items = [ WebcastIconItem(item, is_composited) for item in items]
        if webcast_items:
            self.add_items(webcast_items)
            
    def add_composite_items(self, items, is_composited=True):        
        composite_items = [ CompositeIconItem(item[0], item[1], is_composited) for item in items]
        if composite_items:
            self.add_items(composite_items)

    def draw_mask(self, cr, x, y, w, h):
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")
        return False

    def get_scrolled_window(self):
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(self)
        return scrolled_window

    
class WebcastSearchView(gtk.VBox):    
    
    def __init__(self, source_tab):
        gtk.VBox.__init__(self)
        
        self.source_tab = source_tab
        
        self.webcast_view = MultiDragWebcastView()
        self.webcast_view.connect_after("double-click-item", self.__on_double_click_item) 
        self.webcast_view_sw = self.webcast_view.get_scrolled_window()
        
        self.search_prompt = SearchPrompt(_("Radio"))
        self.add(self.webcast_view_sw)
        
    def start_search_songs(self, keyword):    
        songs = self.webcast_view.get_search_songs(keyword)
        if songs:
            self.webcast_view.add_webcasts(songs)
            switch_tab(self, self.webcast_view_sw)
        else:    
            self.search_prompt.update_keyword(keyword)
            switch_tab(self, self.search_prompt)
            
    def __on_double_click_item(self, *args):        
        Dispatcher.emit("switch-source", self.source_tab)
