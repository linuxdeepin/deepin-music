#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cairo
import pango
import pangocairo
import math
import gtk
import dtk_cairo_blur

OUTLINE_WIDTH = 5
FONT_NAME = "迷你繁启体 32"
LINEAR_POS = [0.2, 0.9, 1.0]
LINEAR_COLOR_COUNT = 3
BLACK_COLOR = (0.0, 0.0, 0.0)
LINEAR_COLORS = [BLACK_COLOR, BLACK_COLOR, BLACK_COLOR]

class RenderContextNew(object):
    ''' The new render context. '''
	
    def __init__(self):
        ''' Init. '''
        self.font_name = FONT_NAME
        self.linear_pos = LINEAR_POS
        self.pango_context = gtk.gdk.pango_context_get()
        self.pango_layout = pango.Layout(self.pango_context)
        self.text = ""
        self.linear_colors = LINEAR_COLORS
        self.blur_radius = 0.0
        self.outline_width = OUTLINE_WIDTH
        self.update_font()
        
    def update_font(self):    
        font_desc = pango.FontDescription(self.font_name)
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
            
    def set_font_name(self, new_font_name):        
        self.font_name = new_font_name
        self.update_font()
        
    def get_font_height(self):    
        return self.font_height
    
    def set_outline_width(self, width):
        self.outline_width = width
        
    def get_outline_width(self):    
        return self.outline_width
    
    def set_blur_radius(self, radius):
        self.blur_radius = radius
        
    def get_blur_radius(self):    
        return self.blur_radius
    
    def set_text(self, text):
        self.text = text
        self.pango_layout.set_text(self.text)
        
    def get_pixel_size(self, text):    
        self.set_text(text)
        w, h = self.pango_layout.get_pixel_size()
        new_width = int(w + self.outline_width + self.blur_radius * 2)
        new_height = int(h + self.outline_width + self.blur_radius * 2)
        return (new_width, new_height)    
    
    def set_linear_color(self, color):
        self.linear_colors = color
        
    def paint_text(self, cr, text, xpos, ypos):   
        self.set_text(text)
        xpos += self.outline_width / 2 + self.blur_radius
        ypos += self.outline_width / 2 + self.blur_radius
        

        cr = pangocairo.CairoContext(cr)
        width, height = self.get_pixel_size(text)
        # Draw the outline of the text.
        cr.move_to(xpos, ypos)
        cr.save()
        cr.layout_path(self.pango_layout)

        cr.set_source_rgb(*BLACK_COLOR)
        
        if self.outline_width > 0:
            cr.set_line_width(self.outline_width)
            if self.blur_radius > math.e - 4:
                cr.stroke_preserve()
                cr.fill()
                dtk_cairo_blur.gaussian_blur(cr.get_target(), self.blur_radius)
            else:    
                cr.stroke()
        cr.restore()            
        cr.save()
        cr.new_path()
        # Create the linear pattern.
        pattern = cairo.LinearGradient(xpos, ypos, xpos, ypos + height)
        for i, each_linear in enumerate(self.linear_colors):
            pattern.add_color_stop_rgb(self.linear_pos[i-1],
                                       each_linear[0],
                                       each_linear[1],
                                       each_linear[2])
        cr.set_source(pattern)    
        cr.set_operator(cairo.OPERATOR_OVER)
        
        # Draw the text.
        cr.move_to(xpos, ypos)
        cr.show_layout(self.pango_layout)
        cr.restore()

        
def surface_gaussion_blur(surface, radious):
    '''Gaussian blur surface.'''
    surface.flush()
    width = surface.get_width()
    height = surface.get_height()
    
    tmp = cairo.ImageSurface(cairo.FORMAT_ARGB32, width, height);
    
    src = surface.get_data()    
    src_stride = surface.get_stride()
    
    print type(src)
    print type(src_stride)
    
    dst = tmp.get_data()
    dst_stride = tmp.get_stride()
    
    size = 17
    half = 17 / 2

    kernel = range(0, size)
    a = 0
    for i in range(0, size):
        f = i - half
        kernel[i] = math.exp(-f * f / 30.0) * 80
        a += kernel[i]
        
    for i in range(0, height):
        s = src + i * src_stride
        d = dst + i * dst_stride
        for j in range(0, width):
            if radious < j and j < width - radious:
                continue

            x = y = z = w = 0
            for k in range(0, size):
                if j - half + k < 0 or j - half + k >= width:
                    continue
                
                p = s[j - half + k]
                x += ((p >> 24) & 0xff) * kernel[k]
                y += ((p >> 16) & 0xff) * kernel[k]
                z += ((p >> 8) & 0xff) * kernel[k]
                w += ((p >> 0) & 0xff) * kernel[k]
                
            d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a

    for i in range(0, height):
        s = dst + i * dst_stride
        d = src + i * src_stride
        for j in range(0, width):
            if radious <= i and i < height - radious:
                d[j] = s[j]
                continue

            x = y = z = w = 0
            for k in range(0, size):
                if i - half + k < 0 or i - half + k >= height:
                    continue
                
                s = dst + (i - half + k) * dst_stride
                p = s[j]
                
                x += ((p >> 24) & 0xff) * kernel[k]
                y += ((p >> 16) & 0xff) * kernel[k]
                z += ((p >> 8) & 0xff) * kernel[k]
                w += ((p >> 0) & 0xff * kernel[k])
                
            d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a    
            
    surface.mark_dirty()

        