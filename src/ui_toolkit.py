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
from dtk.ui.box import EventBox, ImageBox, TextBox
from dtk.ui.button import Button, ImageButton, ToggleButton
from dtk.ui.scalebar import HScalebar, VScalebar
from dtk.ui.volume_button import VolumeButton
from dtk.ui.frame import HorizontalFrame

app_theme = Theme(os.path.join((os.path.dirname(os.path.realpath(__file__))), "../app_theme"))

class NormalWindow():
    
    def __init__(self):
        self.window = Window(True)
        self.window.set_position(gtk.WIN_POS_CENTER)
        self.window.set_default_size(400, 300)
        
        # Init main box.
        self.main_box = self.window.window_frame
        
        # Add titlebar box.
        self.titlebar = None
        self.titlebar_box = gtk.HBox()
        self.main_box.pack_start(self.titlebar_box, False)
        
    def add_titlebar(self, button_mask=["max", "min", "close"], 
                     icon_dpixbuf=None, app_name=None, title=None, add_separator=False):
        '''Add titlebar.'''
        # Init titlebar.
        self.titlebar = Titlebar(button_mask, icon_dpixbuf, app_name, title, add_separator)
        if "min" in button_mask:
            self.titlebar.min_button.connect("clicked", lambda w: self.window.min_window())
        if "max" in button_mask:
            self.titlebar.max_button.connect("clicked", lambda w: self.window.toggle_max_window())
        if "close" in button_mask:
            self.titlebar.close_button.connect("clicked", lambda w: self.window.close_window())
        self.add_toggle_window_event(self.titlebar.drag_box)
        self.add_move_window_event(self.titlebar.drag_box)
        
        # Show titlebar.
        self.show_titlebar()
        
    def show_titlebar(self):
        '''Show titlebar.'''
        if self.titlebar_box.get_children() == [] and self.titlebar != None:
            self.titlebar_box.add(self.titlebar.box)
            
    def hide_titlebar(self):
        '''Hide titlebar.'''
        container_remove_all(self.titlebar_box)            
        
    def raise_to_top(self):
        '''Raise to top.'''
        self.window.present()
        
    def set_title(self, title):
        '''Set application title.'''
        self.window.set_title(title)

    def set_default_size(self, default_width, default_height):
        '''Set application default size.'''
        self.window.set_default_size(default_width, default_height)
        self.window.set_geometry_hints(
            None,
            default_width,       # minimum width
            default_height       # minimum height
            -1, -1, -1, -1, -1, -1, -1, -1
            )

    def set_icon(self, icon_dpixbuf):
        '''Set icon.'''
        gtk.window_set_default_icon(icon_dpixbuf.get_pixbuf())

    def run(self):
        '''Run.'''
        # Show window.
        self.window.show_window()
    
    def double_click_window(self, widget, event):
        '''Handle double click on window.'''
        if is_double_click(event):
            self.window.toggle_max_window()
            
        return False
            
    def add_toggle_window_event(self, widget):
        '''Add toggle window event.'''
        widget.connect("button-press-event", self.double_click_window)
    
    def add_move_window_event(self, widget):
        '''Add move window event.'''
        widget.connect('button-press-event', lambda w, e: move_window(w, e, self.window))
        
    def set_menu_callback(self, callback):
        '''Set menu callback.'''
        self.menu_button_callback = callback
        

song_scalebar = HScalebar(
    app_theme.get_pixbuf("hscalebar/left_fg.png"),
    app_theme.get_pixbuf("hscalebar/left_bg.png"),
    app_theme.get_pixbuf("hscalebar/middle_fg.png"),
    app_theme.get_pixbuf("hscalebar/middle_bg.png"),
    app_theme.get_pixbuf("hscalebar/right_fg.png"),
    app_theme.get_pixbuf("hscalebar/right_bg.png"),
    app_theme.get_pixbuf("hscalebar/point.png"),
    )

playlist_button = ToggleButton(
    app_theme.get_pixbuf("control/playlist_normal.png"),
    app_theme.get_pixbuf("control/playlist_hover.png"))

lyrics_button = ToggleButton(
    app_theme.get_pixbuf("control/lyrics_normal.png"),
    app_theme.get_pixbuf("control/lyrics_hover.png"))

musicbox_button = ToggleButton(
    app_theme.get_pixbuf("control/musicbox_normal.png"),
    app_theme.get_pixbuf("control/musicbox_hover.png"))

media_button = ToggleButton(
    app_theme.get_pixbuf("control/media_normal.png"),
    app_theme.get_pixbuf("control/media_hover.png"))


if __name__ == "__main__":
    a = NormalWindow()
    a.run()
    gtk.main()

