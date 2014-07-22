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
import pango
import gobject
from dtk.ui.menu import Menu
from dtk.ui.draw import draw_pixbuf
from dtk.ui.label import Label
from dtk.ui.utils import propagate_expose, get_content_size
from dtk.ui.constant import BUTTON_PRESS, BUTTON_NORMAL, BUTTON_HOVER
from dtk.ui.cache_pixbuf import CachePixbuf
import dtk.ui.tooltip as Tooltip

from widget.skin import app_theme
from widget.ui_utils import render_text
from nls import _
    

class ComboItem(gtk.Button):
    def __init__(self, bg_image_group, icon_group, index, set_index, get_index):
        gtk.Button.__init__(self)
        
        # Init.
        self.index = index
        self.set_index = set_index
        self.get_index = get_index
        self.icon_group = icon_group
        self.bg_image_group = bg_image_group
        self.resize_button()
        
        # connect
        self.connect("clicked", self.update_button_index)
        self.connect("expose-event", self.expose_button_cb)
        
    def expose_button_cb(self, widget, event):    
        # Init.
        rect = widget.allocation
        
        bg_normal_dpixbuf, bg_hover_dpixbuf, bg_press_dpixbuf = self.bg_image_group
        fg_normal_dpixbuf, fg_hover_dpixbuf, fg_press_dpixbuf = self.icon_group
        select_index = self.get_index()

        bg_image = bg_normal_dpixbuf.get_pixbuf()
        fg_image = fg_normal_dpixbuf.get_pixbuf()
        
        
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
        
        
        if select_status == BUTTON_NORMAL:
            bg_image = bg_normal_dpixbuf.get_pixbuf()
            fg_image = fg_normal_dpixbuf.get_pixbuf()
        elif  select_status == BUTTON_HOVER:
            bg_image = bg_hover_dpixbuf.get_pixbuf()
            fg_image = fg_hover_dpixbuf.get_pixbuf()
        elif select_status == BUTTON_PRESS:
            bg_image = bg_press_dpixbuf.get_pixbuf()
            fg_image = fg_press_dpixbuf.get_pixbuf()
            
        image_width = bg_image.get_width()    
        image_height = bg_image.get_height()
        
        fg_rect_x  = rect.x + (image_width - fg_image.get_width()) / 2
        fg_rect_y  = rect.y + (image_height - fg_image.get_height()) / 2
        
        cr = widget.window.cairo_create()
        draw_pixbuf(cr, bg_image, rect.x, rect.y)
        draw_pixbuf(cr, fg_image, fg_rect_x, fg_rect_y)
        
        propagate_expose(widget, event)
        
        return True
        
    def resize_button(self):
        normal_dpixbuf = self.bg_image_group[0]
        request_width  = normal_dpixbuf.get_pixbuf().get_width()
        request_height = normal_dpixbuf.get_pixbuf().get_height()
        self.set_size_request(request_width, request_height)
        
    def update_icon_group(self, new_group):    
        self.icon_group = new_group
        
    def update_button_index(self, widget):
        self.set_index(self.index)
    
class ComboButton(gtk.Button):
    def __init__(self, bg_image_group, icon_group):
        gtk.Button.__init__(self)
        
        # Init.
        self.icon_group = icon_group
        self.bg_image_group = bg_image_group
        self.resize_button()
        
        # connect
        self.connect("expose-event", self.expose_button_cb)
        
    def expose_button_cb(self, widget, event):    
        # Init.
        rect = widget.allocation
        
        bg_normal_dpixbuf, bg_hover_dpixbuf, bg_press_dpixbuf = self.bg_image_group
        fg_normal_dpixbuf, fg_hover_dpixbuf, fg_press_dpixbuf = self.icon_group
        
        if widget.state == gtk.STATE_NORMAL:
            bg_image = bg_normal_dpixbuf.get_pixbuf()
            fg_image = fg_normal_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_PRELIGHT:
            bg_image = bg_hover_dpixbuf.get_pixbuf()
            fg_image = fg_hover_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_ACTIVE:
            bg_image = bg_press_dpixbuf.get_pixbuf()
            fg_image = fg_press_dpixbuf.get_pixbuf()
            
        image_width = bg_image.get_width()    
        image_height = bg_image.get_height()
        
        fg_rect_x  = rect.x + (image_width - fg_image.get_width()) / 2
        fg_rect_y  = rect.y + (image_height - fg_image.get_height()) / 2
        
        cr = widget.window.cairo_create()
        draw_pixbuf(cr, bg_image, rect.x, rect.y)
        draw_pixbuf(cr, fg_image, fg_rect_x, fg_rect_y)
        
        propagate_expose(widget, event)
        
        return True
        
    def resize_button(self):
        normal_dpixbuf = self.bg_image_group[0]
        request_width  = normal_dpixbuf.get_pixbuf().get_width()
        request_height = normal_dpixbuf.get_pixbuf().get_height()
        self.set_size_request(request_width, request_height)
        
    def update_icon_group(self, new_group):    
        self.icon_group = new_group
        
    
        
