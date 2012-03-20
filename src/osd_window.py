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

import gtk
import cairo
import pango
import pangocairo
import math
from ui_toolkit import app_theme
import copy
import gobject
from collections import namedtuple
from osd_render import RenderContextNew


DEFAULT_COLORS = {
    "inactive": [(0.6, 1.0, 1.0),
                 (0.0, 0.0, 1.0),
                 (0.6, 1.0, 1.0)],
    "active"  : [(0.4, 0.15, 0.0),
                 (1.0, 1.0, 0.0),
                 (1.0, 0.5, 0.0)]
    }


MOUSE_TIMTER_INTERVAL = 100
DEFAULT_LINE_HEIGHT = 45
LINE_PADDING = 0.0
BORDER_WIDTH = 10
DEFAULT_WIDTH = 600
MAX_LYRIC_LEN = 256
FADE_IN_SIZE = 20.0
MAX_LINE_COUNT = 2


# drag state.
DRAG_NONE = 1
DRAG_MOVE = 2
DRAG_EAST = 3
DRAG_WEST = 4

# window mode
OSD_WINDOW_NORMAL = 1
OSD_WINDOW_DOCK   = 2
OSD_WINDOW_NONE   = 3

# linear 
LINEAR_COLOR_COUNT = 3

# rects
rects = namedtuple("rect", "x y width, height")

