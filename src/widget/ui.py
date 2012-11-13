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
from dtk.ui.utils import (move_window, alpha_color_hex_to_cairo, 
                          color_hex_to_cairo, cairo_disable_antialias,
                          propagate_expose)
from dtk.ui.entry import InputEntry, Entry
from dtk.ui.button import ImageButton
from dtk.ui.draw import draw_pixbuf, draw_text
from widget.skin import app_theme
from widget.ui_utils import draw_alpha_mask
from helper import Dispatcher

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
            
    def hide_window(self, widget):        
        self.hide_all()
        

        
class SearchEntry(InputEntry):
    
    def __init__(self, *args, **kwargs):


        entry_button = ImageButton(
            app_theme.get_pixbuf("toolbar/search_normal.png"),
            app_theme.get_pixbuf("toolbar/search_hover.png"),
            app_theme.get_pixbuf("toolbar/search_press.png")
            )
        super(SearchEntry, self).__init__(action_button=entry_button, *args, **kwargs)        
        
        self.action_button = entry_button
        self.set_size(250, 24)
        
gobject.type_register(SearchEntry)        

        
class ComplexButton(gtk.Button):    
    
    def __init__(self, bg_group, icon, content, left_padding=20, label_padding=10, font_size=9):
        super(ComplexButton, self).__init__()
        
        # Init.
        self.normal_bg, self.hover_bg, self.press_bg = bg_group
        self.button_icon = icon
        self.content = content
        self.font_size = font_size
        self.left_padding = left_padding
        self.label_padding = label_padding
        
        # Set size.
        self.set_button_size()
        self.connect("expose-event", self.expose_button)
        
    def set_button_size(self):    
        request_width  = self.normal_bg.get_pixbuf().get_width()
        request_height = self.normal_bg.get_pixbuf().get_height()
        self.set_size_request(request_width, request_height)
        
    def expose_button(self, widget, event):    
        
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        if widget.state == gtk.STATE_NORMAL:
            bg_pixbuf = self.normal_bg.get_pixbuf()
        elif widget.state == gtk.STATE_PRELIGHT:    
            bg_pixbuf = self.hover_bg.get_pixbuf()
        elif widget.state == gtk.STATE_ACTIVE:    
            bg_pixbuf = self.press_bg.get_pixbuf()
            
        icon_pixbuf = self.button_icon.get_pixbuf()    
            
        icon_y = rect.y + (rect.height - icon_pixbuf.get_height()) / 2    
        
        # Draw bg.
        draw_pixbuf(cr, bg_pixbuf, rect.x, rect.y)
        
        # Draw icon.
        draw_pixbuf(cr, icon_pixbuf, rect.x + self.left_padding, icon_y)
        
        # Draw label.
        draw_text(cr, self.content, rect.x + self.left_padding + self.label_padding + icon_pixbuf.get_width(),
                  rect.y, rect.width - self.left_padding - self.label_padding - icon_pixbuf.get_width(), rect.height,
                  self.font_size, text_color="#FFFFFF")
        
        return True

gobject.type_register(ComplexButton)    

class SearchBox(gtk.EventBox):
    
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        # Init signals.
        self.connect("expose-event", self.on_expose_event)
        
        # Init DPixbufs.
        self.normal_dpixbuf = app_theme.get_pixbuf("toolbar/search_normal.png")
        self.hover_dpixbuf = app_theme.get_pixbuf("toolbar/search_hover.png")
        self.press_dpixbuf = app_theme.get_pixbuf("toolbar/search_press.png")
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        # Draw Background.
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.set_source_rgb(*color_hex_to_cairo("#D7D7D7"))
        cr.fill()
        
        pixbuf  = None
        if widget.state == gtk.STATE_NORMAL:
            pixbuf = self.normal_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_PRELIGHT:    
            pixbuf = self.hover_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_ACTIVE:    
            pixbuf = self.press_dpixbuf.get_pixbuf()
            
        if pixbuf is None:    
            pixbuf = self.normal_dpixbuf.get_pixbuf()
            
        icon_x = rect.x + (rect.width - pixbuf.get_width()) / 2
        icon_y = rect.y + (rect.height - pixbuf.get_height()) / 2
        draw_pixbuf(cr, pixbuf, icon_x, icon_y)    
        return True

class CustomEntry(gtk.VBox):
    __gsignals__ = {
        
        "action-active" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str,)),
    }
    
    def __init__(self,  content=""):
        '''
        Initialize InputEntry class.
        '''
        # Init.
        gtk.VBox.__init__(self)
        self.entry = Entry(content)
        self.add(self.entry)
        self.connect("expose-event", self.expose_input_entry)
            
    def set_sensitive(self, sensitive):
        '''
        Internal function to wrap function `set_sensitive`.
        '''
        super(InputEntry, self).set_sensitive(sensitive)
        self.entry.set_sensitive(sensitive)
            
    def emit_action_active_signal(self):
        '''
        Internal callback for `action-active` signal.
        '''
        self.emit("action-active", self.get_text())                
        
    def expose_input_entry(self, widget, event):
        '''
        Internal callback for `expose-event` signal.
        '''
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        x, y, w, h = rect.x, rect.y, rect.width, rect.height

        
        cr.set_source_rgba(*alpha_color_hex_to_cairo(("#FFFFFF", 0.9)))
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
        # Draw frame.
        with cairo_disable_antialias(cr):
            cr.set_line_width(1)
            cr.set_source_rgba(*color_hex_to_cairo("#C3C4C5"))
            cr.move_to(rect.x + rect.width, rect.y)
            cr.rel_line_to(0, rect.height)
            cr.stroke()
        
        propagate_expose(widget, event)
        
        # return True
    
    def set_size(self, width, height):
        '''
        Set input entry size with given value.
        
        @param width: New width of input entry.
        @param height: New height of input entry.
        '''
        # self.set_size_request(width, height)    
        self.entry.set_size_request(width - 2, height - 2)
        
    def set_editable(self, editable):
        '''
        Set editable status of input entry.
        
        @param editable: input entry can editable if option is True, else can't edit.
        '''
        self.entry.set_editable(editable)
        
    def set_text(self, text):
        '''
        Set text of input entry.
        
        @param text: input entry string.
        '''
        self.entry.set_text(text)
        
    def get_text(self):
        '''
        Get text of input entry.
        
        @return: Return text of input entry.
        '''
        return self.entry.get_text()
    
    def focus_input(self):
        '''
        Focus input cursor.
        '''
        self.entry.grab_focus()
        
gobject.type_register(InputEntry)
    