class ComboMenuButton(gtk.HBox):    
    __gsignals__ = {
        "list-actived" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "combo-actived" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING,))
        }
    
    def __init__(self, init_index=0):
        super(ComboMenuButton, self).__init__()
        self.current_index = init_index
        self.current_status = "artist"
        self.set_spacing(0)
        self.msg_content = _("By Artist")
        
        self.list_button = ComboItem(
            (app_theme.get_pixbuf("combo/left_normal.png"),
             app_theme.get_pixbuf("combo/left_hover.png"),
             app_theme.get_pixbuf("combo/left_press.png")),
            (app_theme.get_pixbuf("combo/list_normal.png"),
             app_theme.get_pixbuf("combo/list_normal.png"),
             app_theme.get_pixbuf("combo/list_press.png")
             ), 0, self.set_index, self.get_index)
        
        Tooltip.text(self.list_button, _("List view"))
        
        # draw left_button.
        self.left_button = gtk.Button()
        self.left_button = ComboItem(
            (app_theme.get_pixbuf("combo/left_normal.png"),
             app_theme.get_pixbuf("combo/left_hover.png"),
             app_theme.get_pixbuf("combo/left_press.png")),
            (app_theme.get_pixbuf("combo/artist_normal.png"),
             app_theme.get_pixbuf("combo/artist_normal.png"),
             app_theme.get_pixbuf("combo/artist_press.png")
             ), 1, self.set_index, self.get_index)
        Tooltip.custom(self.left_button, self.get_msg_label).always_update(self.left_button, True)
        
        # draw right_button.
        self.right_button = ComboButton( 
            (app_theme.get_pixbuf("combo/right_normal.png"),
             app_theme.get_pixbuf("combo/right_hover.png"),
             app_theme.get_pixbuf("combo/right_hover.png")),
            (app_theme.get_pixbuf("combo/triangle_normal.png"),
             app_theme.get_pixbuf("combo/triangle_normal.png"),
             app_theme.get_pixbuf("combo/triangle_press.png")
             ))
        
        # signals.
        self.left_button.connect("clicked", lambda w: self.emit_combo_signal())
        self.right_button.connect("button-press-event", self.show_right_menu)
        self.list_button.connect("clicked", lambda w: self.emit_list_signal())
        
        # pack
        combo_box = gtk.HBox()
        combo_box.pack_start(self.left_button)
        combo_box.pack_start(self.right_button)
        self.pack_start(self.list_button)
        self.pack_start(combo_box)
    
    def show_right_menu(self, widget, event):
        menu_items = [
            (self.get_menu_pixbuf_group("artist"), _("By Artist"), self.update_widget_icon, "artist", _("By Artist")),
            (self.get_menu_pixbuf_group("genre"), _("By Genre"), self.update_widget_icon, "genre", _("By Genre")),
            (self.get_menu_pixbuf_group("album"), _("By Album"), self.update_widget_icon, "album", _("By Album")),
            ]
        Menu(menu_items, True).show((int(event.x_root) - 10, int(event.y_root)))
        
    def get_menu_pixbuf_group(self, name):    
        return (app_theme.get_pixbuf("combo/%s_press.png" % name), app_theme.get_pixbuf("combo/%s_hover.png" % name))
        
    def update_widget_icon(self, name, tip_msg):    
        self.left_button.update_icon_group((
                app_theme.get_pixbuf("combo/%s_normal.png" % name),
                app_theme.get_pixbuf("combo/%s_normal.png" % name),
                app_theme.get_pixbuf("combo/%s_press.png" % name)
                ))
        self.set_index(1)
        self.current_status = name
        self.msg_content = tip_msg
        self.emit_combo_signal()
        
    def get_msg_label(self):    
        return Label(self.msg_content)
        
    def set_index(self, index):    
        self.current_index = index
        self.queue_draw()
        
    def get_index(self):    
        return self.current_index
    
    def emit_combo_signal(self):
        self.emit("combo-actived", self.current_status)
    
    def emit_list_signal(self):    
        self.emit("list-actived")
        
    def get_combo_active(self):    
        return self.current_index == 1

