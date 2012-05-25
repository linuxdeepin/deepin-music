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
import gobject

from dtk.ui.constant import BUTTON_PRESS, BUTTON_NORMAL, BUTTON_HOVER
from dtk.ui.button import ImageButton
from dtk.ui.label import Label
from dtk.ui.draw import draw_vlinear, draw_pixbuf, draw_font
from dtk.ui.utils import widget_fix_cycle_destroy_bug, propagate_expose, container_remove_all
from dtk.ui.constant import ALIGN_START, ALIGN_MIDDLE
from widget.ui import app_theme

class BaseBar(gtk.VBox):
        
    def __init__(self, init_index=0):
        gtk.VBox.__init__(self)
        self.current_index = init_index
    
        
    def set_index(self, index):    
        self.queue_draw()
        self.current_index = index
        
    def get_index(self):    
        return self.current_index
    
class OptionBar(BaseBar):    
    
    def __init__(self, items, init_index=0, font_size=10, padding_left=20, padding_middle=0, padding_right=25):
        BaseBar.__init__(self, init_index)
        
        if items:
            for index, item in enumerate(items):
                simple_item = SimpleItem(
                    item, index, font_size, 25, padding_left, padding_middle, padding_right, self.set_index, self.get_index)
                self.pack_start(simple_item)
        self.show_all()        
        
class OptionTitleBar(BaseBar):
    
    def __init__(self, title, child_items=None):
        BaseBar.__init__(self, init_index=-1)

        title_item = SimpleItem(title, 0, 10, 25, 20, 0, 25, self.set_index, self.get_index)
        self.pack_start(title_item, False, True)
        self.child_box = gtk.VBox()
        if child_items:
            for index, item in enumerate(child_items):
                simple_item = SimpleItem(
                    item, index + 1 , 9 , 20, 35, 10, 25, self.set_index, self.get_index)
                self.child_box.pack_start(simple_item)
        self.pack_start(self.child_box)        
        self.show_all()        
        
    def add_items(self, child_items):
        if child_items:
            container_remove_all(self.child_box)            
            for index, item in enumerate(child_items):
                simple_item = SimpleItem(
                    item, index + 1 , 9 , 20, 35, 10, 25, self.set_index, self.get_index)
                self.child_box.pack_start(simple_item)
                
class SongPathBar(BaseBar):                
    
    def __init__(self, title_name, callback):
        BaseBar.__init__(self, init_index=-1)
        self.child_box = gtk.VBox()
        
        self.child_item_height = 20
        self.current_page = 1
        self.page_items_num = 0        
        self.items = []
        
        title_item = SimpleItem(
            (app_theme.get_pixbuf("filter/local_normal.png"),
             app_theme.get_pixbuf("filter/local_press.png"),
             title_name, callback), 0, 10, 25, 20, 10, 25, self.set_index, self.get_index, ALIGN_START)
        self.pack_start(title_item, False, False)
        self.pack_start(self.child_box, True, True)
        self.child_box.connect("size-allocate", self.size_change_cb)
        self.control_box = gtk.HBox()
        previous_align = self.create_simple_button("previous", self.update_current_page, "previous")
        next_align = self.create_simple_button("next", self.update_current_page, "next")
        self.info_label = Label("0/0", app_theme.get_color("labelText"), text_x_align=ALIGN_MIDDLE)
        self.info_label.set_size_request(50, 20)
        self.control_box.pack_start(previous_align, False, False)
        self.control_box.pack_start(self.info_label, False, False)
        self.control_box.pack_start(next_align, False, False)
        control_align = gtk.Alignment()
        control_align.set(1, 1, 0.5, 0.5)
        control_align.add(self.control_box)
        self.pack_start(control_align, False, False)
        
    def update_current_page(self, widget, name):    
        if not self.items or not self.page_items_num:
            return
        total_page = self.adjust_page()
        if name == "previous":
            new_current_page = self.current_page - 1
        else:    
            new_current_page = self.current_page + 1
            
        if new_current_page < 1:    
            new_current_page = 1
        elif new_current_page > total_page:    
            new_current_page = total_page
        if new_current_page == self.current_page:    
            return
        self.current_page = new_current_page
        self.update_items(None)
        self.set_index(-1)
        
    def get_item_num(self):    
        try:
            return self.child_box.allocation.height / self.child_item_height
        except:
            return 0
        
    def set_label(self, value):    
        self.info_label.set_text(value)
        
    def load_items(self, child_items):    
        if child_items:
            container_remove_all(self.child_box)
            for index, item in enumerate(child_items):
                simple_item = SimpleItem(
                    (app_theme.get_pixbuf("filter/folder_normal.png"),
                     app_theme.get_pixbuf("filter/folder_press.png"),
                     item[0], item[1], item[2]),
                    index + 1,   9, self.child_item_height, 35, 10, 25, self.set_index, self.get_index, ALIGN_START)
                self.child_box.pack_start(simple_item, False, False)
                
            if len(child_items) < self.page_items_num:    
                block_num = self.page_items_num - len(child_items)
                for i in range(block_num):
                    self.child_box.pack_start(self.create_block_box(), False, True)
                
            self.show_all()        

            
    def size_change_cb(self, widget, rect):        
        if rect.height > 0:
            new_num = rect.height / self.child_item_height
            if new_num == self.page_items_num:
                return
            else:
                self.page_items_num = new_num                                
                def fire():
                    self.update_items(None)
                gobject.idle_add(fire)    

    def update_items(self, new_items):            
        if new_items:
            self.items = new_items
            
        if not self.items or not self.page_items_num:
            return

        self.items.sort()
        self.update_label()
        total_page = self.adjust_page()
        if total_page == 1:
            page_items = self.items[:]
        elif self.current_page == 1:    
            page_items = self.items[:self.page_items_num]
        elif self.current_page == total_page and total_page > 1:
            page_items = self.items[(self.current_page - 1) * self.page_items_num:]
        else:    
            page_items = self.items[(self.current_page - 1) * self.page_items_num:self.current_page * self.page_items_num]

        self.load_items(page_items)
        
    def adjust_page(self):    
        if not self.items or not self.page_items_num:
            return
        
        total_num = len(self.items)
        if total_num <= self.page_items_num:    
            self.current_page = 1
            return 1
        
        mod_num = total_num % self.page_items_num        
        if mod_num:
            return total_num / self.page_items_num + 1
        else:
            return total_num / self.page_items_num

    def update_label(self):    
        total_page = self.adjust_page()
        self.set_label("%d/%d" % (self.current_page, total_page))
            
    def create_simple_button(self, name, callback, *args):        
        button = ImageButton(
            app_theme.get_pixbuf("filter/%s_normal.png" % name),
            app_theme.get_pixbuf("filter/%s_hover.png" % name),
            app_theme.get_pixbuf("filter/%s_press.png" % name)
            )
        if callback:
            button.connect("clicked", callback, *args)
        align = gtk.Alignment()    
        align.set(0.5, 0.5, 0, 0)
        align.add(button)
        return align 
    
    def create_block_box(self):
        box = gtk.EventBox()
        box.set_visible_window(False)
        box.set_size_request(-1, self.child_item_height)
        return box
            
