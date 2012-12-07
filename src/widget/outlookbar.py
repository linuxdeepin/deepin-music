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
from dtk.ui.draw import draw_vlinear, draw_pixbuf, draw_text
from dtk.ui.utils import propagate_expose, container_remove_all, get_content_size
from dtk.ui.constant import ALIGN_START, ALIGN_MIDDLE
import dtk.ui.tooltip as Tooltip

from widget.skin import app_theme
from widget.ui_utils import draw_single_mask

class BaseBar(gtk.VBox):
        
    def __init__(self, init_index=0):
        gtk.VBox.__init__(self)
        self.current_index = init_index
    
        
    def set_index(self, index):    
        self.current_index = index        
        self.queue_draw()
        
    def get_index(self):    
        return self.current_index
    
class OptionBar(BaseBar):    
    
    def __init__(self, items, init_index=0, font_size=10, padding_left=15, padding_middle=10, padding_right=20):
        BaseBar.__init__(self, init_index)
        
        if items:
            for index, item in enumerate(items):
                simple_item = SimpleItem(
                    item, index, font_size, 26, padding_left, padding_middle, padding_right, self.set_index, self.get_index)
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
    
    def __init__(self, title_name):
        BaseBar.__init__(self, init_index=-1)
        self.set_spacing(3)
        self.child_box = gtk.VBox()
        self.child_item_height = 22
        self.current_page = 1
        self.page_items_num = 0        
        self.items = []
        
        title_item = StaticLabel(
            app_theme.get_pixbuf("filter/local_normal.png"),
            title_name, 10, 25, 15, 10, 25, ALIGN_START)
        self.pack_start(title_item, False, False)
        self.pack_start(self.child_box, True, True)
        self.child_box.connect("size-allocate", self.size_change_cb)

        self.control_box = gtk.HBox()
        self.control_box.set_spacing(15)        
        previous_align = self.create_simple_button("previous", self.update_current_page, "previous")
        next_align = self.create_simple_button("next", self.update_current_page, "next")
        self.info_label = Label("0/0", app_theme.get_color("labelText"), text_x_align=ALIGN_MIDDLE)
        self.control_box.pack_start(previous_align, False, False)
        self.control_box.pack_start(self.info_label, False, False)
        self.control_box.pack_start(next_align, False, False)
        self.control_box.set_no_show_all(True)
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
                    index + 1,   9, self.child_item_height, 25, 10, 15, self.set_index, self.get_index, ALIGN_START)
                self.child_box.pack_start(simple_item, False, False)
                
            if len(child_items) < self.page_items_num:    
                block_num = self.page_items_num - len(child_items)
                for i in range(block_num):
                    self.child_box.pack_start(self.create_block_box(), False, True)
            self.control_box.set_no_show_all(False)        
            self.control_box.show_all()
            self.show_all()        
            self.queue_draw()
            self.get_toplevel().queue_draw()

            
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
        
    def adjust_current_page(self, total_page):    
        if self.current_page > total_page:
            self.current_page = total_page

    def update_label(self):    
        total_page = self.adjust_page()
        self.adjust_current_page(total_page)
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
    def __init__(self):
        BaseBar.__init__(self, init_index=-1)
        self.child_box = gtk.VBox()
        self.pack_start(self.child_box)
        
    def reload_items(self, child_items):    
        if child_items:
            container_remove_all(self.child_box)
            for index, item in enumerate(child_items):
                simple_item = SimpleItem(
                    (app_theme.get_pixbuf("filter/point_normal.png"),
                     app_theme.get_pixbuf("filter/point_press.png"),
                     item[0], item[1]),
                    index + 1, 9, 25, 15, 10, 10, self.set_index, self.get_index, ALIGN_START)
                self.child_box.pack_start(simple_item)
            self.show_all()        
            
    def set_index(self, index):    
        self.queue_draw()
        self.current_index = -1
            
                
