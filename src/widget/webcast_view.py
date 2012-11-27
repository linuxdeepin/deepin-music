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
import pango
from dtk.ui.listview import ListView
from dtk.ui.draw import draw_pixbuf, draw_text


from widget.ui_utils import draw_single_mask, draw_alpha_mask
from widget.skin import app_theme
from helper import Dispatcher
from player import Player
from song import Song

import utils
    
class WebcastItem(gobject.GObject):    
    
    __gsignals__ = {"redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }
    
    def __init__(self, webcast):
        gobject.GObject.__init__(self)
        self.index = 0
        self.side_padding = 5
        self.item_height = 31
        self.webcast = webcast
        self.item_width = 121
        self.star_normal_pixbuf = app_theme.get_pixbuf("webcast/collect_normal.png").get_pixbuf()
        self.star_press_pixbuf = app_theme.get_pixbuf("webcast/collect_press.png").get_pixbuf()
        self.update_sizes()
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
        
    def emit_redraw_request(self):
        self.emit("redraw-request")
    
    def get_column_sizes(self):
        return [
            (100, 32),
            (self.star_icon_w + self.star_icon_padding_x * 2, 32),
            (20, 32)
            ]
    def update_sizes(self):
        self.star_icon_padding_x = 2
        self.star_icon_padding_y = 5
        self.star_icon_w = self.star_normal_pixbuf.get_width()
        self.star_icon_h = self.star_normal_pixbuf.get_height()
    
    def render_content(self, cr, rect, in_select, in_highlight):    
        if in_select or in_highlight:
            text_color = "#ffffff"
            icon_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_press.png").get_pixbuf()            
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            icon_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_normal.png").get_pixbuf()            
            
        icon_y = rect.y + (rect.height - icon_pixbuf.get_height())  / 2 
        padding_x = 10
        draw_pixbuf(cr, icon_pixbuf, rect.x + padding_x, icon_y)    
            
        draw_text(cr, utils.xmlescape(self.webcast.get_str("title")), rect.x + icon_pixbuf.get_width() + padding_x * 2, 
                  rect.y, rect.width - icon_pixbuf.get_width() - padding_x * 2, rect.height, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT, text_size=9)    
        
    def render_star(self, cr, rect, in_select, in_highlight):
        icon_y = rect.y + (rect.height - self.star_icon_h) / 2
        rect.x += self.star_icon_padding_x
        if self.webcast.get("collect", False):
            pixbuf = self.star_press_pixbuf
        else:    
            pixbuf = self.star_normal_pixbuf
        draw_pixbuf(cr, pixbuf, rect.x , icon_y)
        
    def get_renders(self):    
        return [self.render_content, self.render_star, self.render_block]
    
    def render_block(self, cr, rect, in_select, in_highlight):
        pass
        
    def get_webcast(self):
        return self.webcast
    
    def __hash__(self):
        return hash(self.webcast.get("uri"))
    
    def __repr__(self):
        return "<SongItem %s>" % self.webcast.get("uri")
    
    def __cmp__(self, other_item):
        if not other_item:
            return -1
        try:
            return cmp(self.webcast, other_item.get_webcast())
        except AttributeError: return -1
        
    def __eq__(self, other_item):    
        try:
            return self.webcast == other_item.get_webcast()
        except:
            return False
    
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
        self.connect("double-click-item", self.on_double_click_item)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("delete-select-items", self.try_emit_empty_signal)
        
        self.set_expand_column(0)
        Dispatcher.connect("play-webcast", self.on_dispatcher_play_webcast)
        
        
    def draw_mask(self, cr, x, y, width, height):    
        draw_alpha_mask(cr, x, y, width, height, "layoutLeft")
        
    def try_emit_empty_signal(self, widget, items):    
        if len(self.items) <= 0:
            self.emit("empty-items")
        
    def on_double_click_item(self, widget, item, column, x, y):
        if item:
            self.set_highlight(item)
            Player.play_new(item.get_webcast(), seek=item.get_webcast().get("seek", 0))
            if Player.get_source() != self:
                Player.set_source(self)
                
    def on_button_press_event(self, widget, event):            
        ''' TODO: Popup Menu. '''
        pass
    
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
        self.add_webcasts([webcast], play=True)
        
    def add_webcasts(self, webcasts, pos=None, sort=False, play=False):    
        if not webcasts:
            return 
        if not isinstance(webcasts, (list, tuple, set)):
            webcasts = [ webcasts ]
            
        webcast_items = [ WebcastItem(webcast) for webcast in webcasts if webcast not in self.get_webcasts()]    
        
        if webcast_items:
            if not self.items:
                self.emit_add_signal()
            self.add_items(webcast_items, pos, sort)    
            
        if len(webcasts) >= 1 and play:
            del self.select_rows[:]
            self.queue_draw()
            self.set_highlight_webcast(webcasts[0])
            Player.play_new(self.highlight_item.get_webcast(), seek=self.highlight_item.get_webcast().get("seek", 0))
            if Player.get_source() != self:
                Player.set_source(self)
            
    def set_highlight_webcast(self, webcast):        
        if not webcast: return 
        if WebcastItem(webcast) in self.items:
            self.set_highlight(self.items[self.items.index(WebcastItem(webcast))])
            self.visible_highlight()
            self.queue_draw()
            
    def emit_add_signal(self):            
        self.emit("begin-add-items")

        
class MultiDragWebcastView(ListView):        
    
    def __init__(self):
        targets = [("text/deepin-webcasts", gtk.TARGET_SAME_APP, 1),]        
        ListView.__init__(self, drag_data=(targets, gtk.gdk.ACTION_COPY, 1))
        self.set_expand_column(1)
    
        self.connect("drag-data-get", self.__on_drag_data_get) 
        self.connect("double-click-item", self.__on_double_click_item)
        self.connect("right-press-items", self.__on_right_press_items)
        
    def draw_mask(self, cr, x, y, width, height):            
        draw_alpha_mask(cr, x, y, width, height, "layoutMiddle")
        
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
    
    def __on_right_press_items(self, widget, x, y, item, select_items):
        pass
