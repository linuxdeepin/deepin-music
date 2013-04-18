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

import gobject
import pango
import gtk
from dtk.ui.draw import draw_pixbuf, draw_text, color_hex_to_cairo
from dtk.ui.treeview import TreeItem

import utils
from widget.skin import app_theme
from widget.ui_utils import (draw_single_mask, render_item_text, draw_separator)
        
class CategoryTreeItem(TreeItem):    
    def __init__(self, title, category):
        TreeItem.__init__(self)
        self.column_index = 0
        self.side_padding = 5
        self.item_height = 37
        self.title = title
        self.item_width = 121
        self.padding_x = 10
        self.category = category
        
        if self.category.startswith("region"):
            self.init_icon_dpixbuf("region")
        elif self.category.startswith("genre"):    
            self.init_icon_dpixbuf("genre")
        else:    
            self.init_icon_dpixbuf("composite")
            
        self.icon_width = self.normal_dpixbuf.get_pixbuf().get_width()
            
    def init_icon_dpixbuf(self, name):        
        self.normal_dpixbuf = app_theme.get_pixbuf("webcast/%s_normal.png" % name)
        self.press_dpixbuf = app_theme.get_pixbuf("webcast/%s_press.png" % name)
        
    def get_height(self):    
        return self.item_height
    
    def get_column_widths(self):
        return (self.item_width,)
    
    def get_column_renders(self):
        return (self.render_title,)
    
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
        
    def emit_redraw_request(self):    
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def select(self):        
        self.is_select = True
        self.emit_redraw_request()
        
    def render_title(self, cr, rect):        
        # Draw select background.
            
        if self.is_select:    
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width - 2, rect.height, "globalItemHighlight")
        elif self.is_hover:
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width - 2, rect.height, "globalItemHover")
        
        if self.is_select:
            text_color = "#FFFFFF"
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        if self.is_select:    
            icon_pixbuf = self.press_dpixbuf.get_pixbuf()
        else:    
            icon_pixbuf = self.normal_dpixbuf.get_pixbuf()
            
        rect.x += self.padding_x    
        rect.width -= self.padding_x * 2
        icon_y = rect.y + (rect.height - icon_pixbuf.get_height()) / 2
        draw_pixbuf(cr,icon_pixbuf, rect.x, icon_y)    
        rect.x += self.icon_width + self.padding_x
        rect.width -= self.icon_width - self.padding_x
        
        draw_text(cr, self.title, rect.x,
                  rect.y, rect.width,
                  rect.height, text_size=10, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT)    
        
    def expand(self):
        pass
    
    def unexpand(self):
        pass
    
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
        
    def button_press(self, column, offset_x, offset_y):
        pass
    
    def single_click(self, column, offset_x, offset_y):
        pass

    def double_click(self, column, offset_x, offset_y):
        pass        
    
    def draw_drag_line(self, drag_line, drag_line_at_bottom=False):
        pass
    
    
class CollectTreeItem(TreeItem):    
    def __init__(self, title):
        TreeItem.__init__(self)
        self.column_index = 0
        self.side_padding = 5

        self.title = title
        self.item_width = 121
        self.padding_x = 10
        self.padding_y = 5
        self.item_height = 37 + self.padding_y * 2 + 1
        self.collect_flag = True
        self.init_icon_dpixbuf()
        self.icon_width = self.normal_dpixbuf.get_pixbuf().get_width()
        
    def init_icon_dpixbuf(self):        
        self.normal_dpixbuf = app_theme.get_pixbuf("webcast/favorite_normal.png")
        self.press_dpixbuf = app_theme.get_pixbuf("webcast/favorite_press.png")
        
    def get_height(self):    
        return self.item_height
    
    def get_column_widths(self):
        return (self.item_width,)
    
    def get_column_renders(self):
        return (self.render_title,)
    
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
        
    def emit_redraw_request(self):    
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def select(self):        
        self.is_select = True
        self.emit_redraw_request()
        
    def render_title(self, cr, rect):        
        # Draw select background.
        
        rect.y += self.padding_y
        # draw separator.
        draw_separator(cr, rect.x, 
                       rect.y,
                       rect.width, 1
                       )
        rect.y += self.padding_y + 1
        rect.height -= self.padding_y * 2 - 1
                    
        if self.is_select:    
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width - 2, rect.height, "globalItemHighlight")
        elif self.is_hover:
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width - 2, rect.height, "globalItemHover")
        
        if self.is_select:
            text_color = "#FFFFFF"
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        if self.is_select:    
            icon_pixbuf = self.press_dpixbuf.get_pixbuf()
        else:    
            icon_pixbuf = self.normal_dpixbuf.get_pixbuf()
            
        rect.x += self.padding_x    
        rect.width -= self.padding_x * 2
        icon_y = rect.y + (rect.height - icon_pixbuf.get_height()) / 2
        draw_pixbuf(cr,icon_pixbuf, rect.x, icon_y)    
        rect.x += self.icon_width + self.padding_x
        rect.width -= self.icon_width - self.padding_x
        
        draw_text(cr, self.title, rect.x,
                  rect.y, rect.width,
                  rect.height, text_size=10, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT)    
        
    def expand(self):
        pass
    
    def unexpand(self):
        pass
    
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
        
    def button_press(self, column, offset_x, offset_y):
        pass
    
    def single_click(self, column, offset_x, offset_y):
        pass

    def double_click(self, column, offset_x, offset_y):
        pass        
    
    def draw_drag_line(self, drag_line, drag_line_at_bottom=False):
        pass
    
        
