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

from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.iconview import IconView
from dtk.ui.threads import post_gui
from dtk.ui.thread_pool import MissionThreadPool

from widget.slide_switcher import SlideSwitcher
from widget.tab_switcher import TabSwitcher
from widget.ui_utils  import  draw_alpha_mask, switch_tab
from widget.radio_item import RecommendItem
from widget.skin import app_theme
from nls import _
from doubanfm import fmlib

import utils

cover_thread_pool = MissionThreadPool(5)
cover_thread_pool.start()

class HomePage(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        self.connect("expose-event", self.on_expose_event)
        self.set_spacing(5)        
        
        # home slider.
        self.home_slider = SlideSwitcher()
        self.home_slider.set_size_request(-1, 200)
        
        # recommmend tab switcher.
        self.recommend_tab = TabSwitcher([_("热门兆赫"), _("人气兆赫")])
        self.recommend_tab.connect("tab-switch-start", lambda switcher, tab_index: self.switch_recommend_view(tab_index))
        self.recommend_tab.connect("click-current-tab", lambda switcher, tab_index: self.on_click_recommend_tab(tab_index))

        # Init recommend view.
        self.hot_recommend_view, self.hot_recommend_sw = self.get_icon_view(padding_y=5)
        self.fast_recommend_view, self.fast_recommend_sw = self.get_icon_view(padding_y=5)
        
        # Use switch recommend view.
        self.recommend_view_box = gtk.VBox()        
        self.recommend_view_box.add(self.hot_recommend_sw)
        
        self.pack_start(self.home_slider, False, True)
        self.pack_start(self.recommend_tab, False, True)
        self.pack_start(self.recommend_view_box, True, True)
        
        # load fm data
        self.load_fm_data()
        
    def load_fm_data(self):    
        utils.ThreadFetch(
            fetch_funcs=(self.fetch_hot_channels, ()),
            success_funcs=(self.load_hot_channels, ())).start()
        
        utils.ThreadFetch(
            fetch_funcs=(self.fetch_fast_channels, ()),
            success_funcs=(self.load_fast_channels, ())).start()
        
    def fetch_hot_channels(self):    
        ret = fmlib.get_hot_chls(limit=8)
        return  ret.get("data", {}).get("channels", [])
    
    def fetch_fast_channels(self):
        ret = fmlib.get_uptrending_chls(limit=8)
        return  ret.get("data", {}).get("channels", [])
        
    @post_gui
    def load_hot_channels(self, hot_channels):
        hot_items = []
        thread_items = []
        for hot_chl in hot_channels:
            recommend_item = RecommendItem(hot_chl)
            if not recommend_item.is_loaded_cover:
                thread_items.append(recommend_item)
            hot_items.append(recommend_item)    
            
        if thread_items:    
            cover_thread_pool.add_missions(thread_items)
        self.hot_recommend_view.clear()    
        self.hot_recommend_view.add_items(hot_items)    
        
    @post_gui    
    def load_fast_channels(self, fast_channels):    
        hot_items = []
        thread_items = []
        for fast_chl in fast_channels:
            recommend_item = RecommendItem(fast_chl)
            if not recommend_item.is_loaded_cover:
                thread_items.append(recommend_item)
            hot_items.append(recommend_item)    
            
        if thread_items:    
            cover_thread_pool.add_missions(thread_items)
            
        self.fast_recommend_view.clear()    
        self.fast_recommend_view.add_items(hot_items)    
        
        
    def switch_recommend_view(self, tab_index):
        if tab_index == 0:
            switch_tab(self.recommend_view_box, self.hot_recommend_sw)
        if tab_index == 1:    
            switch_tab(self.recommend_view_box, self.fast_recommend_sw)
    
    def on_click_recommend_tab(self, tab_index):
        pass
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutLast")
        
    def get_icon_view(self, padding_x=0, padding_y=0):    
        icon_view = IconView(padding_x, padding_y)
        # targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2)]
        # icon_view.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_COPY)
        # icon_view.connect("drag-data-get", self.__on_drag_data_get) 
        # icon_view.connect("double-click-item", self.__on_double_click_item)
        # icon_view.connect("single-click-item", self.__on_single_click_item)
        icon_view.draw_mask  = self.on_iconview_draw_mask
        scrolled_window = ScrolledWindow()
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(icon_view)
        return icon_view, scrolled_window
    
    def on_iconview_draw_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, "layoutLast")
