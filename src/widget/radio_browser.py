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

from widget.radio_item import CategroyRaidoItem
from widget.skin import app_theme
from widget.radio_home_page import HomePage
from widget.ui_utils import draw_line, draw_alpha_mask
from nls import _

class RadioBrowser(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Init radiobar.
        self.__init_radiobar()
        
        self.page_box = gtk.VBox()
        self.page_box.add(HomePage())
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
        items = [CategroyRaidoItem(title) for title in "推荐首页 热门兆赫 人气兆赫 流派兆赫".split()]
        self.radiobar.add_items(items)
        self.radiobar.set_size_request(121, -1)
        self.radiobar.draw_mask = self.on_radiobar_draw_mask        
        
    def on_radiobar_draw_mask(self, cr, x, y, w, h):    
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")
        draw_line(cr, (x + 1, y), 
                  (x + 1, y + h), "#b0b0b0")
        return False
        
