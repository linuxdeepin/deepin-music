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

import pango
import gtk
import pangocairo
import math

from dtk.ui.utils import (alpha_color_hex_to_cairo, cairo_disable_antialias)
from dtk.ui.line import draw_vlinear, draw_hlinear
from dtk.ui.draw import draw_text
from dtk.ui.line import HSeparator
from dtk.ui.constant import DEFAULT_FONT
from widget.skin import app_theme
from utils import color_hex_to_cairo
from constant import DEFAULT_FONT_SIZE

import utils

def is_in_rect((tx, ty), rect):
    if rect.x <= tx <= rect.x + rect.width and \
            rect.y <= ty <= rect.y + rect.height:
        return True
    else:
        return False

def container_remove_all(container):
    ''' Removee all child widgets for container. '''
    container.foreach(lambda widget: container.remove(widget))

def switch_tab(notebook_box, tab_box):
    '''Switch tab 1.'''
    container_remove_all(notebook_box)
    notebook_box.add(tab_box)
    notebook_box.show_all()

def draw_alpha_mask(cr, x, y, width, height, color_name):
    if not isinstance(color_name, tuple):
        color_info = app_theme.get_alpha_color(color_name).get_color_info()
    else:    
        color_info = color_name

    cr.set_source_rgba(*alpha_color_hex_to_cairo(color_info))
    cr.rectangle(x, y, width, height)
    cr.fill()
    
def draw_single_mask(cr, x, y, width, height, color_name):
    if color_name.startswith("#"):
        color = color_name
    else:    
        color = app_theme.get_color(color_name).get_color()
    cairo_color = color_hex_to_cairo(color)
    cr.set_source_rgb(*cairo_color)
    cr.rectangle(x, y, width, height)
    cr.fill()
    
def get_font_families():
    '''Get all font families in system.'''
    fontmap = pangocairo.cairo_font_map_get_default()
    return sorted(map(lambda f: f.get_name(), fontmap.list_families()))

def draw_item_mask(cr, x, y, width, height, name):    
    draw_vlinear(cr, x, y, width, height,
                 app_theme.get_shadow_color(name).get_color_info())
    
def render_text(cr, content, rect, color, font_size, align=pango.ALIGN_CENTER):    
    draw_text(cr, content, rect.x, rect.y, rect.width, rect.height, font_size, color, alignment=align)
    
def render_item_text(cr, content, rect, in_select, in_highlight, align=pango.ALIGN_LEFT, font_size=9, error=False):
    if in_highlight:
        color = app_theme.get_color("simpleSelectItem").get_color()
    else:    
        # color = app_theme.get_color("labelText").get_color()
        color = "#707070"

    if error:    
        color = "#ff0000"        
        
    content = utils.xmlescape(content)    
    draw_text(cr, content, rect.x, rect.y, rect.width, rect.height, font_size, color, alignment=align)    
    
def render_item_text2(cr, content, rect, in_select, in_highlight, align=pango.ALIGN_LEFT, font_size=9, error=False):
    if in_select:
        color = app_theme.get_color("simpleSelectItem").get_color()
    else:    
        # color = app_theme.get_color("labelText").get_color()
        color = "#707070"

    if error:    
        color = "#ff0000"        
        
    content = utils.xmlescape(content)    
    draw_text(cr, content, rect.x, rect.y, rect.width, rect.height, font_size, color, alignment=align)    
    
def create_separator_box(padding_x=0, padding_y=0):    
    separator_box = HSeparator(
        app_theme.get_shadow_color("hSeparator").get_color_info(),
        padding_x, padding_y)
    return separator_box

def draw_separator(cr, x, y, width, height):
    draw_hlinear(cr, x, y, width, height, 
                 app_theme.get_shadow_color("hSeparator").get_color_info())
    
def create_right_align(paddings=None):    
    align = gtk.Alignment()
    if paddings:
        align.set_padding(*paddings)
    align.set(0, 0, 0, 1)
    return align
    
def create_left_align(paddings=None):
    align = gtk.Alignment()
    if paddings:
        align.set_padding(*paddings)
    align.set(0, 0, 1, 0)
    return align
    
def create_upper_align(paddings=None):
    align = gtk.Alignment()
    if paddings:
        align.set_padding(*paddings)
    align.set(1, 0, 0, 0)
    return align

def create_bottom_align(paddings=None):
    align = gtk.Alignment()
    if paddings:
        align.set_padding(paddings=None)
    align.set(0, 1, 0, 0)
    return align