class SongImportBar(BaseBar):            
    def __init__(self, title_name, callback):
        BaseBar.__init__(self, init_index=-1)
        self.child_box = gtk.VBox()
        title_item = SimpleItem(
            (app_theme.get_pixbuf("filter/import_normal.png"),
             app_theme.get_pixbuf("filter/import_press.png"),
             title_name, callback), 0, 10, 25, 20, 10, 25, self.set_index, self.get_index, ALIGN_START)
        self.pack_start(title_item, False, True)
        self.pack_start(self.child_box)
        
    def reload_items(self, child_items):    
        if child_items:
            container_remove_all(self.child_box)
            for index, item in enumerate(child_items):
                simple_item = SimpleItem(
                    (app_theme.get_pixbuf("filter/point_normal.png"),
                     app_theme.get_pixbuf("filter/point_press.png"),
                     item[0], item[1]),
                    index + 1, 8, 20, 35, 10, 25, self.set_index, self.get_index, ALIGN_START)
                self.child_box.pack_start(simple_item)
            self.show_all()        
            
    def set_index(self, index):    
        self.queue_draw()
        self.current_index = -1
            
                
class SimpleItem(gtk.Button):
    '''Simple item.'''
	
    def __init__(self, element, index, font_size, item_height,
                 padding_left, padding_middle, padding_right,
                 set_index, get_index, x_align=ALIGN_MIDDLE):
        
        # Init.
        super(SimpleItem, self).__init__()
        self.font_size = font_size
        self.index = index
        self.set_index = set_index
        self.get_index = get_index
        self.padding_left = padding_left
        self.padding_right = padding_right
        self.font_offset = padding_middle
        self.args = None
        if len(element) == 4:
            self.normal_dpixbuf, self.press_dpixbuf, self.content, self.clicked_callback = element
        else:    
            self.normal_dpixbuf, self.press_dpixbuf, self.content, self.clicked_callback = element[:4]
            self.args = element[4:]
            
        widget_fix_cycle_destroy_bug(self)
        pixbuf_width = self.normal_dpixbuf.get_pixbuf().get_width()
        self.font_offset += pixbuf_width
        self.x_align = x_align
        
        self.set_size_request(150, item_height)
        self.connect("expose-event", self.expose_simple_item)
        self.connect("clicked", lambda w: self.wrap_item_clicked_action())
        
    def wrap_item_clicked_action(self):   
        if self.clicked_callback:
            if self.args:
                self.clicked_callback(*self.args)
            else:    
                self.clicked_callback()
        self.set_index(self.index)    
        
    def expose_simple_item(self, widget, event):    
        
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        font_color = app_theme.get_color("labelText").get_color()
        item_pixbuf = self.normal_dpixbuf.get_pixbuf()
        select_index = self.get_index()
        
        if widget.state == gtk.STATE_NORMAL:
            if select_index == self.index:
                select_status = BUTTON_PRESS
            else:    
                select_status = BUTTON_NORMAL
                
        elif widget.state == gtk.STATE_PRELIGHT:        
            if select_index == self.index:
                select_status = BUTTON_PRESS
            else:    
                select_status = BUTTON_HOVER
                
        elif widget.state == gtk.STATE_ACTIVE:        
            select_status = BUTTON_PRESS
            
        if select_status == BUTTON_PRESS:    
            draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, 
                         app_theme.get_shadow_color("simpleItemPress").get_color_info())
            font_color = app_theme.get_color("simpleSelectItem").get_color()
            item_pixbuf = self.press_dpixbuf.get_pixbuf()
            
        elif select_status == BUTTON_HOVER:    
            draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, 
                         app_theme.get_shadow_color("simpleItemHover").get_color_info())
            
        # Draw pixbuf.    
        draw_pixbuf(cr, item_pixbuf, rect.x + self.padding_left, rect.y + (rect.height - item_pixbuf.get_height()) / 2)    
        
        
        # Draw content.
        draw_font(cr, self.content, self.font_size, font_color,
                  rect.x + self.padding_left + self.font_offset , 
                  rect.y,
                  rect.width - self.padding_left - self.font_offset - self.padding_right,
                  rect.height, x_align=self.x_align)
        
        propagate_expose(widget, event)
        
        return True
    
gobject.type_register(SimpleItem)    
