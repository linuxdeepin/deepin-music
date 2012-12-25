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

from dtk.ui.window import Window
from dtk.ui.utils import get_content_size, cairo_disable_antialias
from dtk.ui.draw import draw_text,draw_pixbuf, draw_round_rectangle
from dtk.ui.timeline import Timeline, CURVE_SINE

import utils
from cover_manager import BROWSER_COVER_SIZE, CoverManager

class SongNotify(gtk.Window):    
    
    def __init__(self, song=None):
        gtk.Window.__init__(self, 
                        # shadow_visible=False,
                        # shadow_radius=0,
                        # shape_frame_function=self.shape_panel_frame,
                        # expose_frame_function=self.expose_panel_frame,
                        # window_type=gtk.WINDOW_POPUP,
                        )
        
        self.set_can_focus(False)
        self.set_accept_focus(False)
        self.set_keep_above(True)        
        self.set_skip_taskbar_hint(True)
        self.set_skip_pager_hint(True)        
        self.set_decorated(False)
        self.set_app_paintable(True)
        self.set_colormap(gtk.gdk.Screen().get_rgba_colormap() or gtk.gdk.Screen().get_rga_colormap())
        
        self.connect("expose-event", self.on_panel_expose_event)
        self.connect("size-allocate", self.on_size_allocate)
        self.song = song
        self.text_padding_y = 10
        self.text_padding_x = 5
        self.cover_padding_x = 10
        self.line_height = 15
        self.cover_width = self.cover_height = BROWSER_COVER_SIZE["x"]
        self.default_width = 200
        self.default_height = 60
        self.set_size_request(self.default_width, self.default_height)
        
        # animation params.
        self.width_percent = 1.0
        self.in_animation = False
        self.animation_time = 500
        self.animation_timeout_id = None
        self.last_x = None
        self.last_y = None

        
    def shape_panel_frame(self, widget, event):    
        pass
        
    def expose_panel_frame(self, widget, event):
        pass
    
    def on_size_allocate(self, widget, rect):
        x, y, w, h = rect.x, rect.y, rect.width, rect.height
        bitmap = gtk.gdk.Pixmap(None, w, h, 1)
        cr = bitmap.cairo_create()

        cr.set_source_rgb(0.0, 0.0, 0.0)
        cr.set_operator(cairo.OPERATOR_CLEAR)
        cr.paint()
        
        cr.set_operator(cairo.OPERATOR_OVER)
        with cairo_disable_antialias(cr):
            draw_round_rectangle(cr, rect.x, rect.y, rect.width, rect.height, 4)
            cr.fill()
        widget.shape_combine_mask(bitmap, 0, 0)        
        
    def on_panel_expose_event(self, widget, event):    
        cr  = widget.window.cairo_create()
        rect = widget.allocation

        
        # Clear color to transparent window.
        if self.is_composited():
            draw_round_rectangle(cr, rect.x, rect.y, rect.width, rect.height, 2)
            cr.set_source_rgba(0.14, 0.13, 0.15, 0.95)
            cr.set_operator(cairo.OPERATOR_SOURCE)
            cr.paint()
        else:    
            cr.rectangle(rect.x, rect.y, rect.width, rect.height)            
            cr.set_operator(cairo.OPERATOR_SOURCE)
            cr.set_source_rgb(0.9, 0.9, 0.9)
            cr.fill()
        
        if not self.song:
            return
        
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        cover_pixbuf = self.get_song_cover()
        
        rect.x += self.cover_padding_x
        rect.width -= self.cover_padding_x * 2
        
        rect.y += (rect.height - self.cover_height) / 2
        draw_pixbuf(cr, cover_pixbuf, rect.x, rect.y)
        
        rect.x += self.cover_width + self.text_padding_x
        rect.width -= (self.cover_width + self.text_padding_x)
        
        title = utils.xmlescape(self.song.get_str("title"))
        _width, _height = get_content_size(title)        
        draw_text(cr, title, rect.x, rect.y, rect.width, _height, 
                  text_color="#FFFFFF")        
        
        rect.y += _height + self.text_padding_y
        artist = utils.xmlescape(self.song.get_str("artist"))
        _width, _height = get_content_size(artist)
        draw_text(cr, artist, rect.x, rect.y, rect.width, _height,
                  text_color="#FFFFFF")
        
        return True
    
    def get_song_cover(self):
        return CoverManager.get_pixbuf_from_song(self.song, 
                                                BROWSER_COVER_SIZE["x"], 
                                                BROWSER_COVER_SIZE["y"],
                                                try_web=False,
                                                optimum=True)
    def show(self, x, y):
        self.show_all()
        if self.last_x is None or self.last_x != x:
            self.last_x = x
            self.last_y = y
            self.move(x, y)
        else:    
            self.start_animation(x, y)
        
    def start_animation(self, x, y):    
        if not self.in_animation:
            self.in_animation = False
            try:
                self.timeline.stop()
            except:    
                pass
            
            self.timeline = Timeline(self.animation_time, CURVE_SINE)
            self.timeline.connect("update", self.update_animation, x, y)
            self.timeline.connect("completed", self.completed_animation, x, y)
            self.timeline.run()
            
    def update_animation(self, source, status, x, y):        
        height = y - self.last_y
        new_height = status * height
        self.move(x, int(self.last_y + new_height))
    
    def completed_animation(self, source, x, y):    
        self.last_x = x
        self.last_y = y
        
    def hide_notify(self):    
        self.last_x = None
        self.hide_all()
        
    def update_song(self, song):    
        self.song = song
        self.queue_draw()
