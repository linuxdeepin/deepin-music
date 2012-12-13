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
import cairo

from dtk.ui.draw import draw_pixbuf
from dtk.ui.utils import is_left_button, cairo_disable_antialias
from dtk.ui.cache_pixbuf import CachePixbuf

from widget.skin import app_theme
from utils import color_hex_to_cairo

class HScalebar(gtk.HScale):
    
    def __init__(self, 
                 fg_dpixbuf=app_theme.get_pixbuf("scalebar/fg.png"),
                 bg_dpixbuf=app_theme.get_pixbuf("scalebar/bg.png"),
                 point_normal_dpixbuf = app_theme.get_pixbuf("scalebar/point_normal.png"),
                 point_hover_dpixbuf = app_theme.get_pixbuf("scalebar/point_hover.png"),
                 point_press_dpixbuf = app_theme.get_pixbuf("scalebar/point_press.png"),
                 ):
        
        super(HScalebar, self).__init__()
        self.set_draw_value(False)
        self.set_range(0, 100)
        self.fg_dpixbuf = fg_dpixbuf
        self.bg_dpixbuf = bg_dpixbuf
        self.point_normal_dpixbuf = point_normal_dpixbuf
        self.point_hover_dpixbuf = point_hover_dpixbuf
        self.point_press_dpixbuf = point_press_dpixbuf
        self.bottom_side = 0
        self.fg_cache_pixbuf = CachePixbuf()
        self.bg_cache_pixbuf = CachePixbuf()
        self.side_cache_pixbuf = CachePixbuf()
        
        # Colors
        self.fg_left_dcolor = app_theme.get_color("progressBarLeft")
        self.fg_right_dcolor = app_theme.get_color("progressBarRight")
        
        self.set_size_request(-1, self.bg_dpixbuf.get_pixbuf().get_height())
        
        self.connect("expose-event", self.expose_h_scalebar)
        self.connect("button-press-event", self.press_volume_progressbar)
        
    def expose_h_scalebar(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        # Init pixbuf.
        fg_pixbuf = self.fg_dpixbuf.get_pixbuf()
        bg_pixbuf = self.bg_dpixbuf.get_pixbuf()
        point_normal_pixbuf = self.point_normal_dpixbuf.get_pixbuf()
        # point_hover_pixbuf = self.point_hover_dpixbuf.get_pixbuf()
        # point_press_pixbuf = self.point_press_dpixbuf.get_pixbuf()
        
        # Init value.
        upper = self.get_adjustment().get_upper()
        lower = self.get_adjustment().get_lower()
        total_length = max(upper - lower, 1)
        
        point_width = point_normal_pixbuf.get_width()
        point_height = point_normal_pixbuf.get_height()
        x, y, w, h = rect.x + point_width / 2, rect.y, rect.width - point_width, rect.height
        
        line_height = bg_pixbuf.get_height()

        line_y = y + (point_height - line_height) / 2
        value = int((self.get_value() - lower) / total_length * w)
        
        
        # Draw background.
        self.fg_cache_pixbuf.scale(
            bg_pixbuf, w + point_width, line_height)
        draw_pixbuf(
            cr,
            self.fg_cache_pixbuf.get_cache(),
            rect.x, line_y)
        
        self.bg_cache_pixbuf.scale(
            fg_pixbuf, point_width / 2, line_height)
        draw_pixbuf(
            cr,
            self.bg_cache_pixbuf.get_cache(),
            rect.x, line_y)

        
        if value > 0:
            pat = cairo.LinearGradient(0, 0, value + point_width, 0)
            pat.add_color_stop_rgb(0.7, *color_hex_to_cairo(self.fg_left_dcolor.get_color()))
            pat.add_color_stop_rgb(1.0, *color_hex_to_cairo(self.fg_right_dcolor.get_color()))
            cr.set_operator(cairo.OPERATOR_OVER)
            cr.set_source(pat)
            cr.rectangle(rect.x, line_y, value + point_width, line_height)
            cr.fill()
            
            with cairo_disable_antialias(cr):
                cr.set_line_width(1)
                cr.set_source_rgba(1, 1, 1, 0.5)
                cr.move_to(rect.x, line_y + 1)
                cr.rel_line_to(value + point_width, 0)
                cr.stroke()
                
                cr.set_source_rgba(1, 1, 1, 0.3)
                cr.move_to(rect.x, line_y + line_height)
                cr.rel_line_to(value + point_width, 0)
                cr.stroke()
                
            # self.side_cache_pixbuf.scale(
            #     fg_pixbuf, value + point_width, line_height)
            # draw_pixbuf(
            #     cr, 
            #     self.side_cache_pixbuf.get_cache(),
            #     rect.x, line_y)
            
        if value > 0:    
            draw_pixbuf(cr, point_normal_pixbuf, x + value - point_width / 2 + 2, y)    
        else:    
            draw_pixbuf(cr, point_normal_pixbuf, x + value - point_width / 2 - 1, y)
        
        return True
    
    
    def press_volume_progressbar(self, widget, event):
        # Init.
        if is_left_button(event):
            rect = widget.allocation
            lower = self.get_adjustment().get_lower()
            upper = self.get_adjustment().get_upper()
            point_width = self.point_normal_dpixbuf.get_pixbuf().get_width()
            
            self.set_value(lower + ((event.x - point_width / 2)) / (rect.width - point_width) * (upper - lower))
            self.queue_draw()
            
        return False    
            
gobject.type_register(HScalebar)

class VScalebar(gtk.Button):
    '''
    Volume button.
    '''
    __gtype_name__ = "VScalebar"
    __gsignals__ = {"value-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (float,)),
        }
    
    def __init__(self, value=100, lower=0, upper=100, step=5, default_height=100):
        gtk.Button.__init__(self)
        
        # Init default data.
        self.__value = value
        self.__lower = lower
        self.__upper = upper
        self.__step  = step
        self.default_height = default_height
        self.current_y = 0
        
        # Init DPixbuf.
        self.bg_bottom_dpixbuf = app_theme.get_pixbuf("volume/bg_bottom.png")
        self.bg_middle_dpixbuf = app_theme.get_pixbuf("volume/bg_middle.png")
        self.bg_top_dpixbuf = app_theme.get_pixbuf("volume/bg_top.png")
        self.point_dpixbuf = app_theme.get_pixbuf("volume/point.png")
        
        # Init sizes.
        self.fg_width = self.bg_width = self.bg_middle_dpixbuf.get_pixbuf().get_width()
        self.bg_top_height = self.bg_top_dpixbuf.get_pixbuf().get_height()
        self.bg_bottom_height = self.bg_bottom_dpixbuf.get_pixbuf().get_height()
        self.point_width = self.point_dpixbuf.get_pixbuf().get_width()
        self.point_height = self.point_dpixbuf.get_pixbuf().get_height()
        self.bg_x_offset = (self.point_width - self.bg_width) / 2
        self.set_size_request(self.point_width, self.default_height)
        self.real_height = self.default_height - self.point_height        
        
        # Init CachePixbuf.
        self.__bg_cache_pixbuf = CachePixbuf()
        
        # Init events.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.connect("button-press-event", self.on_button_press)
        self.connect("motion-notify-event", self.on_motion_notify)
        self.connect("button-release-event", self.on_button_release)
        self.connect("expose-event", self.on_expose_event)
        self.connect("scroll-event", self.on_scroll_event)
        
        # Init flags
        self.__button_press_flag = False
        
    def value_to_height(self, value):    
        return self.__upper / float(self.real_height)* value
    
    def height_to_value(self, height):
        return height / float(self.real_height) * self.__upper 
    
    def on_scroll_event(self, widget, event):
        if event.direction == gtk.gdk.SCROLL_UP:
            self.increase_value()
        elif event.direction == gtk.gdk.SCROLL_DOWN:
            self.decrease_value()
            
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        x, y, w, h = rect.x + self.bg_x_offset, rect.y + self.point_height / 2, rect.width, rect.height - self.point_height

        fg_x = bg_x = rect.x + self.bg_x_offset        
        # Draw background.
        draw_pixbuf(cr, self.bg_top_dpixbuf.get_pixbuf(), bg_x, y)
        
        middle_height = h - self.bg_top_height - self.bg_bottom_height
        self.__bg_cache_pixbuf.scale(self.bg_middle_dpixbuf.get_pixbuf(), 
                                     self.bg_width, middle_height)
        draw_pixbuf(cr, self.__bg_cache_pixbuf.get_cache(), bg_x, y + self.bg_top_height)
        draw_pixbuf(cr, self.bg_bottom_dpixbuf.get_pixbuf(), bg_x, y + h - self.bg_top_height)
        
        # Draw foreground.
        cr.set_source_rgb(*color_hex_to_cairo("#2868c7"))
        cr.rectangle(fg_x, y + self.current_y, self.fg_width, h - self.current_y)
        cr.fill()
        
        # # Draw point.
        draw_pixbuf(cr, self.point_dpixbuf.get_pixbuf(), rect.x, rect.y + self.current_y)
        return True
    
    def on_button_press(self, widget, event):
        if is_left_button(event):
            rect = widget.allocation
            if event.y < self.point_height / 2:
                motion_y = self.point_height / 2
            elif event.y > rect.height - self.point_height:
                motion_y = rect.height - self.point_height
            else:    
                motion_y = event.y
                
            if self.current_y != motion_y:    
                self.current_y = motion_y
                self.emit("value-changed", self.get_value())
                
                self.queue_draw()            
            self.__button_press_flag = True        
    
    def on_motion_notify(self, widget, event):
        if self.__button_press_flag:
            rect = widget.allocation
            if event.y < 0:
                motion_y = 0
            elif event.y > rect.height - self.point_height:
                motion_y = rect.height - self.point_height
            else:    
                motion_y = event.y
                
            if self.current_y != motion_y:    
                self.current_y = motion_y
                self.emit("value-changed", self.get_value())
                self.queue_draw()
    
    def on_button_release(self, widget, event):
        self.__button_press_flag = False
        
    def get_value(self):
        self.__value = self.height_to_value(self.real_height - self.current_y)
        return round(self.__value, 1)
    
    def set_value(self, value):
        self.current_y = self.real_height - self.value_to_height(value)
        self.queue_draw()
        
    def set_range(self, lower, upper):    
        self.__lower = lower
        self.__upper = upper
        
    def increase_value(self):    
        temp_y = self.current_y
        temp_y += self.value_to_height(self.__step)
        if temp_y > self.real_height:
            temp_y = self.real_height
        if temp_y != self.current_y:    
            self.current_y = temp_y
            self.emit("value-changed", self.get_value())
            self.queue_draw()
            
    def decrease_value(self):        
        temp_y = self.current_y
        temp_y -= self.value_to_height(self.__step)
        if temp_y < 0:
            temp_y = 0
        if temp_y != self.current_y:    
            self.current_y = temp_y
            self.emit("value-changed", self.get_value())
            self.queue_draw()
            
gobject.type_register(VScalebar)
            