OLD_ITEM_COLORS = ["#98B54A", "#AA41B2", "#C9493B", "#4EA549",
               "#CB8D2A", "#54A9B4", "#A0428B", "#4965A3",
               "#5C94E7", "#9F415D"]         
ITEM_COLORS = ["#C4695F","#BF56A7","#A4BB6E","#C79C57",
               "#5DA053", "#6D7EAB","#79B0B9","#62BCDD","#D8BF53",]

distance_random = utils.DistanceRandom(range(len(ITEM_COLORS)))
        
class WebcastIconItem(gobject.GObject):
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, title, is_composited=False):
        gobject.GObject.__init__(self)
        
        self.cell_width = 83
        self.padding_x = 10
        self.padding_y = 5
        self.hover_padding_x = self.padding_x / 2        
        self.hover_padding_y = self.padding_y / 2

        color_index = utils.get_fixed_value(title, len(ITEM_COLORS))
        # color_index = distance_random.get()
        self.bg_color = ITEM_COLORS[color_index]
        self.title_padding_x = 10
        self.title_padding_y = 5
        self.title = title
        self.hover_color = "#34C0FE"
        self.hover_flag = False
        
        self.is_composited = is_composited
        
    def emit_redraw_request(self):    
        self.emit("redraw-request")
        
    def get_width(self):    
        return self.cell_width + self.padding_x * 2
    
    def get_height(self):
        return self.cell_width + self.padding_y * 2
    
    def render(self, cr, rect):
        # if self.hover_flag:
        #     padding_x = self.hover_padding_x
        #     padding_y = self.hover_padding_y
        # else:    
        
        padding_x = self.padding_x
        padding_y = self.padding_y
        
        rect.x += padding_x
        rect.y += padding_y
        rect.width -= padding_x * 2
        rect.height -= padding_y * 2
        
        cr.set_source_rgb(*color_hex_to_cairo(self.bg_color))
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
        text_rect = gtk.gdk.Rectangle(*rect)
        text_rect.x += self.title_padding_x
        text_rect.y += self.title_padding_y
        text_rect.width -= self.title_padding_x * 2
        text_rect.height -= self.title_padding_y * 2
        
        if self.hover_flag:
            cr.set_source_rgba(0, 0, 0, 0.3)
            cr.rectangle(*rect)
            cr.fill()
            
        
        cr.save()
        cr.rectangle(text_rect.x, text_rect.y, text_rect.width, text_rect.height)
        cr.clip()
        
        draw_text(cr, self.title, text_rect.x, text_rect.y, text_rect.width, text_rect.height, 
                  text_color="#FFFFFF", wrap_width=text_rect.width,
                  text_size=12, alignment=pango.ALIGN_LEFT)
        cr.restore()
        
        cr.rectangle(*rect)
        if self.hover_flag:
            side_color = self.hover_color
        else:    
            side_color = "#D6D6D6"
        cr.set_source_rgb(*color_hex_to_cairo(side_color))
        cr.stroke()
                
    def icon_item_motion_notify(self, x, y):    
        self.hover_flag = True
        self.emit_redraw_request()
        
    def icon_item_lost_focus(self):    
        self.hover_flag = False
        self.emit_redraw_request()
        
    def icon_item_highlight(self):    
        self.highlight_flag = True
        self.emit_redraw_request()
        
    def icon_item_normal(self):    
        self.highlight_flag = False
        self.emit_redraw_request()
        
    def icon_item_button_press(self, x, y):    
        pass
    
    def icon_item_button_release(self, x, y):
        pass
    
    def icon_item_single_click(self, x, y):
        pass
    
    def icon_item_double_click(self, x, y):
        pass
    
    def icon_item_release_resource(self):
        # Release pixbuf resource.
        # del self.pixbuf
        # self.pixbuf = None
        
        # Return True to tell IconView call gc.collect() to release memory resource.
        return False
    
