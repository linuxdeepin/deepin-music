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
import os 

from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.listview import ListView
from dtk.ui.new_treeview import TreeView, TreeItem
from dtk.ui.draw import draw_vlinear, draw_pixbuf, draw_text
from dtk.ui.utils import get_content_size

from widget.outlookbar import WebcastsBar
from widget.ui_utils import draw_single_mask, draw_alpha_mask, render_item_text, switch_tab
from widget.skin import app_theme
from widget.webcast_view import WebcastView
from collections import OrderedDict
from constant import DEFAULT_FONT_SIZE
from webcasts import WebcastsDB
from song import Song
from nls import _

class WebcastTreeItem(TreeItem):
    
    def __init__(self, column_index=0):
        TreeItem.__init__(self)
        self.column_index = column_index
        self.side_padding = 5
        self.webcast_icon = app_theme.get_pixbuf("filter/album_normal.png")
        
    def get_height(self):    
        pass
    
    def get_column_widths(self):
        pass
    
    def get_column_renders(self):
        pass
    
    def unselect(self):
        self.is_select = False
        if self.redraw_request_callback:
            self.radraw_request_callback(self)
            
    def select(self):        
        self.is_select = True
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def render_webcast_icon(self, cr, rect):        
        # Draw select background.
        if self.is_select:
            draw_single_mask(cr, rect.x, rect.y, rect.w, rect.h, "simpleItemSelect")
            
        draw_pixbuf(cr, self.webcast_icon, rect.x + self.side_padding, 
                    rect.y + (rect.height - self.webcast_icon.get_height()) / 2)
        
    def render_webcast_name(self, cr, rect):    
        if self.is_select:
            draw_single_mask(cr, rect.x, rect.y, rect.w, rect.h, "simpleItemSelect")
            
        draw_text(cr, self.name, rect.x, rect.y, rect.w, rect.h)    
        
class WebcastListItem(gobject.GObject):
    
    __gsignals__ = {"redraw-request" : ( gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, tags):
        gobject.GObject.__init__(self)
        
        self.webcast = Song()
        self.webcast.init_from_dict(tags)
        self.webcast.set_type("webcast")
        self.index = None
        self.webcast_normal_pixbuf = app_theme.get_pixbuf("webcast/webcast_normal.png").get_pixbuf()
        self.webcast_press_pixbuf = app_theme.get_pixbuf("webcast/webcast_press.png").get_pixbuf()
        self.collect_normal_pixbuf = app_theme.get_pixbuf("webcast/collect_normal.png").get_pixbuf()
        self.collect_press_pixbuf = app_theme.get_pixbuf("webcast/collect_press.png").get_pixbuf()
        
        self.__update_size()
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
        
    def __update_size(self):    
        self.title = self.webcast.get_str("title")
        self.is_collect = WebcastsDB.is_collect(self.webcast.get("uri"))
        
        self.webcast_icon_padding_x = 10
        self.webcast_icon_padding_y = 5
        self.webcast_icon_w = self.webcast_normal_pixbuf.get_width()
        self.webcast_icon_h = self.webcast_normal_pixbuf.get_height()
        
        self.title_padding_x = 5
        self.title_padding_y = 5
        self.title_w, self.title_h = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
        self.collect_icon_padding_x = 2
        self.collect_icon_padding_y = 5
        self.collect_icon_w = self.collect_normal_pixbuf.get_width()
        self.collect_icon_h = self.collect_normal_pixbuf.get_height()
        
    def render_webcast_icon(self, cr, rect, in_select, in_highlight):    
        icon_x = rect.x + self.webcast_icon_padding_x 
        icon_y = rect.y + (rect.height - self.webcast_icon_h) / 2
        if in_select:
            pixbuf = self.webcast_press_pixbuf
        else:    
            pixbuf = self.webcast_normal_pixbuf
        draw_pixbuf(cr, pixbuf, icon_x, icon_y)
        
    def render_title(self, cr, rect, in_select, in_highlight):    
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_item_text(cr, self.title, rect, in_select, in_highlight)
        
    def render_collect_icon(self, cr, rect, in_select, in_highlight):    
        icon_y = rect.y + (rect.height - self.collect_icon_h) / 2
        rect.x += self.collect_icon_padding_x
        if self.is_collect:
            pixbuf = self.collect_press_pixbuf
        else:    
            pixbuf = self.collect_normal_pixbuf
        draw_pixbuf(cr, pixbuf, rect.x , icon_y)
        
    def render_block(self, cr, rect, in_select, in_highlight):    
        pass
    
    def get_column_sizes(self):
        return [
            (self.webcast_icon_w + self.webcast_icon_padding_x * 2, self.webcast_icon_h + self.webcast_icon_padding_y * 2),
            (360, self.title_h + self.title_padding_y * 2),
            (self.collect_icon_w + self.collect_icon_padding_x * 2, self.collect_icon_h + self.collect_icon_padding_y * 2),
            (50, 1)
            ]
    
    def get_renders(self):
        return (self.render_webcast_icon, self.render_title, self.render_collect_icon, self.render_block)
    
    def toggle_is_collect(self):
        if self.is_collect:
            self.is_collect = False
        else:    
            self.is_collect = True
        self.emit_redraw_request()
        
        
class WebcastsManager(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Init data.
        self.source_data = OrderedDict()
        self.source_data["internal"] = _("国内广播")
        self.source_data["foreign"]  = _("国外广播")
        self.source_data["network"] = _("网络广播")
        self.source_data["genres"] = _("流派广播")
        self.source_data["music"]  = _("音乐广播")
        self.source_data["finance"] = _("财经广播")
        self.source_data["sports"] = _("体育广播")
        
        # Init sourcebar
        self.__init_sourcebar()
        
        # Init webcasts view.
        self.source_view, self.source_sw = self.get_webcasts_view()
        self.collect_view, self.collect_sw = self.get_webcasts_view()
        # self.custom_view, custom_view_sw = self.get_webcasts_view()
        
        self.source_view.connect("single-click-item", self.on_source_view_single_click_item)
        
        items = WebcastsDB.get_items("internal", "网络电台")
        self.source_view.add_items([WebcastListItem(tag) for tag in items])        
        
        
        # Used to switch categroy view.
        self.switch_view_box = gtk.VBox()
        self.switch_view_box.add(self.source_sw)
        switch_view_align = gtk.Alignment()
        switch_view_align.set_padding(0, 0, 0, 2)
        switch_view_align.set(1, 1, 1, 1)
        switch_view_align.add(self.switch_view_box)
        
        body_box = gtk.HBox()
        body_box.pack_start(self.sourcebar, False, False)
        body_box.pack_start(switch_view_align, True, True)
        
        self.add(body_box)
        self.show_all()
        
    def __init_sourcebar(self):
        items = []
        for key, value in self.source_data.items():
            items.append((value, None))
        items.append((_("我的收藏"), lambda : switch_tab(self.switch_view_box, self.collect_sw)))    
        items.append((_("自定义"), None))    
            
        self.sourcebar = WebcastsBar(items)        
        self.sourcebar.connect("expose-event", self.on_sourcebar_expose_event)
        
    def get_webcasts_view(self):    
        webcast_view = WebcastView()
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(webcast_view)
        return webcast_view, scrolled_window

    def on_sourcebar_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutRight")
        return False
    
    def on_source_view_single_click_item(self, widget, item, column, x, y):
        if column == 2:
            item.toggle_is_collect()
            
        # add item to collect_view.    
        if item.is_collect:    
            self.collect_view.add_items([item])
