#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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

import gtk
import cairo
import gobject

from dtk.ui.button import ImageButton
from dtk.ui.draw import draw_text
from dtk.ui.dialog import DialogBox, DIALOG_MASK_GLASS_PAGE

from widget.skin import app_theme
from config import config
from render_lyrics import RenderContextNew
from utils import color_hex_to_cairo
from ui_utils import cairo_color_to_hex
from helper import Dispatcher
from nls import _
from constant import PROGRAM_NAME_LONG

# drag state.
DRAG_NONE = 1
DRAG_MOVE = 2
DRAG_EAST = 3
DRAG_WEST = 4
MIN_WIDTH = 400


LINE_ALIGNMENT = { "left"  : (0.0, 0.0),  "centered" : (0.5, 0.5),
                      "right" : (1.0, 1.0), "justified" : (0.0, 1.0)}

class DesktopLyrics(gtk.Window):
    __gsignals__ = {
        "moved" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "resized" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "hide-bg" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "show-bg" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        }
    def __init__(self):
        gtk.Window.__init__(self, gtk.WINDOW_POPUP)
        self.set_property("allow-shrink", True)
        self.set_skip_taskbar_hint(True)
        self.set_decorated(False)
        self.set_skip_pager_hint(True)
        self.set_app_paintable(True)
        self.set_colormap(gtk.gdk.Screen().get_rgba_colormap())
        self.render_lyrics = RenderContextNew()
        self.bg_pixbuf = app_theme.get_pixbuf("lyric/bg.png").get_pixbuf()
        self.line_padding = 0.0
        self.dock_drag_state = DRAG_NONE
        self.padding_x = self.padding_y = 10
        self.old_x = self.old_y = self.old_width = 0
        self.mouse_x = self.mouse_y = 0
        self.raw_x, self.raw_y = self.get_position()
        self.mouse_over = False
        self.mouse_over_lyrics = False
        self.fade_in_size = 20.0
        self.max_line_count = 2
        
        self.active_lyric_surfaces = [None, None]
        self.inactive_lyric_surfaces = [None, None]
        self.lyrics_text = [PROGRAM_NAME_LONG, "for Deepin"]
        self.lyric_rects = [gtk.gdk.Rectangle(0, 0, 0, 0), gtk.gdk.Rectangle(0, 0, 0, 0)]
        self.lyrics_xpos = [0, 0]
        self.line_alignment = LINE_ALIGNMENT[config.get("lyrics", "double_line_align")]
        self.line_percentage = [0.0, 0.0]
        self.current_line = 0
        self.time_source = 0
        
        for i in range(self.get_line_count()):
            self.update_lyric_surface(i)
         
        width = self.adjust_window_height()
        self.set_default_size(600, int( width))           
        # Add events.
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                                   gtk.gdk.BUTTON_RELEASE_MASK |
                                   gtk.gdk.POINTER_MOTION_MASK |
                                   gtk.gdk.ENTER_NOTIFY_MASK |
                                   gtk.gdk.LEAVE_NOTIFY_MASK)
        
        self.connect("button-press-event", self.button_press) # TRY
        self.connect("button-release-event", self.button_release) 
        self.connect("motion-notify-event", self.motion_notify)   
        self.connect("enter-notify-event", self.enter_notify)
        self.connect("leave-notify-event", self.leave_notify)
        self.connect("expose-event", self.expose_before)     
        self.connect("realize", self.on_realize_event)
        config.connect("config-changed", self.update_render_color)
        self.time_source = gobject.timeout_add(200, self.check_mouse_leave)        

        
    def update_render_color(self, obj, selection, option, value):    
        color_option  = ["inactive_color_upper", " inactive_color_middle", "inactive_color_bottom",
                         "active_color_upper", "active_color_middle", "active_color_bottom"]
        if selection == "lyrics" and option in color_option:
            self.update_font()
                
        if selection == "lyrics" and option == "status":        
            status = config.getboolean("lyrics", "status")
            if not status:
                if self.time_source != 0:
                    gobject.source_remove(self.time_source)
            else:    
                lrc_mode = config.getint("lyrics", "mode")
                if lrc_mode == 1:
                    self.time_source = gobject.timeout_add(200, self.check_mouse_leave)        
                    
        if selection == "lyrics" and option == "mode":            
            lrc_mode = config.getint("lyrics", "mode")
            if lrc_mode != 1:
                if self.time_source != 0:
                    gobject.source_remove(self.time_source)
            else:        
                status = config.getboolean("lyrics", "status")
                if status:
                    self.time_source = gobject.timeout_add(200, self.check_mouse_leave)
                    
        if selection == "lyrics" and option == "font_name":            
            self.set_font_name(value)
            
        if selection == "lyrics" and option == "font_size":    
            self.set_font_size(int(value))
            
        if selection == "lyrics" and option == "font_type":
            self.set_font_type(value)
            
        if selection == "lyrics" and option == "line_count":    
            if value == "1":
                self.line_alignment = LINE_ALIGNMENT[config.get("lyrics", "single_line_align")]
            elif value == "2":    
                self.line_alignment = LINE_ALIGNMENT[config.get("lyrics", "double_line_align")]
            self.line_count = int(value)    
            self.update_font()
            
        if selection == "lyrics" and option in ["single_line_align", "double_line_align"]:    
            self.line_alignment = LINE_ALIGNMENT[config.get("lyrics", option)]
            self.update_font()
            
        if selection == "lyrics" and option in ["outline_width", "blur_color"]:    
            self.update_font()
            
    def get_render_color(self, active=False):        
        if active:
            return [color_hex_to_cairo(config.get("lyrics", "active_color_upper")),
                    color_hex_to_cairo(config.get("lyrics", "active_color_middle")),
                    color_hex_to_cairo(config.get("lyrics", "active_color_bottom"))]
        else:
            return [color_hex_to_cairo(config.get("lyrics", "inactive_color_upper")),
                    color_hex_to_cairo(config.get("lyrics", "inactive_color_middle")),
                    color_hex_to_cairo(config.get("lyrics", "inactive_color_bottom"))]
            
    def get_locked(self):        
        return config.getboolean("lyrics", "locked")
        
    def set_locked(self, locked=True):    
        if locked:
            config.set("lyrics", "locked", "true")
            self.set_input_shape_mask(True)
            self.emit("hide-bg")
            self.queue_draw()
        else:    
            config.set("lyrics", "locked", "false")
            self.set_input_shape_mask(False)
            
    def on_realize_event(self, widget):
        if self.get_locked():
            self.set_input_shape_mask(True)
            
    def update_font(self):        
        for i in range(self.get_line_count()):
            self.update_lyric_surface(i)
        self.queue_draw()        
        x, y = self.get_position()
        w, h = self.get_size()
        rect = gtk.gdk.Rectangle(int(x), int(y), int(w), int(h))
        self.move_resize(self, rect, DRAG_NONE)
        
    def set_dock_mode(self, value):
        if config.getboolean("lyrics", "dock_mode"):
            config.set("lyrics", "dock_mode", "false")
            self.set_type_hint(gtk.WINDOW_TYPE_HINT_NORMAL)
        else:    
            config.set("lyrics", "dock_mode", "true")
            self.set_type_hint(gtk.WINDOW_TYPE_HINT_DOCK)
    
    def get_dock_mode(self):
        return config.getboolean("lyrics", "dock_mode")
    
    def get_line_count(self):
        return config.getint("lyrics", "line_count")
    
    def set_line_count(self, value):
        if value in [1, 2]:    
            config.set("lyrics", "line_count", str(value))
            
    def get_karaoke_mode(self):    
        return config.getboolean("lyrics", "karaoke_mode")
    
    def set_karaoke_mode(self):
        if not self.get_karaoke_mode():
            config.set("lyrics", "karaoke_mode", "true")
        else:    
            config.set("lyrics", "karaoke_mode", "false")
            self.line_percentage = [0.0, 0.0]
            for i in range(self.get_line_count()):
                self.update_lyric_surface(i)
            self.queue_draw()    
        
    def get_blur_radius(self):        
        return config.getint("lyrics", "blur_radius")
    
    def set_blur_radius(self, value):
        config.set("lyrics", "blur_radius", str(value))
        
    def get_translucent_on_mouse_over(self):
        return config.getboolean("lyrics", "translucent_on_mouse_over")
    
    def set_translucent_on_mouse_over(self, value):
        if config.getboolean("lyrics", "translucent_on_mouse_over"):
            config.set("lyrics", "translucent_on_mouse_over", "false")
        else:    
            config.set("lyrics", "translucent_on_mouse_over", "true")
            
    def __paint_rect(self, cr, source, src_x, src_y, src_w, src_h,
                     des_x, des_y, des_w, des_h):
        ''' paint rect. '''
        cr.save()
        sw = float(des_w) / float(src_w)
        sh = float(des_h) / float(src_h)
        cr.translate(des_x, des_y)
        cr.rectangle(0, 0, des_w, des_h)
        cr.scale(sw, sh)        
        cr.clip()
        cr.set_source_pixbuf(source,  -src_x,  -src_y)
        cr.paint()
        cr.restore()
    
    def draw_bg_pixbuf(self, widget, cr):
        ''' Paint window bg. '''
        w, h = widget.get_size()
        BORDER_WIDTH = self.padding_x
        if self.is_composited():
            cr.set_operator(cairo.OPERATOR_OVER)        
            sw = self.bg_pixbuf.get_width()
            sh = self.bg_pixbuf.get_height()
            
            self.__paint_rect(cr, self.bg_pixbuf,
                              0, 0, BORDER_WIDTH, BORDER_WIDTH,
                              0, 0, BORDER_WIDTH, BORDER_WIDTH) 
            self.__paint_rect(cr, self.bg_pixbuf,
                              0, sh - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH,
                              0, h - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH)
            self.__paint_rect(cr, self.bg_pixbuf,
                              sw - BORDER_WIDTH, 0, BORDER_WIDTH, BORDER_WIDTH,
                              w - BORDER_WIDTH, 0, BORDER_WIDTH, BORDER_WIDTH)
            self.__paint_rect(cr, self.bg_pixbuf,
                              sw - BORDER_WIDTH, sh - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH,
                              w - BORDER_WIDTH, h - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH)
            self.__paint_rect(cr, self.bg_pixbuf,
                              0, BORDER_WIDTH, BORDER_WIDTH, sh - BORDER_WIDTH * 2,
                              0, BORDER_WIDTH, BORDER_WIDTH, h - BORDER_WIDTH * 2)
            self.__paint_rect(cr, self.bg_pixbuf,
                              sw - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH, sh - BORDER_WIDTH * 2,
                              w - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH, h - BORDER_WIDTH * 2)
            self.__paint_rect(cr, self.bg_pixbuf,
                              BORDER_WIDTH, 0, sw - BORDER_WIDTH * 2, BORDER_WIDTH,
                              BORDER_WIDTH, 0, w - BORDER_WIDTH * 2, BORDER_WIDTH)
            self.__paint_rect(cr, self.bg_pixbuf,
                              BORDER_WIDTH, sh - BORDER_WIDTH, sw - BORDER_WIDTH * 2, BORDER_WIDTH,
                              BORDER_WIDTH, h - BORDER_WIDTH, w - BORDER_WIDTH * 2, BORDER_WIDTH)
            self.__paint_rect(cr, self.bg_pixbuf,
                              BORDER_WIDTH, BORDER_WIDTH, sw - BORDER_WIDTH * 2, sh - BORDER_WIDTH * 2,
                              BORDER_WIDTH, BORDER_WIDTH, w - BORDER_WIDTH * 2, h - BORDER_WIDTH * 2)
        else:    
            widget.get_style().paint_box(widget.window, gtk.STATE_NORMAL, 
                                              gtk.SHADOW_IN, None, widget.get_default_widget(), 
                                              "buttondefalut", 0, 0, w, h)
        
        
    def draw_window_background(self, widget, event):    
        
        # Init
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        # Clear color to transparent window.
        if self.is_composited():
            cr.set_source_rgba(1.0, 1.0, 1.0, 0.0)
            cr.set_operator(cairo.OPERATOR_SOURCE)
            cr.paint()
        else:    
            cr.set_operator(cairo.OPERATOR_SOURCE)
            cr.set_source_rgb(0.9, 0.9, 0.9)
            cr.rectangle(0, 0, rect.width, rect.height)
            cr.fill()
            
        if self.mouse_over and not self.get_locked():    
            self.draw_bg_pixbuf(widget, cr)
        return True
            
    def adjust_lyrics_height(self):        
        font_height = self.render_lyrics.get_font_height()
        height = font_height * self.get_line_count() + (self.get_line_count() - 1) * self.line_padding + self.render_lyrics.get_outline_width()        
        if self.get_blur_radius():        
            height += self.get_blur_radius() * 2.0
        return height    
    
    def adjust_lyric_xpos(self, line, percentage):
        smooth = True
        w, h = self.get_lyrics_size()
        if self.active_lyric_surfaces[line] != None:
            width = self.active_lyric_surfaces[line].get_width()
        else:    
            width = 0
            
        if w >= width:    
            xpos = (w - width) * self.line_alignment[line]
        else:    
            if not self.is_composited(): # self.get_dock_mode()
                smooth = False
            if smooth:    
                if percentage * width < w / 2.0:
                    xpos = 0
                elif (1.0 - percentage) * width < w / 2.0:    
                    xpos = w - width
                else:    
                    xpos = w / 2.0 - width * percentage
            else:        
                if percentage * width < w:
                    xpos = 0
                else:    
                    half_count = (percentage * width - w) / w + 1
                    xpos = -half_count * w
                    if xpos < w - width:
                        xpos = w - width
                if xpos != self.lyrics_xpos[line]:        
                    self.lyrics_xpos[line] = xpos
        return xpos            
        
    def adjust_lyric_ypos(self):    
        return self.padding_x
    
    def adjust_window_height(self):
        return self.adjust_lyrics_height() + self.padding_x * 2
    
    def get_lyrics_size(self):
        width = self.allocation.width - self.padding_x * 2
        height = self.adjust_lyrics_height()
        return (width, height)
            
    def get_edge_on_point(self, widget, event):
        rect = widget.allocation
        width, height = rect.width, rect.height
        
        if event.y >= 0 and event.y <= height:
            if event.x >=0 and event.x < self.padding_x:
                return gtk.gdk.WINDOW_EDGE_WEST
            if event.x >= width - self.padding_x and event.x < width: 
                return gtk.gdk.WINDOW_EDGE_EAST
            
    def get_min_width(self):        
        return MIN_WIDTH
    
    def adjust_move_coordinate(self, widget, x, y):
        screen = widget.get_screen()
        w, h = widget.get_size()
        screen_w, screen_h = screen.get_width(), screen.get_height()
        
        if x + w > screen_w:
            x = screen_w - w
           
        if y + h > screen_h:    
            y = screen_h - h
            
        return (int(x), int(y))
            
            
    def move_resize(self, widget, rect, drag_state):        
        old_x, old_y = widget.get_position()
        old_w, old_h = widget.get_size()
        screen = widget.get_screen()
        screen_w, screen_h = screen.get_width(), screen.get_height()
        min_width = self.get_min_width()
        
        if drag_state == DRAG_EAST:
            new_x = max(0, rect.x)
            new_width = max(min(rect.width, screen_w - new_x), min_width)
        elif drag_state == DRAG_WEST:    
            new_width = max(min(rect.width, rect.x + rect.width), min_width)
            new_x = max(0, min(rect.x, self.old_x + self.old_width - new_width))
        else:    
            new_x = max(0, min(rect.x, screen_w - rect.width))
            new_width = max(rect.width, MIN_WIDTH)
            
        if rect.y + rect.height > screen_h:    
            min_h = self.adjust_window_height()
            new_height = max(min_h, screen_h - rect.y)
            new_y = max (0, screen_h - new_height)
        else:    
            new_y = max(0, rect.y)
            new_height = self.adjust_window_height()

        self.raw_x, self.raw_y = new_x, new_y    
        
        rect = gtk.gdk.Rectangle(int(new_x), int(new_y), int(new_width), int(new_height))
        self.emit("resized", rect)
        widget.resize(int(new_width), int(new_height))
        widget.move(int(new_x), int(new_y))           
        
        widget.queue_draw()
            
    def button_press(self, widget, event):        
        '''Button press callback.'''
        if event.button == 1 and not self.get_locked():
            edge = self.get_edge_on_point(widget, event)
            if not self.get_dock_mode():
                if edge == gtk.gdk.WINDOW_EDGE_EAST or edge == gtk.gdk.WINDOW_EDGE_WEST:
                    widget.begin_resize_drag(edge, event.button, int(event.x_root), int(event.y_root), event.time)
                else:    
                    widget.begin_move_drag(event.button, int(event.x_root), int(event.y_root), event.time)
            else:        
                self.old_width, _ = widget.get_size()
                self.old_x, self.old_y = widget.get_position()
                self.mouse_x, self.mouse_y = event.x_root, event.y_root
                
                if edge == gtk.gdk.WINDOW_EDGE_EAST:
                    self.dock_drag_state = DRAG_EAST
                elif edge == gtk.gdk.WINDOW_EDGE_WEST:    
                    self.dock_drag_state = DRAG_WEST
                else:    
                    self.dock_drag_state = DRAG_MOVE
        return False            
    
    def motion_notify(self, widget, event):
        x = max(self.old_x + (event.x_root - self.mouse_x), 0)
        y = max(self.old_y + (event.y_root - self.mouse_y), 0)
        width, height = widget.get_size()
        if self.dock_drag_state == DRAG_MOVE:
            new_x, new_y = self.adjust_move_coordinate(widget, x, y)
            widget.move(new_x, new_y)
            emit_rect = gtk.gdk.Rectangle(int(new_x), int(new_y), int(width), int(height) )           
            self.emit("moved", emit_rect)

        elif self.dock_drag_state == DRAG_EAST:    
            rect = gtk.gdk.Rectangle(int(self.old_x), int(self.old_y), int(self.old_width + (event.x_root - self.mouse_x)), int(height))
            self.move_resize(widget, rect, DRAG_EAST)
        elif self.dock_drag_state == DRAG_WEST:
            rect = gtk.gdk.Rectangle(int(x), int(self.old_y), int(self.old_width + self.old_x - x), int(height))
            self.move_resize(widget, rect, DRAG_WEST)
        elif self.dock_drag_state == DRAG_NONE:    
            edge = self.get_edge_on_point(widget, event)
            if edge == gtk.gdk.WINDOW_EDGE_EAST:
                cursor = gtk.gdk.RIGHT_SIDE
                widget.window.set_cursor(gtk.gdk.Cursor(cursor))
            elif edge == gtk.gdk.WINDOW_EDGE_WEST:    
                cursor = gtk.gdk.LEFT_SIDE
                widget.window.set_cursor(gtk.gdk.Cursor(cursor))
            else:    
                widget.window.set_cursor(None)
        return False        
    
    def button_release(self, widget, event):
        x, y = widget.get_position()
        self.dock_drag_state = DRAG_NONE    
        return False

    def enter_notify(self, widget, event):
        self.mouse_over = True
        self.emit("show-bg")
        widget.queue_draw()
        
    def leave_notify(self, widget, event):    
        pass
        
    def expose_before(self, widget, event):    
        cr = widget.window.cairo_create()
        self.draw_window_background(widget, event)
        self.draw_lyrics(cr)
        return True
        
    def draw_lyric_surface(self, lyrics):
        if not lyrics:
            return
        width, height = self.render_lyrics.get_pixel_size(lyrics)
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, int(width), int(height))
        cr = cairo.Context(surface)
        cr.set_source_rgba(1.0, 1.0, 1.0, 0.0)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()
        self.render_lyrics.paint_text(cr, lyrics, 0, 0)
        return surface
    
    def update_lyric_rects(self):
        for i in range(self.get_line_count()):
            self.update_lyric_rect(i)
      
    def update_lyric_rect(self, line):
        w = h = 0
        x, y = self.get_position()
        if self.active_lyric_surfaces[line] != None:
            w = self.active_lyric_surfaces[line].get_width()
            h = self.active_lyric_surfaces[line].get_height()
        font_height = self.render_lyrics.get_font_height()    
        self.lyric_rects[line] = gtk.gdk.Rectangle(
            int(x + self.adjust_lyric_xpos(line, self.line_percentage[line])),
            int(y + font_height * line + (line + 1) * self.line_padding), int(w), int(h))
        
    def update_lyric_surface(self, line):    
        self.render_lyrics.set_linear_color(self.get_render_color())
        self.inactive_lyric_surfaces[line] = self.draw_lyric_surface(self.lyrics_text[line])
        
        self.render_lyrics.set_linear_color(self.get_render_color(True))
        self.active_lyric_surfaces[line] = self.draw_lyric_surface(self.lyrics_text[line])
        self.update_lyric_rect(line)
        
    def create_text_mask(self, line, text_xpos, alpha):
        width, _ = self.get_lyrics_size()
        text_width = self.active_lyric_surfaces[line].get_width()
        if self.fade_in_size * 2 > width:
            self.fade_in_size = width / 2
        if self.is_composited():    
            pattern = cairo.LinearGradient(self.padding_x, 0.0, self.padding_x + width, 0.0)
            
            # Set fade in on left edge.
            if text_xpos < self.padding_x:
                pattern.add_color_stop_rgba(0.0, 1.0, 1.0, 1.0, 0.0)
                loffset = 0.0
                if self.padding_x - text_xpos < self.fade_in_size:
                    loffset = float(self.padding_x - text_xpos) / float(width)
                else:    
                    loffset = float(self.fade_in_size) / float(width)
                    
                pattern.add_color_stop_rgba(loffset, 1.0, 1.0, 1.0, alpha)    
            else:    
                pattern.add_color_stop_rgba(0.0, 1.0, 1.0, 1.0, alpha)
                
            # Set fade out on the right edge    
            if text_xpos + text_width > self.padding_x + width:    
                roffset = 0.0
                if text_xpos + text_width - (self.padding_x + width) < self.fade_in_size:
                    roffset = 1.0  - (text_xpos + text_width - (self.padding_x + width)) / width
                else:    
                    roffset = 1.0 - float(self.fade_in_size) / width
                pattern.add_color_stop_rgba(roffset, 0.0, 0.0, 0.0, alpha)    
                pattern.add_color_stop_rgba(1.0, 0.0, 0.0, 0.0, 0.0)
            else:    
                pattern.add_color_stop_rgba(1.0, 0.0, 0.0, 0.0, alpha)
            return pattern    
        return None
            
        
    def draw_lyrics(self, cr):    
        alpha = 0.99
        font_height = self.render_lyrics.get_font_height()
        if self.is_composited() and self.get_locked() and self.mouse_over_lyrics and self.get_translucent_on_mouse_over():
            alpha = 0.3
        w, h = self.get_lyrics_size()    
        ypos = self.adjust_lyric_ypos()
        if self.get_line_count() == 1:
            start = self.current_line
            end = start + 1
        else:    
            start = 0
            end = self.max_line_count
        cr.save()    
        cr.rectangle(self.padding_x, self.padding_y, w, h)
        cr.clip()
        cr.set_operator(cairo.OPERATOR_OVER)
        try:
            for line in range(start, end):
                percentage = self.line_percentage[line]
                if self.active_lyric_surfaces[line] is not None and self.inactive_lyric_surfaces[line] is not None:
                    width = self.active_lyric_surfaces[line].get_width()
                    height = self.active_lyric_surfaces[line].get_height()
                    xpos = self.adjust_lyric_xpos(line, percentage)
                    xpos += self.padding_x
                    text_mask = self.create_text_mask(line, xpos, alpha)
                    cr.save()
                    cr.rectangle(xpos, ypos, width * percentage, height)
                    cr.clip()
                    cr.set_source_surface(self.active_lyric_surfaces[line], xpos, ypos)
                    if text_mask:
                        cr.mask(text_mask)
                    else:    
                        cr.paint_with_alpha(alpha)
                    cr.restore()    
                    
                    cr.save()
                    cr.rectangle(xpos + width * percentage, ypos, width * (1.0 - percentage), height)
                    cr.clip()
                    cr.set_source_surface(self.inactive_lyric_surfaces[line], xpos, ypos)
                    
                    if text_mask:
                        cr.mask(text_mask)
                    else:    
                        cr.paint_with_alpha(alpha)
                    cr.restore()    
                    
                ypos += font_height * ( 1 + self.line_padding)    
            cr.restore()    
        except:    
            pass
        
    def set_input_shape_mask(self, disable_input):    
        if self.get_property("visible"):
            if disable_input:
                region = gtk.gdk.Region()
                self.window.input_shape_combine_region(region, 0, 0)
            else:    
                self.window.input_shape_combine_region(self.window.get_visible_region(), 0, 0)
            
    def point_in_rect(self, x, y, rect):        
        return rect.x <= x < rect.x + rect.width and rect.y <= y < rect.y + rect.height
            
    def check_mouse_leave(self):        
        root_window = gtk.gdk.get_default_root_window()
        screen_h , _ = root_window.get_size()
        rel_x, rel_y = root_window.get_pointer()[:2]
        x, y = self.get_position()
        mouse_over_lyrics = False
        width, height = self.get_size()        
        if y < 40:
            height += 40
        else:
            y -= 40
            height += 40

        rect = gtk.gdk.Rectangle(int(x), int(y), int(width), int(height))
        
        if self.dock_drag_state == DRAG_NONE and not self.point_in_rect(rel_x, rel_y, rect):
            self.mouse_over = False
            self.emit("hide-bg")
            self.queue_draw()
            
        for lyric_rect in self.lyric_rects:    
            if self.point_in_rect(rel_x, rel_y, lyric_rect):
                mouse_over_lyrics = True
                break
            
        if self.mouse_over_lyrics != mouse_over_lyrics:    
            self.mouse_over_lyrics = mouse_over_lyrics
            self.queue_draw()
        return True      
            
    def set_line_percentage(self, line, percentage):        
        if not self.get_karaoke_mode():
            return
        if line < 0 or line >= self.max_line_count:
            return
        if percentage == self.line_percentage[line]:
            return        
        old_percentage = self.line_percentage[line]
        self.line_percentage[line] = percentage

        if self.is_composited() and percentage != old_percentage:
            old_x = self.adjust_lyric_xpos(line, old_percentage)
            new_x = self.adjust_lyric_xpos(line, percentage)
            if old_x != new_x:
                pass
        self.queue_draw()    
        
    def set_current_line(self, line):    
        if 0 <= line <= self.get_line_count():
            self.current_line = line
        self.queue_draw()    
            
    def set_current_percentage(self, percentage):        
        self.set_line_percentage(self.current_line, percentage)
            
    def set_line_alignment(self, line, alignment):        
        if line < 0 or line > self.get_line_count():
            return
        if alignment < 0.0:
            alignment = 0.0
        elif alignment > 1.0:
            alignment = 1.0
        self.line_alignment[line] = alignment    
        self.update_lyric_rect(line)
        self.queue_draw()
        
    def set_lyric(self, line, lyric):    
        self.percentage = 0.0
        self.lyrics_text[line] = lyric
        self.update_lyric_surface(line)
        self.queue_draw()
    
    def set_font_size(self, value):    
        self.render_lyrics.set_font_size(value)
        self.update_font()
        
    def get_font_size(self):    
        return self.render_lyrics.get_font_size()
    
    def set_font_name(self, font_name):
        self.render_lyrics.set_font_name(font_name)
        self.update_font()
        
    def set_font_type(self, font_type):    
        self.render_lyrics.set_font_type(font_type)
        self.update_font()

        
