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
from collections import OrderedDict, namedtuple
from dtk.ui.draw import draw_text
from dtk.ui.utils import (get_content_size, color_hex_to_cairo,
                          cairo_disable_antialias, set_cursor)
from dtk.ui.scrolled_window import ScrolledWindow

from widget.skin import app_theme
from widget.radio_view import RadioIconView, TAG_GENRE
from widget.ui_utils import draw_alpha_mask
from constant import DEFAULT_FONT_SIZE
import utils


GENRE_DATA = OrderedDict({"摇滚" : "335", "古典" : "326", "爵士" : "327",
                          "民谣/乡村" : "337", "流行" : "331", "电子" : "325",
                          "原声配乐" : "328", "轻音乐" : "332", "说唱" : "334",
                          "雷鬼" : "330", "拉丁" : "329", "世界音乐" : "333",
                          "布鲁斯" : "324", "疯克/灵歌/R&B" : "336"})

class GenreItem(object):    
    
    def __init__(self, title, is_select=False):
        self.hover_flag = False
        self.highlight_flag = False
        self.title = title
        self.underline_flag = False
        self.redraw_request_callback = None
        self.is_select = is_select
        
    def get_title(self):    
        return self.title
    
    def emit_redraw_request(self):
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
    
    def render(self, cr, rect):    
        if self.hover_flag or self.is_select:
            color = app_theme.get_color("simpleItemSelect").get_color()            
        else:    
            color = "#333333"
        draw_text(cr, utils.xmlescape(self.title), rect.x, rect.y, rect.width, rect.height,
                  text_color=color, underline=self.underline_flag)
        
    def get_size(self):    
        return get_content_size(self.title, DEFAULT_FONT_SIZE)
        
    def motion_notify(self, x, y):    
        if self.is_select:
            self.underline_flag = False
        else:    
            self.underline_flag = True
            
        self.hover_flag = True
        self.emit_redraw_request()
        
    def lost_focus(self):    
        self.hover_flag = False
        self.underline_flag = False
        self.emit_redraw_request()
        
    def button_press(self):    
        self.underline_flag = False
        self.is_select = True
        self.emit_redraw_request()
        
    def clear_selected_status(self):    
        self.is_select = False
        self.emit_redraw_request()
        
