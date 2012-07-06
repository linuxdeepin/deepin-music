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

from dtk.ui.utils import alpha_color_hex_to_cairo
from dtk.ui.line import draw_vlinear
from dtk.ui.draw import draw_text
from dtk.ui.line import HSeparator
from widget.skin import app_theme
from utils import color_hex_to_cairo

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
    cairo_color = color_hex_to_cairo(app_theme.get_color(color_name).get_color())
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
    
def render_item_text(cr, content, rect, in_select, in_highlight, align=pango.ALIGN_LEFT, font_size=8, error=False):
    if in_select or in_highlight:
        color = app_theme.get_color("simpleSelectItem").get_color()
    else:    
        color = app_theme.get_color("labelText").get_color()

    if error:    
        color = "#ff0000"        
    draw_text(cr, content, rect.x, rect.y, rect.width, rect.height, font_size, color, alignment=align)
    
def create_separator_box(padding_x=0, padding_y=0):    
    separator_box = HSeparator(
        app_theme.get_shadow_color("hSeparator").get_color_info(),
        padding_x, padding_y)
    return separator_box

def create_right_align():    
    align = gtk.Alignment()
    align.set(0, 0, 0, 1)
    return align
    
def create_left_align():
    align = gtk.Alignment()
    align.set(0, 0, 1, 0)
    return align
    
def create_upper_align():
    align = gtk.Alignment()
    align.set(1, 0, 0, 0)
    return align

def create_bottom_align():
    align = gtk.Alignment()
    align.set(0, 1, 0, 0)
    return align

    