class CompositeIconItem(gobject.GObject):
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, title, category, is_composited=False):
        gobject.GObject.__init__(self)
        
        self.cell_width = 83
        self.padding_x = 10
        self.padding_y = 5
        self.hover_padding_x = self.padding_x / 2        
        self.hover_padding_y = self.padding_y / 2

        color_index = utils.get_fixed_value(title, len(ITEM_COLORS))
        # color_index = distance_random.get()
        self.bg_color = ITEM_COLORS[color_index]
        self.title_padding_x = 10
        self.title_padding_y = 5
        self.title = title
        self.category = category
        self.hover_color = "#34C0FE"
        self.hover_flag = False
        
        self.is_composited = is_composited
        
    def emit_redraw_request(self):    
        self.emit("redraw-request")
        
    def get_width(self):    
        return self.cell_width + self.padding_x * 2
    
    def get_height(self):
        return self.cell_width + self.padding_y * 2
    
    def render(self, cr, rect):
        # if self.hover_flag:
        #     padding_x = self.hover_padding_x
        #     padding_y = self.hover_padding_y
        # else:    
        
        padding_x = self.padding_x
        padding_y = self.padding_y
        
        rect.x += padding_x
        rect.y += padding_y
        rect.width -= padding_x * 2
        rect.height -= padding_y * 2
        
        cr.set_source_rgb(*color_hex_to_cairo(self.bg_color))
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
        text_rect = gtk.gdk.Rectangle(*rect)
        text_rect.x += self.title_padding_x
        text_rect.y += self.title_padding_y
        text_rect.width -= self.title_padding_x * 2
        text_rect.height -= self.title_padding_y * 2
        
        if self.hover_flag:
            cr.set_source_rgba(0, 0, 0, 0.3)
            cr.rectangle(*rect)
            cr.fill()
            
        
        cr.save()
        cr.rectangle(text_rect.x, text_rect.y, text_rect.width, text_rect.height)
        cr.clip()
        
        draw_text(cr, self.title, text_rect.x, text_rect.y, text_rect.width, text_rect.height, 
                  text_color="#FFFFFF", wrap_width=text_rect.width,
                  text_size=12, alignment=pango.ALIGN_LEFT)
        cr.restore()
        
        cr.rectangle(*rect)
        if self.hover_flag:
            side_color = self.hover_color
        else:    
            side_color = "#D6D6D6"
        cr.set_source_rgb(*color_hex_to_cairo(side_color))
        cr.stroke()
                
    def icon_item_motion_notify(self, x, y):    
        self.hover_flag = True
        self.emit_redraw_request()
        
    def icon_item_lost_focus(self):    
        self.hover_flag = False
        self.emit_redraw_request()
        
    def icon_item_highlight(self):    
        self.highlight_flag = True
        self.emit_redraw_request()
        
    def icon_item_normal(self):    
        self.highlight_flag = False
        self.emit_redraw_request()
        
    def icon_item_button_press(self, x, y):    
        pass
    
    def icon_item_button_release(self, x, y):
        pass
    
    def icon_item_single_click(self, x, y):
        pass
    
    def icon_item_double_click(self, x, y):
        pass
    
    def icon_item_release_resource(self):
        # Release pixbuf resource.
        # del self.pixbuf
        # self.pixbuf = None
        
        # Return True to tell IconView call gc.collect() to release memory resource.
        return False
    
