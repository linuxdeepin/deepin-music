#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Wang Yong
# 
# Author:     Wang Yong <lazycat.manatee@gmail.com>
# Maintainer: Wang Yong <lazycat.manatee@gmail.com>
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
from dtk.ui.treeview import TreeView, TreeItem
from dtk.ui.constant import DEFAULT_FONT_SIZE
from dtk.ui.utils import (get_widget_root_coordinate, WIDGET_POS_BOTTOM_LEFT, cairo_disable_antialias, 
                          alpha_color_hex_to_cairo, get_content_size,
                          color_hex_to_cairo)
from dtk.ui.draw import draw_text
from dtk.ui.popup_grab_window import PopupGrabWindow, wrap_grab_window
from dtk.ui.window import Window
from widget.ui import SearchBox
from widget.ui_utils import draw_single_mask
from widget.skin import app_theme

class CompletionWindow(Window):
    '''
    class docs
    '''
	
    def __init__(self, window_width, window_height):
        '''
        init docs
        '''
        # Init.
        Window.__init__(
            self,
            shadow_visible=False,
            shadow_radius = 0,
            shape_frame_function=self.shape_completion_window_frame,
            expose_frame_function=self.expose_completion_window_frame,
            # window_type=gtk.WINDOW_POPUP,
            )
        
        self.window_width = window_width
        self.window_height = window_height
        self.window_offset_x = 0
        self.window_offset_y = 0
        self.align_size = 2
        
        self.treeview = TreeView(
            enable_highlight=False,
            enable_multiple_select=False,
            enable_drag_drop=False,)
        
        self.treeview.draw_mask = self.treeview_draw_mask
        self.treeview.scrolled_window.tag_by_popup_grab_window = True
        self.treeview_align = gtk.Alignment()
        self.treeview_align.set(0.5, 0.5, 1, 1)
        self.treeview_align.set_padding(self.align_size, self.align_size, self.align_size, self.align_size)
        self.treeview_align.add(self.treeview)
        
        self.window_frame.pack_start(self.treeview_align, True, True)
        
        self.connect("realize", self.realize_completion_window)
        
        self.get_scrolledwindow = self.get_scrolledwindow
        
        wrap_grab_window(completion_grab_window, self)
        
    def get_scrolledwindow(self):
        return self.treeview.scrolled_window
        
    def shape_completion_window_frame(self, widget, event):
        pass
    
    def treeview_draw_mask(self, cr, x, y, w, h):
        cr.set_source_rgba(1, 1, 1, 0.95)
        cr.rectangle(x, y, w, h)
        cr.fill()
        
    def expose_completion_window_frame(self, widget, event):
        cr = widget.window.cairo_create()        
        rect = widget.allocation

        cr  = widget.window.cairo_create()
        rect = widget.allocation
        cr.set_source_rgb(1,1,1)
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
        cr.set_line_width(1)
        cr.set_source_rgb(*color_hex_to_cairo("#c7c7c7"))
        cr.rectangle(*rect)
        cr.stroke()
            
    def show(self, keyword):
        self.treeview.delete_all_items()
        self.treeview.add_items([TextItem(key) for key in ["linux", "deepin", "word", keyword]])
        self.treeview.select_first_item()        
        # self.treeview.grab_focus()
        
        (x, y) = get_widget_root_coordinate(search_entry, WIDGET_POS_BOTTOM_LEFT, True)
        self.move(x + self.window_offset_x, y + self.window_offset_y)
        self.show_all()
        
    def realize_completion_window(self, widget):
        self.set_default_size(self.window_width, self.window_height)
        self.set_geometry_hints(
            None,
            self.window_width,       # minimum width
            self.window_height,       # minimum height
            self.window_width,
            self.window_height,
            -1, -1, -1, -1, -1, -1
            )
        
gobject.type_register(CompletionWindow)        

class CompletionGrabWindow(PopupGrabWindow):
    '''
    class docs
    '''
	
    def __init__(self):
        '''
        init docs
        '''
        PopupGrabWindow.__init__(self, CompletionWindow)
        
    def popup_grab_window_key_press(self, widget, event):
        if event and event.window:
            for popup_window in self.popup_windows:
                popup_window.event(event)

            search_entry.entry.event(event)    
            

    def popup_grab_window_key_release(self, widget, event):
        if event and event.window:
            for popup_window in self.popup_windows:
                popup_window.event(event)
            
            search_entry.entry.event(event)    
            
        
class TextItem(TreeItem):
    '''
    class docs
    '''
	
    def __init__(self, 
                 text, 
                 # search_string,
                 text_size = DEFAULT_FONT_SIZE,
                 padding_x = 10,
                 padding_y = 6):
        '''
        init docs
        '''
        # Init.
        TreeItem.__init__(self)
        self.text = text
        # self.search_string = search_string
        self.text_size = text_size
        self.padding_x = padding_x
        self.padding_y = padding_y
        (self.text_width, self.text_height) = get_content_size(self.text)
        
    def render_text(self, cr, rect):
        if self.is_hover:
            # Draw background.
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width - 2, rect.height, "globalItemSelect")
            
            # Set font color.
            font_color = "#FFFFFF"
            
            # Don't highlight when select.
            # text = self.text
        else:
            # Set font color.
            font_color = app_theme.get_color("labelText").get_color()
            
            # Highilght match string.
            # (text_pre, text_post) = self.text.split(self.search_string)
            # text = "%s<span foreground=\"#00AAFF\">%s</span>%s" % (text_pre, self.search_string, text_post)
            
        draw_text(cr, 
                  self.text,
                  rect.x + self.padding_x, 
                  rect.y,
                  rect.width - self.padding_x * 2, 
                  rect.height,
                  text_color=font_color)
        
    def get_width(self):
        return self.text_width + self.padding_x * 2
        
    def get_height(self):
        return self.text_size + self.padding_y * 2
    
    def get_column_widths(self):
        return [-1]
    
    def get_column_renders(self):
        return [self.render_text]

    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False

        if self.redraw_request_callback:
            self.redraw_request_callback(self)
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        print "hover "
        
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def button_press(self, column, offset_x, offset_y):
        print "button-press"
    
gobject.type_register(TextItem)

search_entry = SearchBox()
completion_grab_window = CompletionGrabWindow()
completion_window = CompletionWindow(300, 200)