class SimpleItem(gtk.Button):
    '''Simple item.'''
	
    def __init__(self, element, index, font_size, item_height,
                 padding_left, padding_middle, padding_right,
                 set_index, get_index, x_align=ALIGN_START):
        
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
            
        pixbuf_width = self.normal_dpixbuf.get_pixbuf().get_width()
        if self.content:
            Tooltip.text(self, self.content)
        self.font_offset += pixbuf_width
        self.x_align = x_align
        
        self.set_size_request(150, item_height)
        self.connect("expose-event", self.expose_simple_item)
        self.connect("clicked", lambda w: self.wrap_item_clicked_action())
        
    def wrap_item_clicked_action(self):   
        self.set_index(self.index)            
        if self.clicked_callback:
            if self.args:
                self.clicked_callback(*self.args)
            else:    
                self.clicked_callback()
        
    def expose_simple_item(self, widget, event):    
        
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        rect.x += 1
        rect.width -= 2
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
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
            # font_color = app_theme.get_color("simpleSelectItem").get_color()
            font_color = "#FFFFFF"
            item_pixbuf = self.press_dpixbuf.get_pixbuf()
            
        elif select_status == BUTTON_HOVER:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
            
        # Draw pixbuf.    
        draw_pixbuf(cr, item_pixbuf, rect.x + self.padding_left, rect.y + (rect.height - item_pixbuf.get_height()) / 2)    
        
        
        # Draw content.
        draw_text(cr, self.content, 
                  rect.x + self.padding_left + self.font_offset , 
                  rect.y,
                  rect.width - self.padding_left - self.font_offset - self.padding_right,
                  rect.height, 
                  self.font_size, font_color,
                  alignment=self.x_align)
        
        propagate_expose(widget, event)
        
        return True
    
gobject.type_register(SimpleItem)    


class StaticLabel(gtk.Button):
    '''Simple item.'''
	
    def __init__(self, normal_dpixbuf, content, font_size, item_height,
                 padding_left, padding_middle, padding_right,
                 x_align=ALIGN_MIDDLE):
        
        # Init.
        super(StaticLabel, self).__init__()
        self.font_size = font_size
        self.padding_left = padding_left
        self.padding_right = padding_right
        self.font_offset = padding_middle
        self.args = None
        self.normal_dpixbuf = normal_dpixbuf
        self.content = content
            
        pixbuf_width = self.normal_dpixbuf.get_pixbuf().get_width()
        self.font_offset += pixbuf_width
        self.x_align = x_align
        
        self.set_size_request(150, item_height)
        self.connect("expose-event", self.expose_simple_item)
        
    def expose_simple_item(self, widget, event):    
        
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        font_color = app_theme.get_color("labelText").get_color()
        item_pixbuf = self.normal_dpixbuf.get_pixbuf()
        # Draw pixbuf.    
        draw_pixbuf(cr, item_pixbuf, rect.x + self.padding_left, rect.y + (rect.height - item_pixbuf.get_height()) / 2)    
        
        
        # Draw content.
        draw_text(cr, self.content, 
                  rect.x + self.padding_left + self.font_offset , 
                  rect.y,
                  rect.width - self.padding_left - self.font_offset - self.padding_right,
                  rect.height, 
                  self.font_size, font_color,
                  alignment=self.x_align)
        
        propagate_expose(widget, event)
        
        return True
    
gobject.type_register(StaticLabel)    


        
class CategoryBar(BaseBar):            
    def __init__(self,   items, font_size=10, item_height=30, padding_left=20, 
                 padding_middle=10, padding_right=25, x_align=ALIGN_MIDDLE):
        BaseBar.__init__(self, init_index=0)
        for index, item in enumerate(items):
            category_item = CategoryItem(
                (item[0], item[1]), index , font_size, item_height, 
                padding_left, padding_middle, padding_right, self.set_index, self.get_index, x_align)
            self.pack_start(category_item, False, False)
            if category_item.get_child_item():
                self.pack_start(category_item.get_child_item(), False, False)

