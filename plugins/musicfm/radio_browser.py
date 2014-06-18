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

from dtk.ui.treeview import TreeView
from dtk.ui.paned import HPaned
from deepin_utils.net import is_network_connected

from radio_item import CategroyTreeItem
from radio_home_page import HomePage
from radio_genre_page import GenrePage
from radio_view import RadioIconView, TAG_HOT, TAG_FAST

from widget.skin import app_theme
from widget.ui import NetworkConnectFailed
from widget.ui_utils import draw_alpha_mask, switch_tab
from nls import _

class RadioBrowser(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Init radiobar.
        self.__init_radiobar()
        
        # is loaded.
        self.homepage_load_flag = False
        self.home_page = HomePage()
        
        self.hot_page_view, self.hot_page_sw = self.get_radio_icon_view(TAG_HOT, 20)
        self.fast_page_view, self.fast_page_sw = self.get_radio_icon_view(TAG_FAST, 20)
        
        # Genres page
        self.genre_page = GenrePage()
        
        self.page_box = gtk.VBox()
        self.page_box.add(self.home_page)
        
        self.body_paned = HPaned(handle_color=app_theme.get_color("panedHandler"))
        self.body_paned.add1(self.radiobar)
        self.body_paned.add2(self.page_box)
        self.network_failed_box = NetworkConnectFailed(self.check_network_connection)
        self.check_network_connection(auto=True)
        
        
    def check_network_connection(self, auto=False):    
        if is_network_connected():
            switch_tab(self, self.body_paned)
            if not auto:
                self.start_fetch_channels()
        else:    
            switch_tab(self, self.network_failed_box)
            
    def __init_radiobar(self):    
        self.radiobar = TreeView(enable_drag_drop=False, enable_multiple_select=False)
        items = []
        items.append(CategroyTreeItem(_("Home"), "home",  lambda : switch_tab(self.page_box, self.home_page)))
        items.append(CategroyTreeItem(_("Hot MHz"), "hot", lambda : switch_tab(self.page_box, self.hot_page_sw)))
        items.append(CategroyTreeItem(_("Genres MHz"), "genre",  lambda : switch_tab(self.page_box, self.genre_page)))
        items.append(CategroyTreeItem(_("Pop MHz"), "up", lambda : switch_tab(self.page_box, self.fast_page_sw)))        
        self.radiobar.add_items(items)
        self.radiobar.select_items([self.radiobar.visible_items[0]])
        self.radiobar.set_size_request(121, -1)
        self.radiobar.draw_mask = self.on_radiobar_draw_mask        
        
    def on_radiobar_draw_mask(self, cr, x, y, w, h):    
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")
    
    def get_radio_icon_view(self, tag, limit=10, padding_x=0, padding_y=10):
        icon_view = RadioIconView(tag=tag, limit=limit, padding_x=padding_x, padding_y=padding_y)
        scrolled_window = icon_view.get_scrolled_window()
        return icon_view, scrolled_window
    
    def start_fetch_channels(self):
        self.fast_page_view.clear_items()
        self.hot_page_view.clear_items()
        self.home_page.start_fetch_channels()
        self.hot_page_view.start_fetch_channels()
        self.fast_page_view.start_fetch_channels()
        self.genre_page.start_fetch_channels()
