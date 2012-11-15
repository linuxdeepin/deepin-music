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
import cairo
import pangocairo
import pango

from dtk.ui.utils import color_hex_to_cairo

class SlideSwitcher(gtk.EventBox):
    
    def __init__(self):
        gtk.EventBox.__init__(self)
        
        # Init signals.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.connect("expose-event", self.on_expose_event)
        self.connect("motion-notify-event", self.on_motion_notify)
        self.connect("leave-notify-event", self.on_leave_notify)
        self.connect("enter-notify-event", self.on_enter_notify)
        
        
    def draw_surface(self, image_file, width, height):    
        image_surface = cairo.ImageSurface.create_from_png(image_file)
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, int(width), int(height))
        cr = cairo.Context(surface)
        cr.set_source_surface(image_surface, 0, 0)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        
        cr.set_source_rgb(*color_hex_to_cairo("#EFF5F2"))
        
        
        
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        
        
        
        
        
        