gobject.type_register(CategoryBar)                
                
                
class CategoryItem(gtk.Button):
    '''Simple item.'''
	
    def __init__(self, element, index, font_size, item_height, 
                 padding_left, padding_middle, padding_right,
                 set_index, get_index, x_align=ALIGN_MIDDLE):
        
        # Init.
        super(CategoryItem, self).__init__()
        self.font_size = font_size
        self.index = index
        self.set_index = set_index
        self.get_index = get_index
        self.padding_left = padding_left
        self.padding_right = padding_right
        self.args = None
        self.child_category = None
        self.child_status = False
        self.clicked_callback = None
        
        if len(element) == 2:
            self.content, self.node = element
        else:    
            self.content, self.node = element[:2]
            self.args = element[2:]
            
        self.arrow_dpixbuf = app_theme.get_pixbuf("preference/arrow_right.png")
        self.arrow_width = self.arrow_dpixbuf.get_pixbuf().get_width()
        content_width, _height = get_content_size(self.content, font_size)
        self.icon_offset = content_width + padding_middle
        self.x_align = x_align
        
        if isinstance(self.node, CategoryBar):
            self.child_category = self.node
            self.change_child_status(True)
            self.connect("clicked", self.wrap_node_show_action)
        else:    
            self.clicked_callback = self.node
            self.connect("clicked", self.wrap_item_clicked_action)           
        
        self.set_size_request(150, item_height)
        self.connect("expose-event", self.expose_category_item)
        
    def wrap_node_show_action(self, widget):    
        self.change_child_status(self.child_status)
        self.child_status = not self.child_status
        
    def change_child_status(self, hide=False):    
        if not hide:
            self.child_category.set_no_show_all(False)
            self.child_category.show_all()
        else:    
            self.child_category.hide_all()
            self.child_category.set_no_show_all(True)
        
    def wrap_item_clicked_action(self, widget):   
        if self.clicked_callback:
            if self.args:
                self.clicked_callback(*self.args)
            else:    
                self.clicked_callback()
        self.set_index(self.index)    
        
    def get_child_item(self):    
        return self.child_category
        
    def expose_category_item(self, widget, event):    
        
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        font_color = app_theme.get_color("labelText").get_color()
        arrow_pixbuf = self.arrow_dpixbuf.get_pixbuf()
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
            
        elif select_status == BUTTON_HOVER:    
            draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, 
                         app_theme.get_shadow_color("simpleItemHover").get_color_info())
            
        
        # Draw content.
        draw_text(cr, self.content, 
                  rect.x + self.padding_left, 
                  rect.y,
                  rect.width - self.padding_left - self.arrow_width - self.padding_right,
                  rect.height, 
                  self.font_size, font_color,
                  alignment=self.x_align)
        
        # Draw pixbuf.    
        draw_pixbuf(cr, arrow_pixbuf, rect.x + rect.width - self.arrow_width - self.padding_right ,rect.y + (rect.height - arrow_pixbuf.get_height()) / 2)    
        propagate_expose(widget, event)
        
        return True
    
gobject.type_register(CategoryItem)    
                


class SimpleLabel(gtk.Button):
    '''Simple item.'''
	
    def __init__(self, element, index, font_size, item_height,
                 padding_left, padding_middle, padding_right,
                 set_index, get_index, x_align=ALIGN_START):
        
        # Init.
        super(SimpleLabel, self).__init__()
        self.font_size = font_size
        self.index = index
        self.set_index = set_index
        self.get_index = get_index
        self.padding_left = padding_left
        self.padding_right = padding_right
        self.font_offset = padding_middle
        self.args = None
        if len(element) == 2:
            self.content, self.clicked_callback = element
        else:    
            self.content, self.clicked_callback = element[:2]
            self.args = element[2:]

        if self.content:
            Tooltip.text(self, self.content)
        self.x_align = x_align
        self.set_size_request(120, item_height)
        self.connect("expose-event", self.expose_simple_item)
        self.connect("clicked", lambda w: self.wrap_item_clicked_action())
        
    def wrap_item_clicked_action(self):   
        self.set_index(self.index)            
        if self.clicked_callback:
            if self.args:
                self.clicked_callback(*self.args)
            else:    
                self.clicked_callback()
        
    def expose_simple_item(self, widget, event):    
        
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        font_color = app_theme.get_color("labelText").get_color()
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
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "simpleItemHighlight")
            font_color = app_theme.get_color("simpleSelectItem").get_color()
            
        elif select_status == BUTTON_HOVER:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "simpleItemHover")
        
        
        # Draw content.
        draw_text(cr, self.content, 
                  rect.x + self.padding_left + self.font_offset , 
                  rect.y,
                  rect.width - self.padding_left - self.font_offset - self.padding_right,
                  rect.height, 
                  self.font_size, font_color,
                  alignment=self.x_align)
        
        propagate_expose(widget, event)
        
        return True
    
gobject.type_register(SimpleLabel)    


class WebcastsBar(BaseBar):    
    
    def __init__(self, items, init_index=0, font_size=10, padding_left=15, padding_middle=10, padding_right=20):
        BaseBar.__init__(self, init_index)
        
        if items:
            for index, item in enumerate(items):
                simple_item = SimpleLabel(
                    item, index, font_size, 32, padding_left, padding_middle, padding_right, self.set_index, self.get_index)
                self.pack_start(simple_item, False, True)
        self.show_all()        
