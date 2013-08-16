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
import copy
import random

from dtk.ui.threads import post_gui

from widget.slide_switcher import SlideSwitcher
from widget.page_switcher import PageSwitcher
from widget.ui_utils  import switch_tab
from widget.ui import NetworkConnectTimeout, AutoLabel
from radio_view import RadioIconView, TAG_HOT, TAG_FAST
from nls import _
from posterlib import fmlib
import utils


class NetworkRadioView(gtk.VBox):
    
    def __init__(self, tag, padding_x=0, padding_y=0):
        gtk.VBox.__init__(self)
        
        self.radio_view, self.radio_view_sw = self.get_icon_view(tag=tag, padding_x=padding_x, padding_y=padding_y)
        self.network_connect_timeout = NetworkConnectTimeout(self.radio_view.start_fetch_channels)
        prompt_label = AutoLabel("正在加载数据...")
        self.radio_view.fetch_failed = lambda : switch_tab(self, self.network_connect_timeout)
        self.radio_view.fetch_successed = lambda : switch_tab(self, self.radio_view_sw)
        self.add(prompt_label)
            
    def get_icon_view(self, tag, padding_x=0, padding_y=0):    
        icon_view =RadioIconView(tag=tag, limit=8, has_add=False, fetch_add_item=False, 
                                 padding_x=padding_x, padding_y=padding_y)
        scrolled_window = icon_view.get_scrolled_window()
        return icon_view, scrolled_window
    
    def start_fetch_channels(self):
        self.radio_view.clear_items(False)
        self.radio_view.start_fetch_channels()
        

class HomePage(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        self.set_spacing(5)        
        
        # home slider.
        self.home_slider = SlideSwitcher()
        self.home_slider.set_size_request(-1, 200)
        
        # recommmend tab switcher.
        self.recommend_tab = PageSwitcher([_("Hot MHz"), _("Pop MHz")])
        self.recommend_tab.connect("tab-switch-start", lambda switcher, tab_index: self.switch_recommend_view(tab_index))

        # Init recommend view.
        self.hot_recommend_box = NetworkRadioView(tag=TAG_HOT, padding_y=5)
        self.fast_recommend_box = NetworkRadioView(tag=TAG_FAST, padding_y=5)
        
        # Use switch recommend view.
        self.recommend_page_box = gtk.VBox()        
        self.recommend_page_box.add(self.hot_recommend_box)
        
        self.pack_start(self.home_slider, False, True)
        self.pack_start(self.recommend_tab, False, True)
        self.pack_start(self.recommend_page_box, True, True)
        
        # Init data
        self.banner_thread_id = 0
        
    def switch_recommend_view(self, tab_index):
        if tab_index == 0:
            switch_tab(self.recommend_page_box, self.hot_recommend_box)
        if tab_index == 1:    
            switch_tab(self.recommend_page_box, self.fast_recommend_box)
        
    
    def start_fetch_channels(self):
        self.set_banner_channels()
        self.hot_recommend_box.start_fetch_channels()
        self.fast_recommend_box.start_fetch_channels()
        
    def set_banner_channels(self):    
        self.banner_thread_id += 1
        banner_thread_id = copy.deepcopy(self.banner_thread_id)
        utils.ThreadFetch(
            fetch_funcs=(self.fetch_banner_channels, ()),
            success_funcs=(self.load_banner_channels, (banner_thread_id,)),
            ).start()
        
    def fetch_banner_channels(self):    
        start = random.randrange(0, 25, 5)
        ret = fmlib.get_hot_chls(start=start, limit=5)
        return ret.get("data", {}).get("channels", [])
    
    @post_gui
    def load_banner_channels(self, channels, thread_id):
        if thread_id != self.banner_thread_id:
            return 
        if not channels:
            return
        
        # print channels
        self.home_slider.set_infos(channels)
        
        
