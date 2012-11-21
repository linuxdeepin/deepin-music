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

from dtk.ui.new_treeview import TreeView
from dtk.ui.paned import HPaned
from dtk.ui.scrolled_window import ScrolledWindow

from widget.radio_item import CategroyTreeItem
from widget.skin import app_theme
from widget.radio_home_page import HomePage
from widget.radio_genre_page import GenrePage
from widget.radio_view import RadioIconView, TAG_HOT, TAG_FAST
from widget.ui_utils import draw_line, draw_alpha_mask, switch_tab
from nls import _

class RadioBrowser(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Init radiobar.
        self.__init_radiobar()
        
        
        # is loaded.
        self.homepage_load_flag = False
        self.home_page = HomePage()
        
        self.hot_page_view, self.hot_page_sw = self.get_radio_icon_view(TAG_HOT, 0, 10)
        self.fast_page_view, self.fast_page_sw = self.get_radio_icon_view(TAG_FAST, 0, 10)
        
        self.hot_page_view.start_fetch_channels()
        self.fast_page_view.start_fetch_channels()
        
        # Genres page
        self.genre_page = GenrePage()
        
        self.page_box = gtk.VBox()
        self.page_box.add(self.home_page)
        page_box_align = gtk.Alignment()
        page_box_align.set_padding(0, 0, 0, 2)
        page_box_align.set(1, 1, 1, 1)
        page_box_align.add(self.page_box)
        
        body_paned = HPaned(handle_color=app_theme.get_color("panedHandler"))
        body_paned.add1(self.radiobar)
        body_paned.add2(page_box_align)
        self.add(body_paned)
        
    def __init_radiobar(self):    
        self.radiobar = TreeView(enable_drag_drop=False, enable_multiple_select=False)
        items = []
        items.append(CategroyTreeItem(_("推荐首页"), lambda : switch_tab(self.page_box, self.home_page)))
        items.append(CategroyTreeItem(_("热门兆赫"), lambda : switch_tab(self.page_box, self.hot_page_sw)))
        items.append(CategroyTreeItem(_("人气兆赫"), lambda : switch_tab(self.page_box, self.fast_page_sw)))
        items.append(CategroyTreeItem(_("流派兆赫"), lambda : switch_tab(self.page_box, self.genre_page)))
        self.radiobar.add_items(items)
        self.radiobar.select_items([self.radiobar.visible_items[0]])
        self.radiobar.set_size_request(121, -1)
        self.radiobar.draw_mask = self.on_radiobar_draw_mask        
        
    def on_radiobar_draw_mask(self, cr, x, y, w, h):    
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")
        draw_line(cr, (x + 1, y), 
                  (x + 1, y + h), "#b0b0b0")
        return False
    
    def get_radio_icon_view(self, tag, padding_x=0, padding_y=0):
        icon_view = RadioIconView(tag, padding_x, padding_y)
        scrolled_window = ScrolledWindow()
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(icon_view)
        return icon_view, scrolled_window
        
        
    
    
