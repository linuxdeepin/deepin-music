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

from dtk.ui.utils import get_content_size, cairo_disable_antialias, color_hex_to_cairo
from dtk.ui.draw import draw_text,draw_pixbuf, draw_round_rectangle
from dtk.ui.timeline import Timeline, CURVE_SINE
from widget.skin import app_theme
from nls import _

import utils
from cover_manager import CoverManager
BROWSER_COVER_SIZE = {"x" : 50, "y" : 50}

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
        # self.connect("size-allocate", self.on_size_allocate)
        self.song = song
        self.title_text_size = 9
        self.other_text_size = 8
        self.text_padding_y = 5
        self.other_text_padding_y = 2
        self.text_padding_x = 5
        self.cover_padding_x = 7
        self.cover_up_offset = 12
        self.cover_bottom_offset = 10
        self.other_up_offset = 8
        self.line_height = 15
        self.cover_width = self.cover_height = BROWSER_COVER_SIZE["x"]
        self.default_width = 222
        self.default_height = 118
        self.set_size_request(self.default_width, self.default_height)
        self.other_text_color = app_theme.get_color("labelText").get_color()
        
        # animation params.
        self.width_percent = 1.0
        self.in_animation = False
        self.animation_time = 500
        self.animation_timeout_id = None
        self.last_x = None
        self.last_y = None
        self.move_y = None

        
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
            cr.rectangle(*rect)
            cr.set_source_rgba(1, 1, 1, 0.95)
            cr.set_operator(cairo.OPERATOR_SOURCE)
            cr.paint()

        else:    
            cr.rectangle(rect.x, rect.y, rect.width, rect.height)            
            cr.set_operator(cairo.OPERATOR_SOURCE)
            cr.set_source_rgb(0.9, 0.9, 0.9)
            cr.fill()
         
        cr  = widget.window.cairo_create()
        rect = widget.allocation
        
        with cairo_disable_antialias(cr):    
            cr.rectangle(rect.x + 1, rect.y + 1, rect.width - 1, rect.height - 1)
            cr.set_line_width(1)
            cr.set_source_rgb(*color_hex_to_cairo("#D6D6D6"))
            cr.stroke()
            
        if not self.song:
            return True
        
        dash_y = rect.y + self.cover_height + self.cover_up_offset + self.cover_bottom_offset + 1
        dash_x = rect.x
        dash_width = rect.width
                
        cover_pixbuf = self.get_song_cover()
        
        rect.x += self.cover_padding_x
        rect.width -= self.cover_padding_x * 2
        
        # rect.y += (rect.height - self.cover_height) / 2
        rect.y += self.cover_up_offset
        draw_pixbuf(cr, cover_pixbuf, rect.x, rect.y)
        
        rect.x += self.cover_width + self.text_padding_x
        rect.width -= (self.cover_width + self.text_padding_x)
        
        title = utils.xmlescape(self.song.get_str("title"))
        artist = utils.xmlescape(self.song.get_str("artist"))        
        album = utils.xmlescape(self.song.get_str("album"))
        
        _width, _height = get_content_size(title, text_size=self.title_text_size)                
        if not artist and not album:
            _height = self.cover_height
        draw_text(cr, title, rect.x, rect.y, rect.width, _height, 
                  text_color=self.other_text_color, 
                  text_size=self.title_text_size)        
        
        if artist:
            rect.y += _height + self.text_padding_y
            _width, _height = get_content_size(artist, text_size=self.other_text_size)
            draw_text(cr, artist, rect.x, rect.y, rect.width, _height,
                      text_size=self.other_text_size,
                      text_color=self.other_text_color)
            
        if album:    
            rect.y += _height + self.text_padding_y
            _width, _height = get_content_size(artist, text_size=self.other_text_size)
            draw_text(cr, album, rect.x, rect.y, rect.width, _height,
                      text_size=self.other_text_size,
                      text_color=self.other_text_color)
            
        # draw dash    
        with cairo_disable_antialias(cr):    
            cr.set_source_rgb(*color_hex_to_cairo("#D6D6D6"))
            cr.set_line_width(1)            
            cr.move_to(dash_x, dash_y)
            cr.rel_line_to(dash_width, 0)
            # cr.set_dash([2.0, 2.0])
            cr.stroke()
            
            
        other_width = (dash_width - self.cover_padding_x * 2) / 2 - self.cover_padding_x / 2
        other_left_x = dash_x + self.cover_padding_x
        other_right_x = dash_x + dash_width / 2 + self.cover_padding_x / 2
        
        # draw size and format    
        other_y = dash_y + self.other_up_offset
        song_size = "%s: %s" % (_("Size"), utils.xmlescape(self.song.get_str("#size")))
        song_format = "%s: %s" % (_("Type"), utils.get_song_attr(self.song))
        _width, _height = get_content_size(song_size, text_size=self.other_text_size)
        draw_text(cr, song_size, other_left_x, other_y, other_width, _height, 
                  text_size=self.other_text_size,
                  text_color=self.other_text_color
                  )
        draw_text(cr, song_format, other_right_x, other_y, other_width, _height,
                  text_size=self.other_text_size,
                  text_color=self.other_text_color
                  )
        
        # draw playcount and duration.
        other_y += _height + self.other_text_padding_y
        playcount = self.song.get_str("#playcount")
        if playcount == "Never" or not playcount: playcount = "0"
        song_playcount = "%s: %s" % (_("Playcount"), playcount)
        duration = "%s: %s" % (_("Duration"), self.song.get_str("#duration"))
        
        draw_text(cr, song_playcount, other_left_x, other_y, other_width, _height, 
                  text_size=self.other_text_size,
                  text_color=self.other_text_color
                  )
        draw_text(cr, duration, other_right_x, other_y, other_width, _height,
                  text_size=self.other_text_size,
                  text_color=self.other_text_color
                  )
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
            self.timeline.connect("stop", self.stop_animation)
            self.timeline.connect("update", self.update_animation, x, y)
            self.timeline.connect("completed", self.completed_animation, x, y)
            self.timeline.run()
            
    def stop_animation(self, source):        
        if self.move_y is not None:
            self.last_y = self.move_y
            
    def update_animation(self, source, status, x, y):        
        height = y - self.last_y
        new_height = status * height
        self.move_y = int(self.last_y + new_height) 
        self.move(x, self.move_y)
    
    def completed_animation(self, source, x, y):    
        self.last_x = x
        self.last_y = y
        
    def hide_notify(self):    
        self.last_x = None
        self.hide_all()
        
    def update_song(self, song):    
        self.song = song
        self.queue_draw()
