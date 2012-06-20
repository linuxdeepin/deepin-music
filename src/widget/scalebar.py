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
from dtk.ui.utils import is_left_button

from widget.skin import app_theme

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
        
        self.set_size_request(-1, self.point_normal_dpixbuf.get_pixbuf().get_height())
        
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
        draw_pixbuf(cr, bg_pixbuf.scale_simple(w + point_width , line_height, gtk.gdk.INTERP_BILINEAR), rect.x, line_y)
        draw_pixbuf(cr, fg_pixbuf.scale_simple(point_width / 2, line_height, gtk.gdk.INTERP_BILINEAR), rect.x, line_y)

        
        if value > 0:
            draw_pixbuf(cr, fg_pixbuf.scale_simple(value + point_width, line_height, gtk.gdk.INTERP_BILINEAR), rect.x, line_y)
            
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

        
