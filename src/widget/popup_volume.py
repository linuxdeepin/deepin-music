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
from dtk.ui.draw import draw_pixbuf
from dtk.ui.utils import is_left_button, get_match_parent, get_widget_root_coordinate
from dtk.ui.window import Window
from dtk.ui.constant import WIDGET_POS_BOTTOM_LEFT
from dtk.ui.popup_grab_window import PopupGrabWindow, wrap_grab_window

from widget.skin import app_theme
from widget.scalebar import VScalebar

class PopupVolume(Window):
    
    def __init__(self, default_width=27):
        Window.__init__(self, window_type=gtk.WINDOW_POPUP)
        self.set_size_request(default_width, 100)
        
        self.volume_slider = VScalebar(default_height=80)
        self.volume_slider.tag_by_popup_volume_grab_window = True
        
        main_align = gtk.Alignment()
        main_align.set_padding(5, 5, 5, 5)
        main_align.add(self.volume_slider)
        self.window_frame.pack_start(main_align)
        
        wrap_grab_window(volume_grab_window, self)
        
    def show(self, x, y):    
        self.move(x, y)
        self.show_all()
        
gobject.type_register(PopupVolume)        
        
class PopupVolumeGrabWindow(PopupGrabWindow):
    '''
    class docs
    '''
	
    def __init__(self):
        '''
        init docs
        '''
        PopupGrabWindow.__init__(self, PopupVolume)
        self.button_press_callback = None
        
    def popup_grab_window_motion_notify(self, widget, event):
        '''
        Handle `motion-notify` signal of popup_grab_window.
    
        @param widget: Popup_Window widget.
        @param event: Motion notify signal.
        '''
        if event and event.window:
            event_widget = event.window.get_user_data()
            if isinstance(event_widget, VScalebar) and hasattr(event_widget, "tag_by_popup_volume_grab_window"):
                if self.press_flag:
                    if self.button_press_callback:
                        self.button_press_callback()
                    event_widget.event(event)
            elif isinstance(event_widget, PopupVolume):    
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
        
    def popup_grab_window_button_release(self, widget, event):            
        if event and event.window:
            event_widget = event.window.get_user_data()
            if isinstance(event_widget, VScalebar) and hasattr(event_widget, "tag_by_popup_volume_grab_window"):
                self.press_flag = False
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                
            elif isinstance(event_widget, PopupVolume):    
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                
                
    def popup_grab_window_scroll_event(self, widget, event):            
        if event and event.window:
            event_widget = event.window.get_user_data()
            if isinstance(event_widget, VScalebar) and hasattr(event_widget, "tag_by_popup_volume_grab_window"):
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                    
            elif isinstance(event_widget, PopupVolume):    
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                
    def popup_grab_window_enter_notify(self, widget, event):            
        if event and event.window:
            event_widget = event.window.get_user_data()
            if isinstance(event_widget, VScalebar) and hasattr(event_widget, "tag_by_popup_volume_grab_window"):
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                
            elif isinstance(event_widget, PopupVolume):    
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
               
                
    def popup_grab_window_leave_notify(self, widget, event):            
        if event and event.window:
            event_widget = event.window.get_user_data()
            if isinstance(event_widget, VScalebar) and hasattr(event_widget, "tag_by_popup_volume_grab_window"):
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                 
            elif isinstance(event_widget, PopupVolume):    
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                 
                
    def popup_grab_window_button_press(self, widget, event):
        '''
        Handle `button-press-event` signal of popup_grab_window.
    
        @param widget: Popup_Window widget.
        @param event: Button press event.
        '''
        
        if event and event.window:
            event_widget = event.window.get_user_data()
            if self.is_press_on_popup_grab_window(event.window):
                if self.button_press_callback:
                    self.button_press_callback()
                self.popup_grab_window_focus_out()
            elif isinstance(event_widget, self.wrap_window_type):
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
            # elif isinstance(event_widget, PopupVolume):    
            #     if self.button_press_callback:
            #         self.button_press_callback()
            #     event_widget.event(event)
            elif isinstance(event_widget, VScalebar) and hasattr(event_widget, "tag_by_popup_volume_grab_window"):
                self.press_flag = True                
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
            else:
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                self.popup_grab_window_focus_out()
                
volume_grab_window = PopupVolumeGrabWindow()
        

class VolumeButton(gtk.Button):
    
    def __init__(self):
        gtk.Button.__init__(self)
        
        # Init signals.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.connect("expose-event", self.on_expose_event)
        self.connect("button-press-event", self.on_button_press)
        
        # Init app_theme DPixbuf.
        self.update_status_icons("high")
        
        # Init sizes.
        width = self.normal_dpixbuf.get_pixbuf().get_width()
        height = self.normal_dpixbuf.get_pixbuf().get_height()
        self.set_size_request(width, height)
        
        # Init popup volume
        self.popup_volume = PopupVolume()
        self.max_value = 100
        self.volumebar = self.popup_volume.volume_slider 
        self.volumebar.connect("value-changed", self.on_volumebar_value_changed)
        
    def update_status_icons(self, name, redraw=False):
        self.normal_dpixbuf = app_theme.get_pixbuf("volume/%s_normal.png" % name)
        self.hover_dpixbuf = app_theme.get_pixbuf("volume/%s_hover.png" % name)
        self.press_dpixbuf = app_theme.get_pixbuf("volume/%s_press.png" % name)        
        if redraw: self.queue_draw()
        
    def on_volumebar_value_changed(self, widget, value):    
        if value == 0:
            self.update_status_icons("zero", True)
        elif 0 < value <= self.max_value * (1.0/3):
            self.update_status_icons("low", True)
        elif self.max_value * (1.0/3) < value <= self.max_value * (2.0 / 3):    
            self.update_status_icons("medium", True)
        else:    
            self.update_status_icons("high", True)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        pixbuf = None
        
        if widget.state == gtk.STATE_NORMAL:
            pixbuf = self.normal_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_PRELIGHT:
            pixbuf = self.hover_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_ACTIVE:    
            pixbuf = self.press_dpixbuf.get_pixbuf()
            
        if pixbuf is None:    
            pixbuf = self.normal_dpixbuf.get_pixbuf()
            
        draw_pixbuf(cr, pixbuf, rect.x, rect.y)    
        
        return True
        
    def on_button_press(self, widget, event):    
        x, y =  get_widget_root_coordinate(widget, WIDGET_POS_BOTTOM_LEFT)
        self.popup_volume.show(x, y)
        volume_grab_window.popup_grab_window_focus_in()        