class PromptButton(gtk.Button):    
    
    def __init__(self, pixbuf=None, text=None, max_width=360):
        
        # Init.
        gtk.Button.__init__(self)
        
        self.padding_x = 5
        self.max_width = max_width
        self.prompt_pixbuf = pixbuf
        self.prompt_text = text
        self.font_size = 9
        self.widget_h = 26
        
        self.bg_left = app_theme.get_pixbuf("combo/prompt_left.png").get_pixbuf()
        self.bg_middle = app_theme.get_pixbuf("combo/prompt_middle.png").get_pixbuf()
        self.bg_right = app_theme.get_pixbuf("combo/prompt_right.png").get_pixbuf()
        self.cache_bg_pixbuf = CachePixbuf()
        self.update_size()
        self.connect("expose-event", self.on_expose_event)
        
    def update_size(self):    
        if not self.prompt_pixbuf: return
        pixbuf_w = self.prompt_pixbuf.get_width()
        text_w, text_h = get_content_size(self.prompt_text, self.font_size)
        widget_w  = pixbuf_w + text_w + self.padding_x * 3
        if widget_w > self.max_width:
            widget_w = self.max_width
        self.set_size_request(widget_w, self.widget_h)
        self.queue_draw()
        
    def on_expose_event(self, widget, event):    
        if not self.prompt_pixbuf: return
        cr = widget.window.cairo_create()
        rect = widget.allocation
        pixbuf_y = rect.y + (rect.height - self.prompt_pixbuf.get_height()) / 2
        draw_pixbuf(cr, self.bg_left, rect.x, rect.y)
        bg_left_w = self.bg_left.get_width()
        self.cache_bg_pixbuf.scale(self.bg_middle,  rect.width - bg_left_w * 2, self.bg_middle.get_height())
        draw_pixbuf(cr, self.cache_bg_pixbuf.get_cache(), rect.x + bg_left_w, rect.y)
        draw_pixbuf(cr, self.bg_right, rect.x + rect.width - bg_left_w, rect.y)
        
        draw_pixbuf(cr, self.prompt_pixbuf, rect.x + self.padding_x, pixbuf_y)
        
        # draw text.
        text_rect = gtk.gdk.Rectangle(rect.x + self.prompt_pixbuf.get_width() +  self.padding_x * 2, rect.y, 
                                      rect.width - self.prompt_pixbuf.get_width() - self.padding_x * 3,
                                      rect.height)
        render_text(cr, self.prompt_text, text_rect,
                    app_theme.get_color("labelText").get_color(),
                    8)
        
        return True
    
    def set_infos(self, infos):
        temp_pixbuf, self.prompt_text = infos
        if temp_pixbuf:
            Tooltip.text(self, self.prompt_text)
            self.prompt_pixbuf = temp_pixbuf.scale_simple(16, 16, gtk.gdk.INTERP_BILINEAR)
            self.update_size()

gobject.type_register(PromptButton)


class TextPrompt(gtk.Button):    
    
    def __init__(self, text="", max_width=135):
        
        # Init.
        gtk.Button.__init__(self)
        
        self.padding_x = 5
        self.max_width = max_width
        self.prompt_text = text
        self.font_size = 9
        self.widget_h = 26
        
        self.bg_left = app_theme.get_pixbuf("combo/prompt_left.png").get_pixbuf()
        self.bg_middle = app_theme.get_pixbuf("combo/prompt_middle.png").get_pixbuf()
        self.bg_right = app_theme.get_pixbuf("combo/prompt_right.png").get_pixbuf()
        self.cache_bg_pixbuf = CachePixbuf()
        self.update_size()
        self.connect("expose-event", self.on_expose_event)
        
    def update_size(self):    
        text_w, text_h = get_content_size(self.prompt_text, self.font_size)
        widget_w  = text_w + self.padding_x * 2
        if widget_w > self.max_width:
            widget_w = self.max_width
        self.set_size_request(widget_w, self.widget_h)
        self.queue_draw()
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_pixbuf(cr, self.bg_left, rect.x, rect.y)
        bg_left_w = self.bg_left.get_width()
        self.cache_bg_pixbuf.scale(self.bg_middle,  rect.width - bg_left_w * 2, self.bg_middle.get_height())
        draw_pixbuf(cr, self.cache_bg_pixbuf.get_cache(), rect.x + bg_left_w, rect.y)
        draw_pixbuf(cr, self.bg_right, rect.x + rect.width - bg_left_w, rect.y)
        
        # draw text.
        text_rect = gtk.gdk.Rectangle(rect.x + self.padding_x, rect.y, 
                                      rect.width  - self.padding_x * 2,
                                      rect.height)
        render_text(cr, self.prompt_text, text_rect,
                    app_theme.get_color("labelText").get_color(),
                    8)
        
        return True
    
    def set_text(self, text):
        self.prompt_text = text
        Tooltip.text(self, self.prompt_text)
        self.update_size()

gobject.type_register(PromptButton)