class WebcastListItem(gobject.GObject):
    
    __gsignals__ = {"redraw-request" : ( gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, webcast):
        gobject.GObject.__init__(self)
        
        self.index = None
        self.webcast_normal_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_normal.png").get_pixbuf()
        self.webcast_press_pixbuf = app_theme.get_pixbuf("webcast/webcast_large_press.png").get_pixbuf()
        self.collect_normal_pixbuf = app_theme.get_pixbuf("webcast/collect_normal.png").get_pixbuf()
        self.collect_press_pixbuf = app_theme.get_pixbuf("webcast/collect_press.png").get_pixbuf()
        self.collect_hover_pixbuf = app_theme.get_pixbuf("webcast/collect_hover.png").get_pixbuf()
        self.webcast = webcast
        self.title = webcast.get("title")
        self.is_collected = False
        
        self.__update_size()
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
        
    def __update_size(self):    
        self.title = self.webcast.get_str("title")
        
        self.webcast_icon_padding_x = 10
        self.webcast_icon_padding_y = 5
        self.webcast_icon_w = self.webcast_normal_pixbuf.get_width()
        self.webcast_icon_h = self.webcast_normal_pixbuf.get_height()
        
        self.title_padding_x = 5
        self.title_padding_y = 5
        
        self.collect_icon_padding_x = 2
        self.collect_icon_padding_y = 5
        self.collect_icon_w = self.collect_normal_pixbuf.get_width()
        self.collect_icon_h = self.collect_normal_pixbuf.get_height()
        
    def render_webcast_icon(self, cr, rect, in_select, in_highlight):    
        icon_x = rect.x + self.webcast_icon_padding_x 
        icon_y = rect.y + (rect.height - self.webcast_icon_h) / 2
        if in_highlight:
            pixbuf = self.webcast_press_pixbuf
        else:    
            pixbuf = self.webcast_normal_pixbuf
        draw_pixbuf(cr, pixbuf, icon_x, icon_y)
        
    def render_title(self, cr, rect, in_select, in_highlight):    
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        
        if in_highlight:
            color = app_theme.get_color("simpleSelectItem").get_color()
        else:    
            color = app_theme.get_color("labelText").get_color()
        
        # if error:    
        #     color = "#ff0000"        
            
        content = utils.xmlescape(self.title)    
        draw_text(cr, content, rect.x, rect.y, rect.width, rect.height, 
                  text_size=9, text_color=color, alignment=pango.ALIGN_LEFT)    
        
    def render_collect_icon(self, cr, rect, in_select, in_highlight):    
        icon_y = rect.y + (rect.height - self.collect_icon_h) / 2
        rect.x += self.collect_icon_padding_x
        if self.webcast.get("collected", False):
            pixbuf = self.collect_press_pixbuf
        else:    
            if in_highlight:
                pixbuf = self.collect_hover_pixbuf
            else:    
                pixbuf = self.collect_normal_pixbuf
                
        draw_pixbuf(cr, pixbuf, rect.x , icon_y)
        
    def render_block(self, cr, rect, in_select, in_highlight):    
        pass
    
    def get_column_sizes(self):
        return [
            (self.webcast_icon_w + self.webcast_icon_padding_x * 2, self.webcast_icon_h + self.webcast_icon_padding_y * 2),
            (100, 37),
            (self.collect_icon_w + self.collect_icon_padding_x * 2, 30),
            (25, 1)
            ]
    
    def get_renders(self):
        return [self.render_webcast_icon, self.render_title, self.render_collect_icon, self.render_block]
    
    def toggle_is_collected(self):
        if self.is_collected:
            self.is_collected = False
        else:    
            self.is_collected = True
        self.webcast["collect"] = self.is_collected
        self.emit_redraw_request()
        
    def set_draw_collect(self, value):    
        self.draw_collect_flag = value
        self.emit_redraw_request()
        
    def get_webcast(self):    
        return self.webcast
    
    def update_webcast(self, webcast):
        self.webcast = webcast
        self.title = webcast.get("title")
        self.emit_redraw_request()
        
        
    def __hash__(self):    
        return hash(self.webcast.get("uri"))
    
    def __cmp__(self, other_item):
        if not other_item: return -1
        try:
            return cmp(self.webcast.get("search"), other_item.webcast.get("search"))
        except AttributeError: return -1
        
    def __eq__(self, other_item):    
        try:
            return self.webcast.get("uri") == other_item.webcast.get("uri")
        except:
            return False
        
    def __repr__(self):    
        return "<Webcast %s>" % self.webcast.get("uri")
    