def set_widget_gravity(widget, gravity=(0, 0, 0, 0), paddings=(0, 0, 0, 0)):
    align = gtk.Alignment()
    align.set(*gravity)
    align.set_padding(*paddings)
    align.add(widget)
    return align
    
def set_widget_hcenter(widget):
    hbox = gtk.HBox()
    hbox.pack_start(create_right_align(), True, True)
    hbox.pack_start(widget, False, False)
    hbox.pack_start(create_left_align(), True, True)
    return hbox

def set_widget_vcenter(widget):
    vbox = gtk.VBox()
    vbox.pack_start(create_bottom_align(), True, True)
    vbox.pack_start(widget, False, True)
    vbox.pack_start(create_upper_align(), True, True)
    return vbox

# def set_widget_vcenter(widget):
#     align = gtk.Alignment()
#     align.set(0.5, 0.5, 0, 0)
#     align.add(widget)
#     return align
    
def set_widget_left(widget, paddings=None):
    hbox = gtk.HBox()
    hbox.pack_start(widget, False, False)
    hbox.pack_start(create_left_align(paddings), True, True)
    return hbox


def set_widget_right(widget, paddings=None):
    hbox = gtk.HBox()
    hbox.pack_start(create_right_align(paddings), True, True)
    hbox.pack_start(widget, False, False)    
    return hbox

def draw_range(cr, x, y, width, height, color_name):
    if color_name.startswith("#"):
        color = color_name
    else:    
        color = app_theme.get_color(color_name).get_color()
    cairo_color = color_hex_to_cairo(color)        
    with cairo_disable_antialias(cr):
        cr.set_line_width(1)
        cr.set_source_rgb(*cairo_color)
        cr.rectangle(x, y, width, height)
        cr.stroke()

def draw_line(cr, start, end, color_name):
    if color_name.startswith("#"):
        color = color_name
    else:    
        color = app_theme.get_color(color_name).get_color()
    cairo_color = color_hex_to_cairo(color)        
    with cairo_disable_antialias(cr):
        cr.set_line_width(1)
        cr.set_source_rgb(*cairo_color)
        cr.move_to(*start)
        cr.line_to(*end)
        cr.stroke()

def cairo_color_to_hex(cairo_color):
    '''
    Convert cairo color to hex color.
    
    @param rgb_color: (red, green, blue)
    @return: Return hex color.
    '''
    return "#%02X%02X%02X" % (cairo_color[0] * 255, cairo_color[1] * 255, cairo_color[2] * 255)

def set_widget_resize(widget1, widget2, sizes1=(80, 22), sizes2=(175, 22), spacing=5):
    main_box = gtk.HBox(spacing=spacing)
    sub_box1 = gtk.HBox()
    sub_box1.set_size_request(*sizes1)
    sub_box1.add(set_widget_right(widget1))
    main_box.pack_start(sub_box1)
    
    sub_box = gtk.HBox()
    sub_box.set_size_request(*sizes2)
    sub_box.add(set_widget_left(widget2))
    main_box.pack_start(sub_box)
    return main_box
    
    
    

def draw_round_rectangle_with_triangle(cr, 
                                       rect,
                                       radius,
                                       arrow_width, arrow_height, 
                                       offset=0, border=2, 
                                       pos_type=gtk.POS_TOP):
    
    x = rect.x + border
    y = rect.y + border
    w = rect.width - border * 2
    h = rect.height - border * 2
    
    # set position top, bottom.
    if pos_type == gtk.POS_TOP:    
        y += arrow_height
        h -= arrow_height
    elif pos_type == gtk.POS_BOTTOM:    
        h -= arrow_height
        
    # draw.
    cr.arc (x + radius,
            y + radius,
            radius,
            math.pi,
            math.pi * 1.5)

    if pos_type == gtk.POS_TOP:
        cr.line_to(offset, y) 
        cr.rel_line_to(arrow_width / 2.0, -arrow_height)
        cr.rel_line_to(arrow_width / 2.0, arrow_height)

    cr.arc(x + w - radius, y + radius,
            radius, math.pi * 1.5, math.pi * 2.0)
    
    cr.arc(x + w - radius,
           y + h - radius,
           radius,
           0,
           math.pi * 0.5)
    
    if pos_type == gtk.POS_BOTTOM:
        arrow_top = y + h
        cr.line_to(offset + arrow_width, arrow_top) 
        cr.rel_line_to(-arrow_width / 2.0, arrow_height)
        cr.rel_line_to(-arrow_width / 2.0, -arrow_height)
        
    cr.arc(x + radius,
           y + h - radius,
           radius,
           math.pi * 0.5,
           math.pi)
    
    cr.close_path()