class GenreSideBox(gtk.EventBox):        
    
    __gsignals__ = { "press-item" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))}
    
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        # Init signals.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.connect("expose-event", self.on_expose_event)
        self.connect("motion-notify-event", self.on_motion_notify_event)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("button-release-event", self.on_button_release_event)
        self.connect("leave-notify-event", self.on_leave_notify_event)
        self.connect("size-allocate", self.on_size_allocate)
        
        # Init.
        self.padding_x = 10
        self.padding_y = 5
        self.default_height = 70
        self.min_height = 40
        self.oneline_flag = False
        self.set_size_request(-1, self.default_height)
        
        self.separate_text = " | "
        self.separate_width, self.separate_height = get_content_size(self.separate_text)
                
        self.coords = {}
        self.item_range = namedtuple("coord", "start_x end_x start_y end_y")
        self.items = []
        self.block_height = 5
        self.item_interval_height = 12
        self.hover_item = None
        
        self.redraw_request_list = []
        self.redraw_delay = 100 # update redraw item delay, milliseconds
        gobject.timeout_add(self.redraw_delay, self.update_redraw_request_list)
        
        self.item_total_width = self.padding_x
        
        # Init Item.
        for index, key in enumerate(GENRE_DATA.keys()):
            if index == 0:
                self.select_item = item = GenreItem(key, True)
            else:    
                item = GenreItem(key)
            item.redraw_request_callback = self.redraw_request
            self.item_total_width += item.get_size()[0] + self.separate_width
            self.items.append(item)
            
    def redraw_request(self, item):
        if item not in self.redraw_request_list:
            self.redraw_request_list.append(item)

    def update_redraw_request_list(self):
        if len(self.redraw_request_list) > 0:
            self.queue_draw()
        
        # Clear redraw request list.
        self.redraw_request_list = []
        return True
        
    def on_size_allocate(self, widget, rect):    
        if rect.width >= self.item_total_width:
            self.oneline_flag = True
        else:    
            self.oneline_flag = False

        
    def on_expose_event(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height ,"layoutRight")
        
        with cairo_disable_antialias(cr):
            cr.set_source_rgb(*color_hex_to_cairo("#444444"))            
            cr.set_line_width(1)
            cr.set_dash([4.0, 4.0])
            if self.oneline_flag:
                dash_height = self.min_height
            else:    
                dash_height = self.default_height
                
            cr.move_to(rect.x, rect.y + dash_height - 1)
            cr.rel_line_to(rect.width, 0)
            cr.stroke()
        

        rect.y += self.block_height        
        start_x, start_y = self.padding_x, self.padding_y
        
        for index, item in enumerate(self.items):
            item_width, item_height = item.get_size()
            if rect.width - start_x < item_width + self.separate_width:
                start_y += item_height + self.item_interval_height
                start_x = self.padding_x
                
            item.render(cr, gtk.gdk.Rectangle(rect.x + start_x, rect.y + start_y,
                                              item_width, item_height))    
            
            self.coords[index] = self.item_range(start_x, start_x + item_width + 6,
                                                 start_y, start_y + item_height + 6,)
            start_x += item_width            
            draw_text(cr, self.separate_text, rect.x + start_x, rect.y + start_y, self.separate_width, 
                      self.separate_height, text_color=app_theme.get_color("labelText").get_color())
            start_x += self.separate_width
            
        return True    
    
    
    def on_motion_notify_event(self, widget, event):
        if not self.coords:
            return 
        for key, coord in self.coords.iteritems():
            if coord.start_x <= event.x <= coord.end_x and coord.start_y <= event.y <= coord.end_y:
                set_cursor(widget, gtk.gdk.HAND2)
                current_item = self.items[key]
                if self.hover_item and current_item != self.hover_item:
                    self.hover_item.lost_focus()
                current_item.motion_notify(event.x, event.y)    
                self.hover_item = current_item
                break
        else:        
            set_cursor(widget, None)
            if self.hover_item:    
                self.hover_item.lost_focus()
                self.hover_item = None
            
    def on_button_press_event(self, widget, event):        
        if self.hover_item:
            if self.hover_item != self.select_item:
                self.hover_item.button_press()
                self.emit("press-item", self.hover_item.title)
                self.select_item.clear_selected_status()
                self.select_item = self.hover_item
                
    def on_leave_notify_event(self, widget, event):            
        set_cursor(widget, None)
        if self.hover_item:    
            self.hover_item.lost_focus()
            self.hover_item = None
        
    def on_button_release_event(self, widget, event):
        pass
    
    def get_key(self):
        return self.select_item.title
    
class GenrePage(gtk.VBox):    
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        self.genre_side_height = 70
        self.genre_side = GenreSideBox()
        self.genre_side.connect("press-item", self.on_genreside_press_item)
        self.genre_side.set_size_request(-1, self.genre_side_height)
        self.channels_box = gtk.VBox()
        init_genre_id = self.genre_side.get_key()
        
        # Genres view
        self.channels_view, self.channels_sw = self.get_radio_icon_view()
        self.channels_view.set_genre_id(GENRE_DATA.get(init_genre_id, "335"))
        self.channels_view.start_fetch_channels()
        self.channels_box.add(self.channels_sw)

        self.pack_start(self.genre_side, False, True)
        self.pack_start(self.channels_box, True, True)
        
    def get_radio_icon_view(self):
        icon_view = RadioIconView(tag=TAG_GENRE, padding_x=0, padding_y=10)
        scrolled_window = ScrolledWindow()
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(icon_view)
        return icon_view, scrolled_window
    
    def on_genreside_press_item(self, widget, key):
        self.channels_view.clear_items()
        self.channels_view.set_genre_id(GENRE_DATA.get(key, "335"))
        self.channels_view.start_fetch_channels()