class OsdWindow(gobject.GObject):
    __gsignals__ = {
        "osd-moved" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
        "osd-resized" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))}
    	
    def __init__(self):
        ''' Init. '''
        gobject.GObject.__init__(self)
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)

        # self.window = gtk.Window(gtk.WINDOW_POPUP)
        self.window.set_property("allow-shrink", True)
        self.window.set_skip_taskbar_hint(True)
        self.window.set_type_hint(gtk.gdk.WINDOW_TYPE_HINT_DOCK)
        
        self.window.set_position(gtk.WIN_POS_CENTER)
        self.window.set_decorated(False)
        self.window.set_app_paintable(True)
        self.window.stick()
        self.window.set_keep_above(True)
        self.window.set_default_size(600, 120)
        
        self.empty_mask = True
        self.time_out = 0.1
        
        # Init the osd.
        self.osd_current_line = 0
        self.osd_bg_pixbuf = app_theme.get_pixbuf("skin/desktop_lrc.png").get_pixbuf()
        self.osd_line_count = 2
        self.osd_lyrics = ["", ""]
        self.osd_line_alignment = [0.0, 1.0]
        self.osd_lyric_rects = []
        self.osd_render_context = RenderContextNew()
        self.osd_translucent_on_mouse_over = True
        self.osd_percentage = [0.0, 0.0]
        
        # Initilaize private data
        self.priv_active_lyric_surfaces = []
        self.priv_inactive_lyric_surfaces = []
        self.priv_lyric_xpos = []
        self.priv_shape_pixmap = None
        self.priv_width = DEFAULT_WIDTH
        self.priv_locked = False
        self.priv_composited = False
        self.priv_visible = False
        self.priv_mouse_timer_id = 0
        self.priv_mouse_over = False
        self.priv_configure_event = 0
        self.priv_child_requisition = [0, 0] # width, height
        self.priv_mouse_over_lyrics = False
        self.priv_composited_signal = 0
        self.priv_raw_x = 0
        self.priv_raw_y = 0
        self.priv_old_x = 0
        self.priv_old_y = 0
        self.priv_mouse_x = 0
        self.priv_mouse_y = 0
        self.priv_drag_state = DRAG_NONE 
        self.priv_update_shape = False
        self.priv_blur_radius = 3
        self.priv_mode = OSD_WINDOW_NONE
        self.priv_osd_height = 0
        
        # Give the init value.
        for i in range(MAX_LINE_COUNT):
            self.osd_lyrics.append(None)
            self.osd_line_alignment.append(0.5)
            # self.osd_percentage.append(0.0)
            self.priv_active_lyric_surfaces.append(None)
            self.priv_inactive_lyric_surfaces.append(None)
            self.osd_lyric_rects.append(rects(0, 0, 0, 0))
            self.priv_lyric_xpos.append(0)
            
        self.osd_active_colors = DEFAULT_COLORS["active"]
        self.osd_inactive_colors = DEFAULT_COLORS["inactive"]
            
        # # The setting.
        self.set_window_mode(OSD_WINDOW_NORMAL)
        # self.set_window_mode(OSD_WINDOW_DOCK)
        self.update_window_colormap()
        self.screen_composited_changed()
                        
        # Add events.
        self.window.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                               gtk.gdk.BUTTON_RELEASE_MASK |
                               gtk.gdk.POINTER_MOTION_MASK |
                               gtk.gdk.ENTER_NOTIFY_MASK |
                               gtk.gdk.LEAVE_NOTIFY_MASK)

        self.window.connect("button-press-event", self.button_press) # TRY
        self.window.connect("button_release_event", self.button_release) 
        self.window.connect("motion-notify-event", self.motion_notify)   
        self.window.connect("enter-notify-event", self.enter_notify)
        self.window.connect("leave-notify-event", self.leave_notify)
        self.window.connect("expose-event", self.expose_before)          
        self.window.connect_after("expose-event", self.expose_after)     
        self.window.connect("map-event", self.window_map_cb)             
        self.window.connect("unmap-event", self.window_unmap_cb)         
        self.window.connect("realize", self.window_realize_cb)           
        self.window.connect("unrealize", self.window_unrealize_cb)       
                
        
    def set_window_mode(self, mode):
        ''' Set window mode. '''
        mapped = self.window.get_mapped()
        realized = self.window.get_realized()
        
        if self.priv_mode == mode:
            return True
        if mapped:
            self.window.unmap()
        if realized:    
            self.window.unrealize()
        if mode == OSD_WINDOW_NORMAL:    
            self.window.set_type_hint(gtk.gdk.WINDOW_TYPE_HINT_NORMAL)
        elif mode == OSD_WINDOW_DOCK:    
            self.window.set_type_hint(gtk.gdk.WINDOW_TYPE_HINT_DOCK)
        else:    
            print "Invalid OSD Window type %d \n" % mode
            mode = self.priv_mode
        self.priv_mode = mode    
        
        self.update_render_blur_radius()
        if realized:
            self.window.realize()
        if mapped:    
            self.window.map()
            # We need to specify the position so that OSD window keeps its place. 
            self.queue_resize()
        self.queue_reshape()    
        
    def update_window_colormap(self):    
        ''' Update the window colormap. '''
        mapped = self.window.get_mapped()
        realized = self.window.get_realized()
        screen = self.window.get_screen()
        colormap = screen.get_rgba_colormap()
        
        if mapped: self.window.unmap()
        if realized: self.window.unrealize()
        
        self.window.set_colormap(colormap)
        
        if realized: self.window.realize()
        if mapped: self.window.map()
        
    def screen_composited_changed(self):        
        ''' When screen composited changed. '''
        self.priv_composited = self.window.is_composited()
        if self.priv_blur_radius > 0:
            self.update_render_blur_radius() 
            for i in range(self.osd_line_count):
                self.update_lyric_surface(i)
        self.queue_reshape()    
        self.window.queue_draw()
        
    def queue_resize(self):
        ''' Queue window resize. '''
        if self.window.get_realized(): return True
        self.update_layout()
        width = self.compute_window_width() 
        height = self.compute_window_height()
        self.move_resize(self.priv_raw_x, self.priv_raw_y, width, height, DRAG_NONE)
        
    def queue_reshape(self):    
        ''' Queue window reshape. '''
        self.priv_update_shape = True
        
    def reset_shape_pixmap(self):
        if not self.window.get_realized():
            return 
        width = self.priv_width
        height = self.compute_window_height()
        if self.priv_shape_pixmap !=None:
            w, h = self.priv_shape_pixmap.get_size()
            if w == width and h == height:
                return 
            else:
                self.priv_shape_pixmap = None
                
        self.priv_shape_pixmap = gtk.gdk.Pixmap(self.window.get_window(), width, height, 1)        
        cr = self.priv_shape_pixmap.cairo_create()
        self.clear_cairo(cr)    
        self.queue_reshape()
        
    def update_window_shape(self): # TRY
        ''' update window shape. '''
        if not self.window.get_realized():
            return 
        if self.priv_composited or self.panel_visible():
            if not self.empty_mask:
                self.empty_mask = True
                self.window.window.shape_combine_mask(None, 0.0, 0.0)
            return
        
        self.empty_mask = False
        shape_mask = self.priv_shape_pixmap
        color = gtk.gdk.Color()
        color.pixel = 0.0
        fg_gc = shape_mask.new_gc()
        fg_gc.set_foreground(color)
        fg_gc.set_foreground(color)
        cr = shape_mask.cairo_create()
        self.clear_cairo(cr)
        self.paint_window_lyrics(cr)
        self.window.window.shape_combine_mask(shape_mask, 0.0, 0.0)
        self.window.queue_draw()
        
        # if self.priv_mouse_over and not self.priv_locked:
        #     self.window.window.shape_combine_mask(None, 0, 0)
        # else:
        #     cr = self.window.window.cairo_create()
        #     self.clear_cairo(cr)
        #     self.paint_window_lyrics(cr)
        # self.priv_update_shape = False
        
        
    def update_render_blur_radius(self):    
        ''' Update render blur radius. '''
        if self.window_should_blur():
            self.osd_render_context.set_blur_radius(self.priv_blur_radius)
        else:    
            self.osd_render_context.set_blur_radius(0)
            
    def update_lyric_surface(self, line):        
        ''' Update lyrics surface. '''
        # Draws the inactive surfaces.
        self.osd_render_context.set_linear_color(self.osd_inactive_colors)
        self.priv_inactive_lyric_surfaces[line] = self.draw_lyric_surface(self.osd_lyrics[line]) # TRY
        # self.priv_inactive_lyric_surfaces[line].write_to_png("inacitve%d.png" % line)
        
        # Draws the active surfaces.
        self.osd_render_context.set_linear_color(self.osd_active_colors)
        self.priv_active_lyric_surfaces[line] = self.draw_lyric_surface(self.osd_lyrics[line])
        # self.priv_active_lyric_surfaces[line].write_to_png("acitve%d.png" % line)
        
        self.update_lyric_rect(line)                                                          
        
    def update_layout(self):    
        ''' update the layout.'''
        old_height = self.priv_osd_height                   
        self.priv_osd_height = self.compute_osd_height()
        if self.priv_osd_height != old_height:
            self.reset_shape_pixmap() 
        self.update_child_size_requisition() 
            
    def compute_window_width(self):    
        ''' compute window width. '''
        width = self.priv_width
        if width < self.priv_child_requisition[0]:
            width = self.priv_child_requisition[0]
        return width    
    
    def compute_window_height(self):
        ''' compute window height. '''
        height = BORDER_WIDTH * 2
        height += self.priv_osd_height + self.priv_child_requisition[1]
        return height
    
    def move_resize(self, x, y, width, height, drag_state):
        ''' The window move resize. '''
        size_changed = False
        pos_changed = False
        old_x, old_y = self.window.get_position()
        old_w, old_h = self.window.get_size()
        new_x, new_y, new_width, new_height = self.compute_constrain(x, y, width, height, drag_state)
        if new_height != old_h and new_width != old_w:
            size_changed = True
        if self.priv_raw_x != new_x and self.priv_raw_y != new_y and old_x != new_x and old_y != new_y:    
            pos_changed = True
        self.priv_raw_x = new_x    
        self.priv_raw_y = new_y

        if pos_changed:
            self.window.move(new_x, new_y)
        if size_changed and pos_changed:    
            self.window.resize(width, height)
            self.queue_reshape()
            self.window.queue_draw()
            
    def compute_constrain(self, x, y, width, height, drag_state):    
        ''' Constrain compute the window size. '''
        screen = self.window.get_screen()
        if self.get_mode() == OSD_WINDOW_DOCK and screen: 
            sw = screen.get_width()
            sh = screen.get_height()
        if drag_state == DRAG_EAST:
            new_x = max(0, x)
            new_width = min(width, sw - new_x)
        elif drag_state == DRAG_WEST:    
            new_width = min(width, x + width)
            new_x = max(0, min(x, self.priv_old_x + self.priv_old_width - new_width))
        else:    
            new_x = max(0, min(x, sw-width))
            
        if y + height >  sh:    
            min_h = self.compute_window_height() + BORDER_WIDTH * 2
            new_height = max(min_h, sh - y)
            new_y = max(0, sh - new_height)
        else:    
            new_y = max(0, y)
            new_height = height
            
        return (new_x, new_y, new_width, new_height)    

    def window_should_blur(self):
        ''' Whether window should blur. '''
        return self.priv_composited and self.priv_mode == OSD_WINDOW_NORMAL
    
    def draw_lyric_surface(self, lyrics):
        ''' Draw lyric surface. '''
        if not self.window.get_realized():
            self.window.realize()
        width, height = self.osd_render_context.get_pixel_size(lyrics)
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, int(width), int(height))
        cr = cairo.Context(surface)
        cr.set_source_rgba(1.0, 1.0, 1.0, 0.0)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()
        self.osd_render_context.paint_text(cr, lyrics, 0, 0)
        return surface
        
    def update_lyric_rect(self, line):    
        ''' Update lyric rect. '''
        w = h = 0
        if self.priv_active_lyric_surfaces[line] != None:
            w = self.priv_active_lyric_surfaces[line].get_width()
            h = self.priv_active_lyric_surfaces[line].get_height()
        font_height = self.osd_render_context.get_font_height()    
        self.osd_lyric_rects[line] = rects(self.compute_lyric_xpos(line, self.osd_percentage[line]), font_height * line * (1 + LINE_PADDING), w, h)
        
    def compute_osd_height(self):    
        ''' compute the osd height. '''
        font_height = self.osd_render_context.get_font_height()
        height = font_height * (self.osd_line_count + (self.osd_line_count - 1) * LINE_PADDING ) + self.osd_render_context.get_outline_width()
        if self.window_should_blur():
            height += self.priv_blur_radius * 2.0
        return height    
    
    def update_child_size_requisition(self):    
        ''' Update child size requisition. '''
        child = self.window.child
        if child and child.get_visible():
            child.set_size_request(*self.priv_child_requisition)
        else:    
            self.priv_child_requisition = [0, 0]
        
    def compute_lyric_xpos(self, line, percentage):        
        ''' Compute lyrics xpos. '''
        smooth = True
        w, h = self.get_osd_size() 
        if self.priv_active_lyric_surfaces[line] != None:
            width = self.priv_active_lyric_surfaces[line].get_width()
        else:    
            width = 0
        if w >= width:    
            xpos = (w -width) * self.osd_line_alignment[line]
        else:    
            if self.priv_composited and self.get_mode() == OSD_WINDOW_DOCK:
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
                if xpos != self.priv_lyric_xpos[line]:                
                    self.queue_reshape()
                    self.priv_lyric_xpos[line] = xpos
        return xpos            
    
    def compute_lyric_ypos(self):
        '''compute lyric_ypos '''
        return BORDER_WIDTH
    
    def get_mode(self):
        ''' Get the window mode. '''
        return self.priv_mode
    
    def clear_cairo(self, cr):
        ''' Clear the specify cairo. '''
        cr.save()
        cr.set_source_rgba(1.0, 1.0, 1.0, 0.0)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()
        cr.restore()
        
    def get_osd_size(self):
        ''' Get osd size. '''
        width = self.priv_width - BORDER_WIDTH * 2
        height = self.compute_osd_height()
        return (width, height)
    
    def button_press(self, widget, event):
        ''' button press callback.'''
        if event.button == 1 and not self.priv_locked:
            edge = self.get_edge_on_point(event.x, event.y) 
            if self.use_system_drag():                      
                if edge == gtk.gdk.WINDOW_EDGE_EAST or edge == gtk.gdk.WINDOW_EDGE_WEST:
                    self.window.begin_resize_drag(edge,
                                                  event.button,
                                                  int(event.x_root),
                                                  int(event.y_root),
                                                  event.time)
                else:    
                    self.window.begin_move_drag(event.button,
                                                int(event.x_root),
                                                int(event.y_root),
                                                event.time)
            else:        
                self.priv_old_x = widget.allocation.x
                self.priv_old_y = widget.allocation.y
                self.priv_old_width = widget.allocation.width
                self.priv_mouse_x = event.x_root
                self.priv_mouse_y = event.y_root
                if edge == gtk.gdk.WINDOW_EDGE_EAST:
                    self.priv_drag_state = DRAG_EAST
                elif edge == gtk.gdk.WINDOW_EDGE_WEST:    
                    self.priv_drag_state = DRAG_WEST
                else:    
                    self.priv_drag_state = DRAG_MOVE
        return False            
    
    def get_edge_on_point(self, x, y):                
        ''' Get edge on point with x, y.'''
        width = self.window.allocation.width
        height = self.window.allocation.height
        if y >= 0 and y <= height:
            if x >= 0 and x < BORDER_WIDTH:                    
                return gtk.gdk.WINDOW_EDGE_WEST
            if x >= width - BORDER_WIDTH and x < width:
                return gtk.gdk.WINDOW_EDGE_EAST
        return -1    
    
    def use_system_drag(self):
        ''' Whether use system drag. '''
        return self.priv_mode == OSD_WINDOW_NORMAL
                
    def button_release(self, widget, event):
        ''' button release callback. '''
        if self.priv_drag_state != DRAG_NONE:
            self.emit("osd-moved", widget)
            self.emit("osd-resized", widget)
        self.priv_drag_state = DRAG_NONE    
        return False
    
    def motion_notify(self, widget, event):    
        ''' motion notify callback. '''
        x = self.priv_old_x + (event.x_root - self.priv_mouse_x)
        y = self.priv_old_y + (event.y_root - self.priv_mouse_y)
        width, height = widget.get_size()
        if self.priv_drag_state == DRAG_MOVE:
            widget.move(int(x), int(y))
        elif self.priv_drag_state == DRAG_EAST:    
            self.move_resize(self.priv_old_x, self.priv_old_y,
                             self.priv_old_width + (event.x_root - self.priv_mouse_x),
                             height,
                             DRAG_EAST)

        elif self.priv_drag_state == DRAG_WEST:    
            self.move_resize(x, self.priv_old_y,
                             self.priv_old_width + self.priv_old_x - x,
                             height,
                             DRAG_WEST)

        elif self.priv_drag_state == DRAG_NONE:
            edge = self.get_edge_on_point(event.x, event.y)
            if edge == gtk.gdk.WINDOW_EDGE_EAST:
                cursor = gtk.gdk.RIGHT_SIDE
                widget.window.set_cursor(gtk.gdk.Cursor(cursor)) 
            elif edge == gtk.gdk.WINDOW_EDGE_WEST:    
                cursor = gtk.gdk.LEFT_SIDE
                widget.window.set_cursor(gtk.gdk.Cursor(cursor))
            else:    
                widget.window.set_cursor(None)
                 

        return False        
    
    def enter_notify(self, widget, event):
        ''' enter notify. '''
        self.priv_mouse_over = True
        self.queue_reshape()
        self.window.queue_draw()
        return False
    
    def leave_notify(self, widget, event):
        ''' leave notify.'''
        self.check_mouse_leave()
        self.priv_mouse_over = False
        self.queue_reshape()
        self.window.queue_draw()
        
    def get_min_width(self):    
        tmp_list = []
        for i in self.priv_inactive_lyric_surfaces:
            if i:
                tmp_list.append(i.get_width())
        return min(tmp_list)        
    
    def expose_before(self, widget, event):
        ''' expose before callback. '''
        if self.panel_visible(): 
            self.paint_window()  
        return False    
    
    def expose_after(self, widget, event):
        ''' expose after callback. '''
        if not self.panel_visible():
            self.paint_window()
        return False
    
    def panel_visible(self):    
        ''' Whether panel is visible. '''
        return self.priv_mode == OSD_WINDOW_NORMAL or (not self.priv_locked and self.priv_mouse_over) #TRY
    
    def paint_window(self):
        ''' The window paint. '''
        cr = self.window.window.cairo_create()
        self.paint_window_bg(cr) 
        self.paint_window_lyrics(cr) 
        if self.priv_update_shape:
            self.update_window_shape() 
            
            
    def paint_window_bg(self, cr):
        ''' Paint window bg. '''
        w, h = self.window.window.get_size()
        self.clear_cairo(cr)
        if self.panel_visible():
            if self.osd_bg_pixbuf != None:
                if  self.priv_composited:
                    self.clear_cairo(cr)
                else:    
                    cr.set_operator(cairo.OPERATOR_SOURCE)
                    cr.set_source_rgb(0.9, 0.9, 0.9)
                    cr.rectangle(0, 0, w, h)
                    cr.fill()
            cr.set_operator(cairo.OPERATOR_OVER)        
            sw = self.osd_bg_pixbuf.get_width()
            sh = self.osd_bg_pixbuf.get_height()
            
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              0, 0, BORDER_WIDTH, BORDER_WIDTH,
                              0, 0, BORDER_WIDTH, BORDER_WIDTH) 
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              0, sh - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH,
                              0, h - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH)
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              sw - BORDER_WIDTH, 0, BORDER_WIDTH, BORDER_WIDTH,
                              w - BORDER_WIDTH, 0, BORDER_WIDTH, BORDER_WIDTH)
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              sw - BORDER_WIDTH, sh - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH,
                              w - BORDER_WIDTH, h - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH)
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              0, BORDER_WIDTH, BORDER_WIDTH, sh - BORDER_WIDTH * 2,
                              0, BORDER_WIDTH, BORDER_WIDTH, h - BORDER_WIDTH * 2)
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              sw - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH, sh - BORDER_WIDTH * 2,
                              w - BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH, h - BORDER_WIDTH * 2)
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              BORDER_WIDTH, 0, sw - BORDER_WIDTH * 2, BORDER_WIDTH,
                              BORDER_WIDTH, 0, w - BORDER_WIDTH * 2, BORDER_WIDTH)
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              BORDER_WIDTH, sh - BORDER_WIDTH, sw - BORDER_WIDTH * 2, BORDER_WIDTH,
                              BORDER_WIDTH, h - BORDER_WIDTH, w - BORDER_WIDTH * 2, BORDER_WIDTH)
            self.__paint_rect(cr, self.osd_bg_pixbuf,
                              BORDER_WIDTH, BORDER_WIDTH, sw - BORDER_WIDTH * 2, sh - BORDER_WIDTH * 2,
                              BORDER_WIDTH, BORDER_WIDTH, w - BORDER_WIDTH * 2, h - BORDER_WIDTH * 2)
        else:    
            self.window.get_style().paint_box(self.window.window, gtk.STATE_NORMAL, 
                                              gtk.SHADOW_IN, None, self.window.get_default_widget(), 
                                              "buttondefalut", 0, 0, w, h)
        return True    

    
    def paint_window_lyrics(self, cr):
        ''' Paint window lyrics. '''
        # if not self.window.window.is_visible():
        #     return True
        alpha = 1.0
        font_height = self.osd_render_context.get_font_height()
        if self.priv_composited and self.priv_locked and self.priv_mouse_over_lyrics and self.osd_translucent_on_mouse_over:
            alpha = 0.3
        if not self.window.get_realized():    
            self.window.realize()
        w, h = self.get_osd_size()    
        ypos = self.compute_lyric_ypos()
        if self.osd_line_count == 1:
            start = self.osd_current_line
            end = start + 1
        else:    
            start = 0 
            end = MAX_LINE_COUNT
        cr.save()    
        cr.rectangle(BORDER_WIDTH, BORDER_WIDTH, w, h)
        cr.clip()
        cr.set_operator(cairo.OPERATOR_OVER)
        for line in range(start, end):
            percentage = self.osd_percentage[line]
            if self.priv_active_lyric_surfaces[line] != None and self.priv_inactive_lyric_surfaces[line] !=None:
                width = self.priv_active_lyric_surfaces[line].get_width()
                height = self.priv_active_lyric_surfaces[line].get_height()
                xpos = self.compute_lyric_xpos(line, self.osd_percentage[line])
                xpos += BORDER_WIDTH
                text_mask = self.create_text_mask(line, xpos, alpha) 
                cr.save()
                cr.rectangle(xpos, ypos, width * percentage, height)
                cr.clip()
                cr.set_source_surface(self.priv_active_lyric_surfaces[line], xpos, ypos)
                if text_mask:
                    cr.mask(text_mask)
                else:    
                    cr.paint_with_alpha(alpha)
                cr.restore()    
                cr.save()
                cr.rectangle(xpos + width * percentage, ypos, width * (1.0 - percentage), height)
                cr.clip()
                cr.set_source_surface(self.priv_inactive_lyric_surfaces[line], xpos, ypos)
                
                if text_mask:
                    cr.mask(text_mask)
                else:    
                    cr.paint_with_alpha(alpha)
                cr.restore()    
            ypos += font_height * ( 1 + LINE_PADDING)    

            
        
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
        
    def create_text_mask(self, line, text_xpos, alpha):                
        ''' Create text mask. '''
        width, _height = self.get_osd_size()
        left = BORDER_WIDTH
        text_width = self.priv_active_lyric_surfaces[line].get_width()
        fade_in_size = FADE_IN_SIZE
        if fade_in_size * 2 > width:
            fade_in_size = width / 2
        if self.priv_composited:    
            pattern = cairo.LinearGradient(left, 0.0, left+width, 0.0)
            # Set fade in on left edge
            if text_xpos < left:
                pattern.add_color_stop_rgba(0.0, 1.0, 1.0, 1.0, 0.0) # offset r, g, b, a
                loffset = 0.0
                if left - text_xpos < fade_in_size:    
                    loffset = float(left - text_xpos) / float(width)
                else:    
                    loffset = float(fade_in_size) / float(width)
                    
                pattern.add_color_stop_rgba(loffset, 1.0, 1.0, 1.0, alpha)                            
            else:    
                pattern.add_color_stop_rgba(0.0, 1.0, 1.0, 1.0, alpha)
                
            # Set fade out on the right edge    
            if text_xpos + text_width > left + width:    
                roffset = 0.0
                if text_xpos + text_width - (left + width) < fade_in_size:
                    roffset = 1.0 - (text_xpos + text_width - (left + width)) / width
                else:    
                    roffset = 1.0 - float(fade_in_size) / width
                pattern.add_color_stop_rgba(roffset, 0.0, 0.0, 0.0, alpha)    
                pattern.add_color_stop_rgba(1.0, 0.0, 0.0, 0.0, 0.0)
            else:    
                pattern.add_color_stop_rgba(1.0, 0.0, 0.0, 0.0, alpha)
            return pattern    
        
        return None
    
    def window_map_cb(self, widget, event):
        ''' Window map callback. '''
        if self.priv_mouse_timer_id == 0:
            self.priv_mouse_timer_id = gobject.timeout_add(MOUSE_TIMTER_INTERVAL, self.window_mouse_timer)
        self.queue_reshape()

        self.priv_configure_event = self.window.connect("configure-event", self.window_configure_event) 
        return False
    
    def window_unmap_cb(self, widget, event):        
        ''' window unmap callback. '''
        if self.priv_mouse_timer_id != 0:       
            gobject.source_remove(self.priv_mouse_timer_id)
            self.priv_mouse_timer_id = 0
        if self.priv_configure_event > 0:    
            self.window.disconnect(self.priv_configure_event)
            self.priv_configure_event = 0
        return False    
    
    def window_realize_cb(self, widget):
        '''window realized callback. '''
        if self.priv_composited_signal == 0:
            self.priv_composited_signal = self.window.connect("composited-changed", lambda w: self.screen_composited_changed())
        self.reset_shape_pixmap()    
        self.set_input_shape_mask(self.priv_locked) 

        
    def window_unrealize_cb(self, widget):    
        ''' window unrealize callback.'''
        if self.priv_composited_signal != 0:
            self.window.disconnect(self.priv_composited_signal)
            self.priv_composited_signal = 0
            
    def window_mouse_timer(self):    
        if self.window.get_realized():
            mouse_over = False
            rel_x, rel_y = self.window.window.get_pointer()[:2]
            
            for i in range(self.osd_line_count):
                if self.__point_in_rect(rel_x, rel_y, self.osd_lyric_rects[i]): 
                    mouse_over = True
                    break
            if self.priv_mouse_over_lyrics != mouse_over:    
                self.priv_mouse_over_lyrics = mouse_over
                self.window.queue_draw()
            if self.get_mode() == OSD_WINDOW_DOCK:    
                window_rect = rects(0, 0, self.window.allocation.width, self.window.allocation.height)
                mouse_over = self.__point_in_rect(rel_x, rel_y, window_rect)
                if mouse_over != self.priv_mouse_over:
                    self.priv_mouse_over = mouse_over
                    self.queue_reshape()
                    self.window.queue_draw()
            else:        
                self.check_mouse_leave() 
        return True        
    
    def window_configure_event(self, widget, event):
        ''' The window configure event. '''
        size_changed = False
        widget.allocation.x = event.x
        widget.allocation.y = event.y
        if self.priv_width != event.width:
            self.priv_width = event.width
            size_changed = True
            if self.priv_drag_state == DRAG_NONE:
                self.emit("osd-resized", widget)
          
        if size_changed:        
            self.update_child_allocation() 
            self.reset_shape_pixmap()
        if self.priv_raw_x != event.x or self.priv_raw_y != event.y:
            self.priv_raw_x = event.x
            self.priv_raw_y = event.y
            if self.priv_drag_state == DRAG_NONE:
                self.emit("osd-moved", widget)
        return False        
    
    def set_input_shape_mask(self, disable_input):
        ''' set input shape mask. '''
        if disable_input:
            region = gtk.gdk.Region()
            self.window.window.input_shape_combine_region(region, 0, 0)
        else:    
            self.window.window.input_shape_combine_region(self.window.window.get_visible_region(), 0, 0)
            
    def __point_in_rect(self, x, y, rect):        
        '''Whether the point in rect. '''
        return rect.x <= x < rect.x + rect.width and rect.y <= y < rect.y + rect.height
    
    def check_mouse_leave(self):
        '''check the mouse leave.'''
        if not self.window.window.is_visible():
            return True
        rel_x, rel_y = self.window.window.get_pointer()[:2]
        rect = rects(0, 0, self.window.allocation.width, self.window.allocation.height)
        if self.priv_mode != OSD_WINDOW_DOCK or self.priv_drag_state == DRAG_NONE and not self.__point_in_rect(rel_x, rel_y, rect):
            self.priv_mouse_over = False
            self.queue_reshape()
            self.window.queue_draw()
           
    def update_child_allocation(self):        
        ''' update child allocation. '''
        child = self.window.get_child()
        if child and child.get_visible():
            print "set child allocation."
            x = (self.window.allocation.width - self.priv_child_requisition[0]) / 2
            y = self.window.allocation.height - BORDER_WIDTH - self.priv_child_requisition[1]
            width = self.priv_child_requisition[0]
            height = self.priv_child_requisition[1]
            rect = gtk.gdk.Rectangle(x, y, width, height)
            child.set_allocation(rect)
            
    def set_window_width(self, width):        
        '''set window width'''
        self.priv_width = width
        self.queue_resize()
        self.queue_reshape()
        self.window.queue_draw()
        
    def get_window_width(self):    
        '''get window width'''
        return self.priv_width
    
    def set_locked(self, locked):
        '''whether window will locked the desktop'''
        if self.priv_locked == locked:
            return True
        self.priv_locked = locked
        if self.window.get_realized():
            self.set_input_shape_mask(locked)
        self.queue_reshape()    
        self.window.queue_draw()
        
    def set_percentage(self, line, percentage):
        ''' set line percentage'''
        if line < 0 or line >= MAX_LINE_COUNT:
            return
        if percentage == self.osd_percentage[line]:
            return
        old_percentage = self.osd_percentage[line]
        self.osd_percentage[line] = percentage
        # update shape if line is to long. 
        if self.priv_composited and percentage != old_percentage:
            old_x = self.compute_lyric_xpos(line, old_percentage)
            new_x = self.compute_lyric_xpos(line, percentage)
            if old_x != new_x:
                self.priv_update_shape = True
        self.window.queue_draw()        
        
        
    def set_current_line(self, line):
        if 0 <= line <= self.get_line_count():
            self.osd_current_line = line
            
    def get_current_line(self):
        return self.osd_current_line
        
    def set_line_count(line_count):
        if 1 <= line_count <= 2:
            self.osd_line_count = line_count
            self.queue_resize()
            self.queue_reshape()
    
    def get_line_count(self):
        return self.osd_line_count
    
    def set_current_percentage(self, percentage):
        self.set_percentage(self.osd_current_line, percentage)
        
    def get_current_percentage(self):
        return self.osd_percentage[self.osd_current_line]
    
    def set_line_alignment(self, line, alignment):
        if line < 0 or line > self.osd_line_count:
            return
        if alignment < 0.0:
            alignment = 0.0
        elif alignment > 1.0:    
            alignment = 1.0
        self.osd_line_alignment[line] = alignment    
        self.update_lyric_rect(line)
        self.queue_reshape()
        self.window.queue_draw()
        
    def set_active_colors(self, top_color, middle_color, bottom_color):    
        pass
    
    def set_inactive_colors(self, top_color, middle_color, bottom_color):
        pass
    
    def set_translucent_on_mouse_over(self, value):
        self.osd_translucent_on_mouse_over = value
        
    def get_translucent_on_mouse_over(self):   
        return self.osd_translucent_on_mouse_over
        
    def move_window(self, x, y):
        self.move_resize(x, y, self.compute_window_width, self.compute_window_height, DRAG_MOVE)
        
    def get_window_mode(self):    
        return self.priv_mode
        
    def get_window_pos(self):    
        return (self.priv_raw_x, self.priv_raw_y)
    
    def get_blur_radius(self):
        return self.priv_blur_radius
    
    def set_window_bg(self, pixbuf):
        self.osd_bg_pixbuf = pixbuf
        self.window.queue_draw()
        
    def set_lyric(self, line, lyric):    
        self.osd_lyrics[line] = lyric
        self.update_lyric_surface(line)
        self.update_window_shape()
        self.window.queue_draw()
        
        if line == 1:
            self.time_out = 0.0
            gobject.timeout_add(50, self.test_cb)
            
    def test_cb(self):        
        self.time_out += 0.005
        self.set_percentage(1, self.time_out)
        if self.time_out > 1:
            return False
        return True
        
lrc_window = OsdWindow()    

if __name__ == "__main__":        
    window = OsdWindow()
    window.set_lyric(0, "把你捧在手上 虔诚地焚香")
    window.set_lyric(1, "剪下一段烛光 将经纶点亮, 不求荡气回肠 只求爱一场, 爱到最后受了伤 哭得好绝望, 我用尽一生一世 来将你供养")
    window.set_percentage(0, 0.2)
    window.set_percentage(1, 0.5)
    # window.set_line_alignment(0, 0.0)
    # window.set_locked(True)
    window.window.show_all()
    print window.get_min_width()
    gtk.main()
   
    
