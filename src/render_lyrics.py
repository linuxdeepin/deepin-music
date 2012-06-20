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

import cairo
import pango
import pangocairo
import math
import gtk
import dtk_cairo_blur
from config import config
from utils import color_hex_to_cairo

LINEAR_POS = [0.0, 0.3, 0.8]
LINEAR_COLOR_COUNT = 3
BLACK_COLOR = (0.0, 0.0, 0.0)
LINEAR_COLORS = [BLACK_COLOR, BLACK_COLOR, BLACK_COLOR]

class RenderContextNew(object):
    ''' The new render context. '''
	
    def __init__(self):
        ''' Init. '''
        self.font = self.get_font()
        self.linear_pos = LINEAR_POS
        self.pango_context = gtk.gdk.pango_context_get()
        self.pango_layout = pango.Layout(self.pango_context)
        self.text = ""
        self.linear_colors = LINEAR_COLORS
        self.update_font()
        
    def update_font(self):    
        font_desc = pango.FontDescription(self.font)
        self.pango_layout.set_font_description(font_desc)
        self.update_font_height()
        
    def update_font_height(self):    
        metrics = self.pango_context.get_metrics(self.pango_layout.get_font_description())
        if not metrics:
            print "cannot get font metrics!"
        else:    
            ascent = metrics.get_ascent()
            descent = metrics.get_descent()
            self.font_height = (ascent + descent) / pango.SCALE
            
    def set_font(self, new_font):        
        self.font = new_font
        self.update_font()
        
    def get_font(self):    
        font_name = self.get_font_name()
        font_type = self.get_font_type()
        font_size = self.get_font_size()
        return "%s %s %d" % (font_name, font_type, font_size)
    
    def set_font_name(self, font_name):
        font_type = self.get_font_type()
        font_size = self.get_font_size()
        font_des = pango.FontDescription("%s %s %d" % (font_name, font_type, font_size))
        self.pango_layout.set_font_description(font_des)
        self.update_font_height()
        
    def get_font_name(self):    
        return config.get("lyrics", "font_name")
        
    def set_font_size(self, value):    
        font_name = self.get_font_name()
        font_type = self.get_font_type()
        font_des = pango.FontDescription("%s %s %d" % (font_name, font_type, value))
        self.pango_layout.set_font_description(font_des)
        self.update_font_height()
        
    def get_font_size(self):    
        return int(config.get("lyrics", "font_size", 30))
    
    def set_font_type(self, value):
        font_name = self.get_font_name()
        font_size = self.get_font_size()
        font_des = pango.FontDescription("%s %s %d" % (font_name, value, font_size))
        self.pango_layout.set_font_description(font_des)
        self.update_font_height()
        
    def get_font_type(self):    
        return config.get("lyrics", "font_type", "Regular")
    
    def get_font_height(self):    
        return self.font_height
        
    def get_outline_width(self):    
        return config.getint("lyrics", "outline_width")
    
    def get_blur_radius(self):    
        return config.getint("lyrics", "blur_radius")
    
    def get_blur_color(self):
        return color_hex_to_cairo(config.get("lyrics", "blur_color", "#000000"))
    
    def set_text(self, text):
        self.text = text
        self.pango_layout.set_text(self.text)
        
    def get_pixel_size(self, text):    
        self.set_text(text)
        w, h = self.pango_layout.get_pixel_size()
        new_width = int(w + self.get_outline_width() + self.get_blur_radius() * 2 + 5)
        new_height = int(h + self.get_outline_width() + self.get_blur_radius() * 2)
        return (new_width, new_height)    
    
    def set_linear_color(self, color):
        self.linear_colors = color
        
    def paint_text(self, cr, text, xpos, ypos):   
        self.set_text(text)
        xpos += self.get_outline_width() / 2 + self.get_blur_radius()
        ypos += self.get_outline_width()  + self.get_blur_radius()
        cr = pangocairo.CairoContext(cr)
        width, height = self.get_pixel_size(text)
        # Draw the outline of the text.
        cr.move_to(xpos, ypos)
        cr.save()
        cr.layout_path(self.pango_layout)

        cr.set_source_rgb(*self.get_blur_color())
        
        if self.get_outline_width() > 0:
            cr.set_line_width(self.get_outline_width())
            if self.get_blur_radius() > math.e - 4:
                cr.stroke_preserve()
                cr.fill()
                dtk_cairo_blur.gaussian_blur(cr.get_target(), self.get_blur_radius())
            else:    
                cr.stroke()
        cr.restore()            
        cr.save()
        cr.new_path()
        # Create the linear pattern.
        pattern = cairo.LinearGradient(xpos, ypos, xpos, ypos + height)

        for i, each_linear in enumerate(self.linear_colors):
            pattern.add_color_stop_rgb(self.linear_pos[i],
                                       each_linear[0],
                                       each_linear[1],
                                       each_linear[2])
            
        cr.set_source(pattern)    
        cr.set_operator(cairo.OPERATOR_OVER)
        
        # Draw the text.
        cr.move_to(xpos, ypos)
        cr.show_layout(self.pango_layout)
        cr.restore()
