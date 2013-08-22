#! /usr/bin/env python
# -*- coding: utf-8 -*-

import gtk
import gobject
import cairo
import pango

from dtk.ui.draw import draw_pixbuf, draw_text
from dtk.ui.utils import is_left_button, cairo_disable_antialias, set_cursor, get_widget_root_coordinate, WIDGET_POS_TOP_LEFT
from dtk.ui.cache_pixbuf import CachePixbuf


from widget.skin import app_theme
from utils import color_hex_to_cairo
from widget.ui_utils import draw_round_rectangle_with_triangle

class ProgressBar(gtk.Button):
    
    __gsignals__ = { 
        "value-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,)),
                     }
    
    def __init__(self, value=100, lower=0, upper=100, step=5):
        gtk.Button.__init__(self)
        
        # Init data.
        self.__value = value
        self.__lower = lower
        self.__upper = upper
        self.__step = step
        self.drag_flag = False
        self.move_flag = False
        
        # Init DPixbufs.
        self.bg_dpixbuf = app_theme.get_pixbuf("scalebar/bg.png")
        
        self.point_dpixbuf = app_theme.get_pixbuf("scalebar/point_normal.png")
        self.point_width = self.point_dpixbuf.get_pixbuf().get_width()        
        
        # Init Dcolors.
        self.fg_left_dcolor = app_theme.get_color("progressBarLeft")
        self.fg_right_dcolor = app_theme.get_color("progressBarRight")
        
        # self.progressbar_tip = progressBarTip()
        
        # Init Sizes.
        self._value = lower
        self.padding_x = 0
        self.padding_y = 0
        self.progress_x = 0
        self.point_offset = 0

        self.fg_offset = self.bg_offset = 0
        self.default_height = self.bg_dpixbuf.get_pixbuf().get_height()
        
        # Init CachePixbufs
        self.bg_cache_pixbuf = CachePixbuf()
        self.fg_cache_pixbuf = CachePixbuf()
        
        
        self.set_size_request(-1, self.default_height)        
        
        # Init Events.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.connect("expose-event", self.on_expose_event)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("motion-notify-event", self.on_motion_notify_event)
        self.connect("button-release-event", self.on_button_release_event)
        # self.connect("enter-notify-event", self.on_enter_notify_event)
        # self.connect("leave-notify-event", self.on_leave_notify_event)
        
        
    def set_range(self, lower, upper):    
        self.__lower = lower
        self.__upper = upper
        
        self.queue_draw()
        
    @property        
    def progress_width(self):        
        return self.allocation.width - getattr(self, "point_width", 0)
    
    @property
    def virtual_width(self):
        return self.allocation.width
            
    def value_to_width(self, value):    
        return value / float(self.__upper) * self.progress_width
    
    def width_to_value(self, width):
        return width / float(self.progress_width) * self.__upper 
    
    @property
    def current_progress_width(self):
        return int(self.value_to_width(self.value))
    
    def get_size(self):
        rect = self.allocation
        return (rect.width, rect.height)    
    
    def emit_value_changed(self):        
        self.emit("value-changed", self.value)
    
    def update_progress_width(self, event, emit=False):
        progress_width = int(event.x - self.point_width)
        
        if progress_width < 0:
            progress_width = 0

        elif progress_width > self.progress_width:    
            progress_width = self.progress_width
            
        self.set_value(self.width_to_value(progress_width), emit)            
            
        self.queue_draw()
        
    def on_expose_event(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        self.draw_progress_bar(cr, rect)
        return True
    
    def on_enter_notify_event(self, widget, event):
        self.show_progressbar_tip(event)
        
    def on_leave_notify_event(self, widget, event):    
        self.hide_progressbar_tip()
    
    def adjust_event_coords(self, event):    
        _, y = get_widget_root_coordinate(self, pos_type=WIDGET_POS_TOP_LEFT)
        x, _ = event.get_root_coords()
        return x, y
    
    def show_progressbar_tip(self, event):
        self.progressbar_tip.move_to(*self.adjust_event_coords(event))
        self.progressbar_tip.show_all()
        
    def hide_progressbar_tip(self):    
        # self.progressbar_tip.hide_all()
        pass
        
    def draw_progress_bar(self, cr, rect):                    
        
        # Draw progressbar background.
        bg_height = self.bg_dpixbuf.get_pixbuf().get_height()
        self.bg_cache_pixbuf.scale(self.bg_dpixbuf.get_pixbuf(), self.virtual_width,
                                   bg_height)

        # bg_y = rect.y + (rect.height - bg_height) / 2
        draw_pixbuf(cr, self.bg_cache_pixbuf.get_cache(), rect.x + self.bg_offset, rect.y)
        
        # Draw progressbar foreground.
        if self.current_progress_width > 0:
            fg_height = self.default_height
            fg_y = rect.y
            lg_width = int(self.current_progress_width)
            pat = cairo.LinearGradient(rect.x + self.fg_offset, fg_y, rect.x + self.fg_offset + lg_width, fg_y)
            pat.add_color_stop_rgb(0.7, *color_hex_to_cairo(self.fg_left_dcolor.get_color()))
            pat.add_color_stop_rgb(1.0, *color_hex_to_cairo(self.fg_right_dcolor.get_color()))
            cr.set_operator(cairo.OPERATOR_OVER)
            cr.set_source(pat)
            cr.rectangle(rect.x + self.fg_offset, fg_y, lg_width, fg_height)
            cr.fill()
            
            with cairo_disable_antialias(cr):
                cr.set_line_width(1)
                cr.set_source_rgba(1, 1, 1, 0.5)
                cr.move_to(rect.x, fg_y + 1)                
                cr.rel_line_to(lg_width, 0)
                cr.stroke()
        
        # Draw point.
        point_y = rect.y + (rect.height - self.point_dpixbuf.get_pixbuf().get_height()) / 2
        
            
        draw_pixbuf(cr, self.point_dpixbuf.get_pixbuf(), 
                    rect.x + self.current_progress_width, 
                    point_y)
            
    def on_button_press_event(self, widget, event):
        self.update_progress_width(event, emit=True)        
        self.drag_flag = True
        self.queue_draw()
    
    def on_motion_notify_event(self, widget, event):
        # self.show_progressbar_tip(event)
        
        if self.drag_flag:
            self.update_progress_width(event, emit=False)
        self.queue_draw()

    def on_button_release_event(self, widget, event):
        self.drag_flag = False
        self.queue_draw()
        
    def get_value(self):    
        return self._value
    
    def set_value(self, value, emit=False):
        self._value = value
        if emit:
            self.emit_value_changed()
        self.queue_draw()
        
    value = property(get_value, set_value)
        
SHADOW_VALUE = 2 
ARROW_WIDTH = 10
ARROW_HEIGHT = ARROW_WIDTH / 2

from dtk_cairo_blur import gaussian_blur
from dtk.ui.utils import alpha_color_hex_to_cairo

    
class progressBarTip(gtk.Window):
    '''
    class docs
    '''
	
    def __init__(self):
        '''
        init docs
        '''
        gtk.Window.__init__(self)
        
        self.set_colormap(gtk.gdk.Screen().get_rgba_colormap() or 
                          gtk.gdk.Screen().get_rgb_colormap())
        
        self.set_keep_above(True)
        self.set_decorated(False)
        self.set_app_paintable(True)
        self.set_skip_pager_hint(True)
        self.set_skip_taskbar_hint(True)
        self.set_position(gtk.WIN_POS_NONE)
        
        self.surface = None
        self.surface_x = SHADOW_VALUE
        self.surface_y = SHADOW_VALUE
        self.surface_border = SHADOW_VALUE
        self.arrow_width = ARROW_WIDTH
        self.arrow_height = ARROW_HEIGHT
        self.radius = 5
        self.pos_type = gtk.POS_BOTTOM
        self.offset = 25        
        self.reset_surface_flag = False
        
        self.content = "03:12"
        self.shadow_color = ("#000000", 0.6)
        self.mask_color = ("#ffffff", 0.8)
        self.border_out_color = ("#000000", 1.0)
        self.set_redraw_on_allocate(True)
        
        self.drawing = gtk.Alignment()
        self.drawing.set_redraw_on_allocate(True)
        self.drawing.connect("expose-event", self.on_expose_event)
        self.add(self.drawing)
                
        self.set_size_request(60, 40)
        
    def compute_shadow(self, rect):    
        self.surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, rect.width, rect.height)
        surface_cr = cairo.Context(self.surface)
        
        draw_round_rectangle_with_triangle(surface_cr, 
                                           rect,
                                           self.radius, 
                                           self.arrow_width, self.arrow_height, self.offset,
                                           border=5,
                                           pos_type=self.pos_type)
        
        surface_cr.set_line_width(2)
        surface_cr.set_source_rgba(*alpha_color_hex_to_cairo(self.shadow_color))
        surface_cr.stroke_preserve()
        gaussian_blur(self.surface, 2)
        
        # border.
        # out border.
        surface_cr.clip()
        draw_round_rectangle_with_triangle(surface_cr, 
                                           rect,
                                           self.radius, 
                                           self.arrow_width, self.arrow_height, self.offset,
                                           border=6,
                                           pos_type=self.pos_type)
        surface_cr.set_source_rgba(*alpha_color_hex_to_cairo(self.mask_color))
        surface_cr.set_line_width(1)
        surface_cr.fill()
        
        # in border.
        # surface_cr.reset_clip()
        # draw_round_rectangle_with_triangle(surface_cr, 
        #                                    rect,
        #                                    self.radius, 
        #                                    self.arrow_width, self.arrow_height, self.offset,
        #                                    border=2,
        #                                    pos_type=self.pos_type) 
        
        # surface_cr.set_source_rgba(1, 1, 1, 1.0) # set in border color.
        # surface_cr.set_line_width(self.border_width)
        # surface_cr.fill()
        
    def on_expose_event(self, widget, event):
        '''
        docs
        '''
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.set_source_rgba(1, 1, 1, 0)
        cr.rectangle(*rect)
        cr.paint()
        
        cr.set_operator(cairo.OPERATOR_OVER)
        
        
        if not self.surface or self.reset_surface_flag:
            self.compute_shadow(rect)
        cr.set_source_surface(self.surface, 0, 0)    
        cr.paint()
        
        draw_text(cr, self.content, 
                  rect.x + 6, rect.y + 6, rect.width - 6 * 2, rect.height - 6 *2 - self.arrow_height, 
                  9, "#707070", 
                  alignment=pango.ALIGN_CENTER)
        
        return True
    
    def reset_surface(self):
        self.reset_surface_flag = True
        
    def move_to(self, x, y):    
        self.move(int(x - self.allocation.width / 2), int(y - self.allocation.height + 3))
        
        
        
