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

import pango

from dtk.ui.treeview import TreeItem
from dtk.ui.draw import draw_text, draw_pixbuf

from widget.ui_utils import (draw_single_mask)
from widget.skin import app_theme

class ExpandItem(TreeItem):
    
    def __init__(self, title, allocate_widget=None, column_index=0):
        TreeItem.__init__(self)
        self.column_index = column_index
        self.side_padding = 5
        self.item_height = 37
        self.title = title
        self.item_width = 36
        self.allocate_widget = allocate_widget
        self.child_items = []
        
        self.title_padding_x = 30
        self.arrow_padding_x = 10        
        
        # Init dpixbufs.
        self.down_normal_dpixbuf = app_theme.get_pixbuf("arrow/down_normal.png")
        self.down_press_dpixbuf = app_theme.get_pixbuf("arrow/down_press.png")
        self.right_normal_dpixbuf = app_theme.get_pixbuf("arrow/right_normal.png")
        self.right_press_dpixbuf = app_theme.get_pixbuf("arrow/right_press.png")
        
        
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
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
        if self.is_select:
            text_color = "#FFFFFF"
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        # draw arrow    
        if self.is_expand:    
            if self.is_select:
                arrow_pixbuf = self.down_press_dpixbuf.get_pixbuf()
            else:
                arrow_pixbuf = self.down_normal_dpixbuf.get_pixbuf()
        else:        
            if self.is_select:
                arrow_pixbuf = self.right_press_dpixbuf.get_pixbuf()
            else:
                arrow_pixbuf = self.right_normal_dpixbuf.get_pixbuf()
                
        arrow_x = rect.x + self.arrow_padding_x
        arrow_y = rect.y + (rect.height - arrow_pixbuf.get_height()) / 2
        draw_pixbuf(cr, arrow_pixbuf, arrow_x, arrow_y)
        draw_text(cr, self.title, rect.x + self.title_padding_x, rect.y, 
                  rect.width - self.title_padding_x, rect.height, text_size=10, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT)    
        
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
        
    def button_press(self, column, offset_x, offset_y):
        pass
    
    def single_click(self, column, offset_x, offset_y):
        if self.is_expand:
            self.unexpand()
        else:
            self.expand()

    def double_click(self, column, offset_x, offset_y):
        # if self.is_expand:
        #     self.unexpand()
        # else:
        #     self.expand()
        pass
    
    def add_child_item(self):        
        self.add_items_callback(self.child_items, self.row_index + 1)
    
    def delete_child_item(self):
        self.delete_items_callback(self.child_items)
        
    def expand(self):
        self.is_expand = True
        self.add_child_item()
        self.emit_redraw_request()
    
    def unexpand(self):
        self.is_expand = False
        self.delete_child_item()
        self.emit_redraw_request()
        
    def try_to_expand(self):    
        if not self.is_expand:
            self.expand()
        
    def add_childs(self, child_items, pos=None, expand=False):    
        items = []
        for child_item in child_items:
            items.append(NormalItem(child_item[0], child_item[1], self.column_index + 1))
            
        for item in items:    
            item.parent_item = self
            
        if pos is not None:    
            for item in items:
                self.child_items.insert(pos, item)
                pos += 1
        else:            
            self.child_items.extend(items)
            
        if expand:    
            self.expand()
            
    def __repr__(self):        
        return "<ExpandItem %s>" % self.title
        
class NormalItem(TreeItem):        
    def __init__(self, title, allocate_widget, column_index=0):
        TreeItem.__init__(self)
        self.column_index = column_index
        self.side_padding = 5
        if column_index > 0:
            self.item_height = 30
        else:    
            self.item_height = 37
            
        self.title = title
        self.item_width = 36
        self.allocate_widget = allocate_widget
        self.title_padding_x = 30
        self.column_offset = 15
        
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
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
        if self.is_select:
            text_color = "#FFFFFF"
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
            
        column_offset = self.column_offset * self.column_index    
        draw_text(cr, self.title, rect.x + self.title_padding_x + column_offset,
                  rect.y, rect.width - self.title_padding_x - column_offset ,
                  rect.height, text_size=10, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT)    
        
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

    def __repr__(self):        
        return "<NormalItem %s>" % self.title
    
