#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2013 Deepin, Inc.
#               2011 ~ 2013 Hou ShaoHui
# 
# Author:     Hou ShaoHui <houshao55@gmail.com>
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
import cairo

from dtk.ui.cache_pixbuf import CachePixbuf
from dtk.ui.draw import draw_pixbuf
from dtk.ui.utils import set_cursor, cairo_disable_antialias, color_hex_to_cairo
from widget.skin import app_theme

STATE_NORMAL = 1
STATE_HOVER = 2
STATE_PRESS = 3

class VolumeButton(gtk.Button):
    
    __gsignals__ = { 
        "volume-state-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT, gobject.TYPE_PYOBJECT)),
                     }
    
    def __init__(self, value=100, lower=0, upper=100, step=5, progress_width=45, auto_hide=True, mini_mode=False):
        gtk.Button.__init__(self)
        
        # Init data.
        self.__value = value
        self.__lower = lower
        self.__upper = upper
        self.__step = step
        self.progress_width = progress_width
        self.mute_flag = False
        self.state_press_flag = False
        self.drag_flag = False
        self.drag_out_area = False
        self.hide_progress_flag = True
        self.current_progress_width = self.value_to_width(self.__value)
        self.icon_state = STATE_NORMAL
        self.auto_hide = auto_hide
        
        # Init DPixbufs.
        if mini_mode:
            self.volume_prefix = "mini_volume"
        else:    
            self.volume_prefix = "volume"
        self.bg_dpixbuf = app_theme.get_pixbuf("%s/bg.png" % self.volume_prefix)
        self.fg_dpixbuf = app_theme.get_pixbuf("%s/fg.png" % self.volume_prefix)
        self.point_dpixbuf = app_theme.get_pixbuf("%s/point.png" % self.volume_prefix)
        self.update_state_dpixbufs(self.get_state_name(self.__value))
        
        # Init Dcolors.
        self.fg_left_dcolor = app_theme.get_color("progressBarLeft")
        self.fg_right_dcolor = app_theme.get_color("progressBarRight")
        
        
        # Init Sizes.
        self.padding_x = 0
        self.padding_y = 0
        self.progress_x = 0
        self.state_icon_rect = gtk.gdk.Rectangle(
            self.padding_x, self.padding_y,
            self.normal_dpixbuf.get_pixbuf().get_width() - 2,
            self.normal_dpixbuf.get_pixbuf().get_height())
        
        self.point_width = self.point_dpixbuf.get_pixbuf().get_width()
        self.base_width = self.padding_x * 2 + self.normal_dpixbuf.get_pixbuf().get_width()        
        self.expand_width = self.base_width + self.progress_width + self.progress_x  + self.point_width
        self.default_height = self.padding_y * 2 + self.normal_dpixbuf.get_pixbuf().get_height()
        self.point_offset = self.state_icon_rect.x + self.state_icon_rect.width + self.progress_x - 2
        self.fg_offset = self.bg_offset = self.point_offset + self.point_width / 2
        
        # Init CachePixbufs
        self.bg_cache_pixbuf = CachePixbuf()
        self.fg_cache_pixbuf = CachePixbuf()
        
        # Init Events.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.connect("expose-event", self.on_expose_event)
        self.connect("scroll-event", self.on_scroll_event)
        self.connect("enter-notify-event", self.on_enter_notify_event)
        self.connect("leave-notify-event", self.on_leave_notify_event)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("motion-notify-event", self.on_motion_notify_event)
        self.connect("button-release-event", self.on_button_release_event)
        
        if self.auto_hide:
            if self.hide_progress_flag:
                self.set_size_request(self.base_width, self.default_height)
            else:    
                self.set_size_request(self.expand_width, self.default_height)
        else:        
            self.set_size_request(self.expand_width, self.default_height)
            
    def value_to_width(self, value):    
        return value / float(self.__upper) * self.progress_width
    
    def get_size(self):
        if self.auto_hide:
            if self.hide_progress_flag:
                return (self.base_width, self.default_height)
        return (self.expand_width, self.default_height)    
    
    def width_to_value(self, width):
        return width / float(self.progress_width) * self.__upper 
    
    def update_state_by_value(self, emit=True):
        value = self.width_to_value(int(self.current_progress_width))
        state_name = self.get_state_name(value)
        self.update_state_dpixbufs(state_name, queue_draw=True)
        if emit:
            self.emit("volume-state-changed", self.get_value(), self.mute_flag)        
    
    def update_progress_width(self, event, emit=False):
        self.current_progress_width = int(event.x - self.fg_offset)
        if self.current_progress_width < 0:
            self.current_progress_width = 0
        elif self.current_progress_width > self.progress_width:    
            self.current_progress_width = self.progress_width
        self.update_state_by_value(emit=emit)    
        self.queue_draw()
        
    def update_state_dpixbufs(self, name, queue_draw=False):    
        self.normal_dpixbuf = app_theme.get_pixbuf("%s/%s_normal.png" % (self.volume_prefix, name))
        self.hover_dpixbuf = app_theme.get_pixbuf("%s/%s_hover.png" % (self.volume_prefix, name))
        self.press_dpixbuf = app_theme.get_pixbuf("%s/%s_press.png" % (self.volume_prefix, name))
        
        if queue_draw:
            self.queue_draw()
        
    def get_state_name(self, value):
        if value == 0:
            state_name = "zero"
        elif 0 < value <= self.__upper * (1.0/3):
            state_name = "low"
        elif self.__upper * (1.0/3) < value <= self.__upper * (2.0 / 3):    
            state_name = "medium"
        else:    
            state_name = "high"
        return state_name    
    
    def on_expose_event(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        self.draw_volume(cr, rect)
        return True
        
    def draw_volume(self, cr, rect):    
        # Draw state icon.
        if self.icon_state == STATE_HOVER:
            pixbuf = self.hover_dpixbuf.get_pixbuf()
        elif self.icon_state == STATE_PRESS:    
            pixbuf = self.press_dpixbuf.get_pixbuf()
        else:    
            pixbuf = self.normal_dpixbuf.get_pixbuf()
        draw_pixbuf(cr, pixbuf, rect.x + self.padding_x, rect.y + self.padding_y)    
        
        if self.auto_hide:
            if not self.hide_progress_flag:
                self.draw_progress_bar(cr, rect)
        else:        
            self.draw_progress_bar(cr, rect)
        
    def draw_progress_bar(self, cr, rect):                    
        
        # Draw progressbar background.
        bg_height = self.bg_dpixbuf.get_pixbuf().get_height()
        self.bg_cache_pixbuf.scale(self.bg_dpixbuf.get_pixbuf(), self.progress_width, 
                                   bg_height)
        

        bg_y = rect.y + (rect.height - bg_height) / 2
        draw_pixbuf(cr, self.bg_cache_pixbuf.get_cache(), rect.x + self.bg_offset, bg_y)
        
        # Draw progressbar foreground.
        if self.current_progress_width > 0:
            fg_height = self.fg_dpixbuf.get_pixbuf().get_height()
            # self.fg_cache_pixbuf.scale(self.fg_dpixbuf.get_pixbuf(), 
            #                            int(self.current_progress_width),
            #                            fg_height)
        
            fg_y = rect.y + (rect.height - fg_height) / 2
            # draw_pixbuf(cr, self.fg_cache_pixbuf.get_cache(),  rect.x + self.fg_offset, fg_y)
            
            lg_width = int(self.current_progress_width)
            pat = cairo.LinearGradient(rect.x + self.fg_offset, fg_y, rect.x + self.fg_offset + lg_width, fg_y)
            pat.add_color_stop_rgb(0.6, *color_hex_to_cairo(self.fg_left_dcolor.get_color()))
            pat.add_color_stop_rgb(1.0, *color_hex_to_cairo(self.fg_right_dcolor.get_color()))
            cr.set_operator(cairo.OPERATOR_OVER)
            cr.set_source(pat)
            cr.rectangle(rect.x + self.fg_offset, fg_y, lg_width, fg_height)
            cr.fill()
            
            with cairo_disable_antialias(cr):
                cr.set_line_width(1)
                cr.set_source_rgba(1, 1, 1, 0.3)
                cr.rectangle(rect.x + self.fg_offset, fg_y, lg_width, fg_height)
                cr.stroke()
        
        # Draw point.
        point_y = rect.y + (rect.height - self.point_dpixbuf.get_pixbuf().get_height()) / 2
        draw_pixbuf(cr, self.point_dpixbuf.get_pixbuf(), 
                    rect.x + self.point_offset + self.current_progress_width, 
                    point_y)
            
    def on_enter_notify_event(self, widget, event):
        if self.auto_hide:
            self.hide_progress_flag = False
            self.set_size_request(self.expand_width, self.default_height)
            self.queue_draw()
        
    def hide_progressbar(self):    
        self.hide_progress_flag = True
        self.set_size_request(self.base_width, self.default_height)
        self.icon_state = STATE_NORMAL
        set_cursor(self, None)
        self.queue_draw()
    
    def on_leave_notify_event(self, widget, event):
        if self.drag_flag:
            self.drag_out_area = True
        else:    
            if self.auto_hide:
                self.hide_progressbar()
                
        set_cursor(widget, None)        
    
    def pointer_in_state_icon(self, event):    
        if self.state_icon_rect.x <= event.x <= self.state_icon_rect.x + self.state_icon_rect.width  and \
                self.state_icon_rect.y <= event.y <= self.state_icon_rect.y + self.state_icon_rect.height:
            return True
        return False
        
    def on_button_press_event(self, widget, event):
        if self.pointer_in_state_icon(event):
            self.state_press_flag = True
            self.icon_state = STATE_PRESS
            self.drag_flag = False
        else:    
            self.update_progress_width(event, emit=True)            
            self.state_press_flag = False
            self.drag_flag = True
            self.mute_flag = False
            
        self.queue_draw()
    
    def on_motion_notify_event(self, widget, event):
        if self.pointer_in_state_icon(event):
            self.icon_state = STATE_HOVER
            set_cursor(widget, None)
        else:    
            self.icon_state = STATE_NORMAL
            set_cursor(widget, gtk.gdk.HAND2)
            
        if self.drag_flag:
            self.update_progress_width(event, emit=True)
        self.queue_draw()

    def on_button_release_event(self, widget, event):
        if self.drag_out_area:
            if self.auto_hide:
                self.hide_progressbar()
        self.drag_out_area = False    
        
        if self.state_press_flag:
            if self.mute_flag:
                self.update_state_by_value(emit=False)
                self.mute_flag = False
            else:    
                self.update_state_dpixbufs("mute")
                self.mute_flag = True
            
        self.emit("volume-state-changed", self.get_value(), self.mute_flag)    
                
        self.icon_state = STATE_NORMAL        
        self.state_press_flag = False        
        self.drag_flag = False
        self.queue_draw()
        
    def get_value(self):    
        return self.width_to_value(self.current_progress_width)
    
    def set_value(self, value, emit=True):
        self.current_progress_width = self.value_to_width(value)
        self.update_state_by_value(emit=emit)
        self.queue_draw()
        
    def set_mute(self):
        self.update_state_dpixbufs("mute")
        self.mute_flag = True
        self.queue_draw()
        
    def unset_mute(self):    
        self.mute_flag = False
        self.queue_draw()
        
    def on_scroll_event(self, widget, event):
        self.mute_flag = False
        if event.direction == gtk.gdk.SCROLL_UP:
            self.increase_value()
        elif event.direction == gtk.gdk.SCROLL_DOWN:
            self.decrease_value()
            
    def increase_value(self):    
        self.current_progress_width += self.value_to_width(self.__step)
        if self.current_progress_width > self.progress_width:
            self.current_progress_width = self.progress_width
        self.update_state_by_value()
        self.queue_draw()
            
    def decrease_value(self):        
        self.current_progress_width -= self.value_to_width(self.__step)
        if self.current_progress_width < 0:
            self.current_progress_width = 0
        self.update_state_by_value()
        self.queue_draw()
