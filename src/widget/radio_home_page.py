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

from widget.slide_switcher import SlideSwitcher
from widget.tab_switcher import TabSwitcher
from widget.ui_utils  import  draw_alpha_mask

class HomePage(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        self.connect("expose-event", self.on_expose_event)
        self.home_slider = SlideSwitcher()
        self.home_slider.set_size_request(-1, 200)
        self.recommend_tab = TabSwitcher(["热门兆赫", "人气兆赫"])
        self.recommend_box = gtk.VBox()        
        self.set_spacing(5)
        
        self.pack_start(self.home_slider, False, True)
        self.pack_start(self.recommend_tab, False, True)
        self.pack_start(self.recommend_box, True, True)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutLast")
