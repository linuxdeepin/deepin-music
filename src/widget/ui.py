#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 Deepin, Inc.
#               2011 Hou Shaohui
#
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou ShaoHui <houshao55@gmail.com>
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

import os
import gtk
from dtk.ui.window import Window
from dtk.ui.theme import Theme
from dtk.ui.utils import move_window, is_double_click
from dtk.ui.titlebar import Titlebar

app_theme = Theme(os.path.join((os.path.dirname(os.path.realpath(__file__))), "../../app_theme"))

class NormalWindow(object):
    
    def __init__(self, parent=None):
        
        # Init Window
        self.window = Window()
        self.window.set_position(gtk.WIN_POS_CENTER)
        # self.window.set_modal(True)
        self.titlebar = Titlebar(["close"])
        self.titlebar.close_button.connect("clicked", lambda w: self.hide_window())
        self.titlebar.drag_box.connect('button-press-event', lambda w, e: move_window(w, e, self.window))
        self.parent = parent
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(5, 5, 10, 10)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)

        self.window.window_frame.pack_start(self.titlebar.box, False, False)
        self.window.window_frame.pack_start(main_align)
        
    def show_window(self):    
        if self.parent:
            parent_rect = self.parent.get_toplevel().get_allocation()
            self.window.move(parent_rect.x + parent_rect.width / 2, parent_rect.y + parent_rect.height / 2)
        else:    
            self.window.show_all()
            
    def hide_window(self):        
        self.window.hide_all()
        

# song_scalebar = HScalebar(
#     app_theme.get_pixbuf("hscalebar/left_fg.png"),
#     app_theme.get_pixbuf("hscalebar/left_bg.png"),
#     app_theme.get_pixbuf("hscalebar/middle_fg.png"),
#     app_theme.get_pixbuf("hscalebar/middle_bg.png"),
#     app_theme.get_pixbuf("hscalebar/right_fg.png"),
#     app_theme.get_pixbuf("hscalebar/right_bg.png"),
#     app_theme.get_pixbuf("hscalebar/point.png"),
#     )