SCROLL_ALWAYS, SCROLL_BY_LINES = 0, 1
LINE_ALIGN_LEFT, LINE_ALIGN_MIDDLE, LINE_ALIGN_RIGHT = range(3)
import pango
import pangocairo

class ScrollLyrics(DialogBox):
    __gsignals__ = {
        "seek" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_INT, gobject.TYPE_FLOAT)),
        "right-press" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))
        }
    
    def __init__(self):
        super(ScrollLyrics, self).__init__(_("Lyrics window"), 300, 200, DIALOG_MASK_GLASS_PAGE,
                                           self.hide_all, False,  gtk.gdk.WINDOW_TYPE_HINT_NORMAL,
                                           gtk.WIN_POS_CENTER, False, True
                                           )
        
        self.revert_button = ImageButton(
            app_theme.get_pixbuf("scroll_lyrics/revert_normal.png"),
            app_theme.get_pixbuf("scroll_lyrics/revert_hover.png"),
            app_theme.get_pixbuf("scroll_lyrics/revert_press.png"),
            )
        self.titlebar.button_box.pack_start(self.revert_button, False, False)
        self.titlebar.button_box.reorder_child(self.revert_button, 0)
        
        self.percentage = 0.0
        self.whole_lyrics = []
        self.message_text = "%s for %s" % (PROGRAM_NAME_LONG, "Deepin")
        self.current_lyric_id = -1
        self.line_count = 20
        self.active_color = self.get_render_color(True)
        self.inactive_color = self.get_render_color()
        self.bg_color = (1, 1, 1)
        self.font_name = "文泉驿微米黑 10"
        self.line_margin = 1
        self.padding_x = 20
        self.padding_y = 10
        self.bg_opacity= 0.9
        self.frame_width = 7
        self.text = ""
        self.can_seek = True
        self.seeking = False
        self.current_pointer_y = 0
        self.saved_lyric_id = -1
        self.saved_seek_offset = 0
        self.saved_pointer_y = 0 
        
        self.drawing = gtk.EventBox()
        self.drawing.set_visible_window(False)
        self.drawing.set_app_paintable(True)
        self.drawing.connect_after("expose-event", self.expose_cb)
        self.drawing.connect("button-press-event", self.button_press_cb)
        self.drawing.connect("button-release-event", self.button_release_cb)
        self.drawing.connect("motion-notify-event", self.motion_notify_cb)
        self.titlebar.close_button.connect("clicked", lambda w: self.hide_and_emit())
        self.body_box.add(self.drawing)
        self.update_line_height()
        config.connect("config-changed", self.changed_scroll_status)
        
    def changed_scroll_status(self, obj, selection, option, value):
        if selection == "scroll_lyrics" and option in ["font_name", "font_size", "font_type", "scroll_mode", "line_align"]:
            self.update_line_height()
            self.drawing.queue_draw()
            
        if selection == "scroll_lyrics" and option in ["inactive_color", "active_color"]:    
            self.active_color = self.get_render_color(True)
            self.inactive_color = self.get_render_color()
            self.drawing.queue_draw()
        
    def get_render_color(self, active=False):        
        if active:
            return color_hex_to_cairo(config.get("scroll_lyrics", "active_color"))
        return color_hex_to_cairo(config.get("scroll_lyrics", "inactive_color"))
            
    def get_font(self):    
        font_name = config.get("scroll_lyrics", "font_name")
        font_type = config.get("scroll_lyrics", "font_type", "Regular")
        font_size = config.get("scroll_lyrics", "font_size", "10")
        return "%s %s %s" % (font_name, font_type, font_size)
    
    def get_scroll_mode(self):
        return int(config.get("scroll_lyrics", "scroll_mode", SCROLL_ALWAYS))
    
    def get_alignment(self):
        return int(config.get("scroll_lyrics", "line_align", LINE_ALIGN_MIDDLE))
            
    def update_line_height(self):
        pango_context = gtk.gdk.pango_context_get()
        pango_layout = pango.Layout(pango_context)
        font_desc = pango.FontDescription(self.get_font())
        pango_layout.set_font_description(font_desc)
        
        metrics = pango_context.get_metrics(pango_layout.get_font_description())
        ascent = metrics.get_ascent()
        descent = metrics.get_descent()
        line_height = (ascent + descent) / pango.SCALE + self.line_margin
        self.__line_height = line_height
        
    def record_seek(self, event):    
        self.current_pointer_y = event.y_root
        self.drawing.queue_draw()
        
    def begin_seek(self, event):    
        self.seeking = True
        self.saved_seek_offset = self.saved_lrc_y
        self.saved_pointer_y = event.y_root
        self.saved_lyric_id = self.current_lyric_id
        
    def end_seek(self, event):    
        self.seeking = False
        
    def button_press_cb(self, widget, event):    
        if event.button == 1:
            if self.whole_lyrics and self.can_seek:
                self.begin_seek(event)
        elif event.button == 3:        
            self.emit("right-press", event)
                
    def button_release_cb(self, widget, event):            
        if self.seeking:
            current_lyric_id, lrc_y = self.calc_paint_pos()
            percentage = float(lrc_y) / float(self.__line_height)
            self.end_seek(event)
            self.emit("seek", int(current_lyric_id), percentage)
    
    def motion_notify_cb(self, widget, event):
        if self.seeking:
            self.record_seek(event)
        
    def expose_cb(self, widget, event):    
        cr = widget.window.cairo_create()
        # rect = widget.allocation
        # draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info(), 4)
        
        if self.whole_lyrics:
            self.paint_lyrics(cr)
        elif self.message_text:    
            self.paint_text(cr)
        return False    
    
    def get_pango(self, cr): 
        layout = pangocairo.CairoContext(cr).create_layout()
        font_desc = pango.FontDescription(self.get_font())
        layout.set_font_description(font_desc)
        return layout
    
    def calc_lrc_ypos(self, percentage):
        if self.get_scroll_mode() == SCROLL_BY_LINES:
            if percentage < 0.15:
                percentage = percentage / 0.15
            else:    
                percentage = 1.0
        return self.__line_height * percentage        
    
    def calc_paint_pos(self):
        if self.seeking:
            lyric_id = self.saved_lyric_id
            y = self.saved_seek_offset - self.current_pointer_y + self.saved_pointer_y
            lyric_id += y / self.__line_height
            y %= self.__line_height
            
            if y < 0:
                y += self.__line_height
                lyric_id -= 1
            if lyric_id < 0:    
                lyric_id = 0
                y = 0
            elif lyric_id >= len(self.whole_lyrics):
                lyric_id = len(self.whole_lyrics) - 1
                y = self.__line_height
                
            return int(lyric_id), int(y) 
        
        else:
            lyric_id = self.current_lyric_id
            self.saved_lrc_y = self.calc_lrc_ypos(self.percentage)
            return (lyric_id, self.saved_lrc_y)
        
    def get_drawing_size(self):    
        width, height = self.get_size()
        return width ,height
        
    def adjust_line_count(self):    
        width, height = self.get_drawing_size()
        line_count = height - self.padding_y * 2 / self.__line_height
        return line_count
    
    def get_active_color_ratio(self, line):
        current_lyric_id, lrc_y = self.calc_paint_pos()
        ratio = 0.0
        percentage = lrc_y / self.__line_height
        if line == current_lyric_id:
            ratio = (1.0 - percentage) / 0.1
            if ratio > 1.0: ratio = 1.0
            if ratio < 0.0: ratio = 0.0

        elif line == current_lyric_id + 1:
            ratio = (percentage - 0.9) / 0.1
            if ratio > 1.0: ratio = 1.0
            if ratio < 0.0: ratio = 0.0
        return ratio    
        
    def paint_lyrics(self, cr):    
        count = self.adjust_line_count()
        width , height = self.get_drawing_size()
        layout = self.get_pango(cr)
        
        cr.save()
        cr.new_path()
        cr.rectangle(self.padding_x, 0, width - self.padding_x * 2, height - self.padding_y * 2)
        cr.clip()
        current_lyric_id, lrc_y = self.calc_paint_pos()
        begin = current_lyric_id - count / 2
        end = current_lyric_id + count / 2 + 1
        ypos = height / 2 - lrc_y - (count / 2 + 1) * self.__line_height
        cr.set_source_rgb(*self.inactive_color)
        
        if self.whole_lyrics:
            for i in range(begin, end):
                ypos += self.__line_height
                if i < 0: continue
                if i > len(self.whole_lyrics) - 1:
                    break                
                layout.set_text(self.whole_lyrics[i])
                layout.set_alignment(pango.ALIGN_CENTER)        
                extent = layout.get_pixel_extents()[0]
                if self.get_alignment() == LINE_ALIGN_LEFT:
                    x = self.padding_x
                elif self.get_alignment() == LINE_ALIGN_MIDDLE:    
                    x = (width - extent[2]) / 2
                elif self.get_alignment() == LINE_ALIGN_RIGHT:    
                    x = width - extent[2] - self.padding_x
                    
                cr.save()
                ratio = self.get_active_color_ratio(i)
                alpha = 1.0
                
                if ypos < self.__line_height / 2.0 + 30:
                    alpha = 1.0 - (self.__line_height / 2.0 + 30 - ypos) * 1.0 / self.__line_height * 2
                elif ypos > height - self.__line_height * 1.5 - self.padding_y:    
                    alpha = (height - self.__line_height - self.padding_y - ypos) * 1.0 / self.__line_height * 2
                if alpha < 0.0: alpha = 0.0    
                cr.set_source_rgba(
                    self.active_color[0] * ratio + self.inactive_color[0] * (1 - ratio),
                    self.active_color[1] * ratio + self.inactive_color[1] * (1 - ratio),
                    self.active_color[2] * ratio + self.inactive_color[2] * (1 - ratio),
                    alpha
                    )
                cr.move_to(x, ypos)
                cr.update_layout(layout)
                cr.show_layout(layout)
                cr.restore()
        cr.restore()        
        
    def paint_text(self, cr):    
        width, height = self.get_drawing_size()
        cr.set_source_rgb(*self.active_color)
        draw_text(cr, self.message_text, 10, 5, width - 20, height - 10, 
                  text_color=cairo_color_to_hex(self.active_color),
                  wrap_width=width-40)
        
    def set_progress(self, lyric_id, percentage):    
        saved_lyric_id = self.current_lyric_id
        self.current_lyric_id = lyric_id
        self.percentage = percentage
        if saved_lyric_id != lyric_id or self.saved_lrc_y != self.calc_lrc_ypos(percentage):
            self.drawing.queue_draw()
            
    def set_whole_lyrics(self, whole_lyrics):        
        self.message_text = ""
        self.whole_lyrics = whole_lyrics
        self.saved_lrc_y = -1
        self.can_seek = True
        self.drawing.queue_draw()
        if self.seeking:    
            self.seeking = False
            
    def set_message(self, message):        
        del self.whole_lyrics[:]
        self.message_text = message
        self.can_seek = False
        self.drawing.queue_draw()
            
    def get_current_lyric_id(self):        
        return self.current_lyric_id
    
    def hide_and_emit(self):    
        self.hide_all()
        config.set("lyrics", "status", "false")        
        Dispatcher.close_lyrics()

