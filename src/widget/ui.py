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

import gtk
import gobject

from dtk.ui.window import Window
from dtk.ui.titlebar import Titlebar
from dtk.ui.utils import move_window
from dtk.ui.entry import TextEntry
from dtk.ui.button import ImageButton
from dtk.ui.draw import draw_vlinear
from dtk.ui.button import Button

from widget.skin import app_theme

class NormalWindow(Window):
    
    def __init__(self):
        super(NormalWindow, self).__init__(True)
        
        # Init Window
        self.set_position(gtk.WIN_POS_CENTER)
        self.titlebar = Titlebar(["close"])
        self.titlebar.close_button.connect_after("clicked", self.hide_window)
        self.titlebar.drag_box.connect('button-press-event', lambda w, e: move_window(w, e, self.window))
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(5, 10, 10, 10)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)
        self.window_frame.pack_start(self.titlebar, False, False)
        self.window_frame.pack_start(main_align, True, True)
        
    def show_window(self):    
        self.show_all()
        print ""
            
    def hide_window(self, widget):        
        self.hide_all()
        
        
class SearchEntry(TextEntry):
    
    def __init__(self, *args, **kwargs):


        entry_button = ImageButton(
            app_theme.get_pixbuf("entry/search_normal.png"),
            app_theme.get_pixbuf("entry/search_hover.png"),
            app_theme.get_pixbuf("entry/search_press.png")
            )
        super(SearchEntry, self).__init__(action_button=entry_button, *args, **kwargs)        
        
        self.action_button = entry_button
        self.set_size(250, 24)
        
gobject.type_register(SearchEntry)        

        
    
class MaskHBox(gtk.HBox):    
    
    def __init__(self, color_info, **kwargs):
        super(MaskHBox, self).__init__(**kwargs)
        
        self.color_info = color_info
        self.connect("expose-event", self.draw_mask)
        
    def draw_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x , rect.y, rect.width, rect.height,
                     self.color_info)
        return False
    
class MaskVBox(gtk.VBox):    
    
    def __init__(self, width, height, color_info):
        super(MaskVBox, self).__init__()
        
        self.set_size_request(width, height)
        self.color_info = color_info
        self.connect("expose-event", self.draw_mask)
        
    def draw_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height,
                     self.color_info)
        return False
    
    
class ProgressBox(gtk.VBox):
    
    def __init__(self, scalebar):
        super(ProgressBox, self).__init__()
        scalebar_align = gtk.Alignment()
        scalebar_align.set_padding(0, 0, 2, 2)
        scalebar_align.set(0, 0, 1, 1)
        scalebar_align.add(scalebar)
        
        self.set_size_request(-1, 20)
        self.rect_list = [
            (98, app_theme.get_shadow_color("playlistLeft").get_color_info()),
            (220, app_theme.get_shadow_color("playlistMiddle").get_color_info()),
            (140, app_theme.get_shadow_color("playlistRight").get_color_info()),
            ]
        
        self.pack_start(scalebar_align, False, True)

        self.connect("expose-event", self.draw_mask)
        
    def draw_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        start_x = rect.x + 2
        start_y = rect.y + 8

        for size, color_info in self.rect_list:
            draw_vlinear(cr, start_x, start_y, size, rect.height - 8, color_info)
            start_x += size
            
        last_width = rect.width - (start_x - rect.x)    
        draw_vlinear(cr, start_x, start_y, last_width - 2, rect.height - 8,
                     app_theme.get_shadow_color("playlistLast").get_color_info())
        return False
    
class ColorButton(gtk.EventBox):    
    
    def __init__(self):
        super(ColorButton, self).__init__()
        self.set_visible_window(False)
        self.set_app_paintable(True)
        self.set_size_request(60, 25)
        self.connect("expose-event", self.draw_color_button_mask)
        
    def draw_color_button_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height,
                     self.get_shadow_colors("#FF0000"), 3)
        
        return True
        
    def get_shadow_colors(self, html_color):    
        return [
            (0,    ("%s" % html_color, 0.8)),
            (0.05, ("%s" % html_color, 1.0)),
            (0.95, ("%s" % html_color, 1.0)),
            (1,    ("%s" % html_color, 0.8)),
            ]
