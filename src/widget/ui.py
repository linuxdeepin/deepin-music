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
import gobject
import cairo

from dtk.ui.window import Window
from dtk.ui.titlebar import Titlebar
from dtk.ui.utils import (move_window, alpha_color_hex_to_cairo, 
                          color_hex_to_cairo, cairo_disable_antialias,
                          propagate_expose, get_content_size)
from dtk.ui.new_entry import InputEntry, Entry
from dtk.ui.button import ImageButton
from dtk.ui.draw import draw_pixbuf, draw_text, draw_round_rectangle, draw_vlinear
from widget.skin import app_theme
from widget.ui_utils import draw_alpha_mask, draw_line

from helper import Dispatcher
from constant import EMPTY_WEBCAST_ITEM, EMPTY_RADIO_ITEM

import utils

class NormalWindow(Window):
    
    def __init__(self):
        super(NormalWindow, self).__init__(True)
        
        # Init Window
        self.set_position(gtk.WIN_POS_CENTER)
        self.titlebar = Titlebar(["close"])
        self.titlebar.close_button.connect_after("clicked", self.hide_window)
        self.titlebar.drag_box.connect('button-press-event', lambda w, e: move_window(w, e, self.window))
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(5, 10, 10, 10)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)
        self.window_frame.pack_start(self.titlebar, False, False)
        self.window_frame.pack_start(main_align, True, True)
        
    def show_window(self):    
        self.show_all()
            
    def hide_window(self, widget):        
        self.hide_all()
        

        
class SearchEntry(InputEntry):
    
    def __init__(self, *args, **kwargs):


        entry_button = ImageButton(
            app_theme.get_pixbuf("toolbar/search_normal.png"),
            app_theme.get_pixbuf("toolbar/search_hover.png"),
            app_theme.get_pixbuf("toolbar/search_press.png")
            )
        super(SearchEntry, self).__init__(action_button=entry_button, *args, **kwargs)        
        
        self.action_button = entry_button
        self.set_size(250, 24)
        
gobject.type_register(SearchEntry)        

        
class ComplexButton(gtk.Button):    
    
    def __init__(self, bg_group, icon, content, left_padding=20, label_padding=10, font_size=9):
        super(ComplexButton, self).__init__()
        
        # Init.
        self.normal_bg, self.hover_bg, self.press_bg = bg_group
        self.button_icon = icon
        self.content = content
        self.font_size = font_size
        self.left_padding = left_padding
        self.label_padding = label_padding
        
        # Set size.
        self.set_button_size()
        self.connect("expose-event", self.expose_button)
        
    def set_button_size(self):    
        request_width  = self.normal_bg.get_pixbuf().get_width()
        request_height = self.normal_bg.get_pixbuf().get_height()
        self.set_size_request(request_width, request_height)
        
    def expose_button(self, widget, event):    
        
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        if widget.state == gtk.STATE_NORMAL:
            bg_pixbuf = self.normal_bg.get_pixbuf()
        elif widget.state == gtk.STATE_PRELIGHT:    
            bg_pixbuf = self.hover_bg.get_pixbuf()
        elif widget.state == gtk.STATE_ACTIVE:    
            bg_pixbuf = self.press_bg.get_pixbuf()
            
        icon_pixbuf = self.button_icon.get_pixbuf()    
            
        icon_y = rect.y + (rect.height - icon_pixbuf.get_height()) / 2    
        
        # Draw bg.
        draw_pixbuf(cr, bg_pixbuf, rect.x, rect.y)
        
        # Draw icon.
        draw_pixbuf(cr, icon_pixbuf, rect.x + self.left_padding, icon_y)
        
        # Draw label.
        draw_text(cr, utils.xmlescape(self.content), rect.x + self.left_padding + self.label_padding + icon_pixbuf.get_width(),
                  rect.y, rect.width - self.left_padding - self.label_padding - icon_pixbuf.get_width(), rect.height,
                  self.font_size, text_color="#FFFFFF")
        
        return True

gobject.type_register(ComplexButton)    

class SearchButton(gtk.Button):
    
    def __init__(self):
        gtk.Button.__init__(self)
        # Init signals.
        self.connect("expose-event", self.on_expose_event)
        
        self.bg_pixbuf = app_theme.get_pixbuf("search/bg.png").get_pixbuf()

        # Init DPixbufs.
        self.normal_dpixbuf = app_theme.get_pixbuf("search/search_normal.png")
        self.hover_dpixbuf = app_theme.get_pixbuf("search/search_hover.png")
        self.press_dpixbuf = app_theme.get_pixbuf("search/search_press.png")
        
        self.set_size_request(self.bg_pixbuf.get_width(), self.bg_pixbuf.get_height())
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        # Draw Background.
        draw_pixbuf(cr, self.bg_pixbuf, rect.x, rect.y)
        # cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        # cr.set_source_rgb(*color_hex_to_cairo("#D7D7D7"))
        # cr.fill()
        
        pixbuf  = None
        if widget.state == gtk.STATE_NORMAL:
            pixbuf = self.normal_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_PRELIGHT:    
            pixbuf = self.hover_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_ACTIVE:    
            pixbuf = self.press_dpixbuf.get_pixbuf()
            
        if pixbuf is None:    
            pixbuf = self.normal_dpixbuf.get_pixbuf()
            
        icon_x = rect.x + (rect.width - pixbuf.get_width()) / 2
        icon_y = rect.y + (rect.height - pixbuf.get_height()) / 2
        draw_pixbuf(cr, pixbuf, icon_x, icon_y)    
        return True

class CustomEntry(gtk.VBox):
    __gsignals__ = {
        
        "action-active" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str,)),
    }
    
    def __init__(self,  content=""):
        '''
        Initialize InputEntry class.
        '''
        # Init.
        gtk.VBox.__init__(self)
        self.entry = Entry(content)
        entry_align = gtk.Alignment()
        entry_align.set(0.5, 0.5, 1, 1)
        entry_align.add(self.entry)
        self.add(entry_align)
        self.connect("expose-event", self.expose_input_entry)
            
    def set_sensitive(self, sensitive):
        '''
        Internal function to wrap function `set_sensitive`.
        '''
        super(InputEntry, self).set_sensitive(sensitive)
        self.entry.set_sensitive(sensitive)
            
    def emit_action_active_signal(self):
        '''
        Internal callback for `action-active` signal.
        '''
        self.emit("action-active", self.get_text())                
        
    def expose_input_entry(self, widget, event):
        '''
        Internal callback for `expose-event` signal.
        '''
        # Init.
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        cr.set_source_rgba(*alpha_color_hex_to_cairo(("#FFFFFF", 0.9)))
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
        # Draw frame.
        with cairo_disable_antialias(cr):
            cr.set_line_width(1)
            cr.set_source_rgba(*color_hex_to_cairo("#C3C4C5"))
            cr.move_to(rect.x + rect.width, rect.y)
            cr.rel_line_to(0, rect.height)
            cr.stroke()
        
        propagate_expose(widget, event)
        
        # return True
    
    def set_size(self, width, height):
        '''
        Set input entry size with given value.
        
        @param width: New width of input entry.
        @param height: New height of input entry.
        '''
        # self.set_size_request(width, height)    
        self.entry.set_size_request(width - 2, height - 2)
        
    def set_editable(self, editable):
        '''
        Set editable status of input entry.
        
        @param editable: input entry can editable if option is True, else can't edit.
        '''
        self.entry.set_editable(editable)
        
    def set_text(self, text):
        '''
        Set text of input entry.
        
        @param text: input entry string.
        '''
        self.entry.set_text(text)
        
    def get_text(self):
        '''
        Get text of input entry.
        
        @return: Return text of input entry.
        '''
        return self.entry.get_text()
    
    def focus_input(self):
        '''
        Focus input cursor.
        '''
        self.entry.grab_focus()
        
gobject.type_register(CustomEntry)


class EmptyListItem(gtk.EventBox):
    
    def __init__(self, drag_data_received_cb, item_type):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        self.connect("expose-event", self.on_expose_event)
        
        if item_type == EMPTY_WEBCAST_ITEM:
            targets = [("text/deepin-webcasts", gtk.TARGET_SAME_APP, 1),]
            self.empty_pixbuf = app_theme.get_pixbuf("webcast/empty_cn.png").get_pixbuf()            
        elif item_type == EMPTY_RADIO_ITEM:    
            targets = [("text/deepin-radios", gtk.TARGET_SAME_APP, 1),]
            self.empty_pixbuf = app_theme.get_pixbuf("webcast/empty_cn.png").get_pixbuf()            
            
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP,
                           targets, gtk.gdk.ACTION_COPY)
        self.connect("drag-data-received", drag_data_received_cb)
        

        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutLeft")
        
        icon_x = rect.x + (rect.width - self.empty_pixbuf.get_width()) / 2 
        icon_y = rect.y + (rect.height - self.empty_pixbuf.get_height()) / 2 - 50
        
        draw_pixbuf(cr, self.empty_pixbuf, icon_x, icon_y)
        
        return True

    

class WaitBox(gtk.EventBox):
    
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        self.max_number = 10
        self.item_width = self.item_height = 20
        self.padding_x = 8
        self.padding_y = 5
        self.auto_animiation_time = 500
        self.active_color = "#33CCFF"
        self.inactive_color = "#999999"        
        self.active_color_info = [(0.2, (self.active_color, 0.8)), 
                                  (0.8, (self.active_color, 0.95)), (1.0, (self.active_color, 0.8))]

        self.inactive_color_info = [(0.2, (self.inactive_color, 0.9)), 
                                  (0.8, (self.inactive_color, 0.95)), (1.0, (self.inactive_color, 0.9))]
        self.active_number = 1
        
        self.__init_size()
        self.connect("expose-event", self.on_expose_event)
        
        gobject.timeout_add(self.auto_animiation_time, self.loop_animiation)
        
    def __init_size(self):    
        width = self.max_number * self.item_width  + self.padding_x * (self.max_number + 1)
        height = self.item_height + self.padding_y * 2
        self.set_size_request(width, height)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        item_x = rect.x + self.padding_x
        item_y = rect.y + self.padding_y
        
        for i in range(1, self.max_number + 1):

            if i <= self.active_number:
                color_info = self.active_color_info
            else:    
                color_info = self.inactive_color_info
            cr.save()                
            draw_vlinear(cr, item_x, item_y, self.item_width, self.item_height, color_info, 3)                                
            cr.restore()
            item_x += self.padding_x + self.item_width
        
            
    def loop_animiation(self):        
        self.active_number += 1
        if self.active_number > self.max_number:
            self.active_number = 1
        self.queue_draw()    
        
        return True
            
        
class WaitProgress(gtk.EventBox):
    
    def __init__(self):
        
        gtk.EventBox.__init__(self)
        self.connect("expose-event", self.on_expose_event)
        self.padding_x = 10
        self.padding_y = 5        
        self.default_height = 22
        self.item_width = 18
        self.item_height = 22
        self.max_number = 18
        self.move_x = 0
        
        self.init_size()
        
        gobject.timeout_add(500, self.loop_animiation)
        
    def init_size(self):
        width = self.max_number * self.item_width + self.padding_x * 2
        height = self.item_height * self.padding_y * 2
        self.set_size_request(width, height)
        
    def on_expose_event(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        # Draw left arc.
        cr.save()
        cr.translate(self.padding_x, self.padding_y)
        cr.rectangle(rect.x, rect.y, rect.width - self.padding_x * 2, rect.height - self.padding_y * 2)
        cr.set_line_width(1)
        cr.set_source_rgb(*color_hex_to_cairo("#999999"))
        cr.stroke()
        cr.restore()    
        
        cr.save()    
        for  i in range(1, self.max_number + 1):
            if i % 2 == 0:
                first_upper_x = self.move_x + rect.x + self.padding_y + self.item_width * i
                cr.move_to(first_upper_x, rect.y + self.padding_y)
                cr.rel_line_to(-self.item_width, self.item_height)
                cr.rel_line_to(self.item_width, 0)
                cr.rel_line_to(self.item_width, -self.item_height)
                cr.set_source_rgb(*color_hex_to_cairo("#3E89CF"))
                cr.fill()
            else:
                second_upper_x = self.move_x + rect.x + self.padding_y + self.item_width * i
                cr.move_to(second_upper_x, rect.y + self.padding_y)
                cr.rel_line_to(-self.item_width, self.item_height)
                cr.rel_line_to(self.item_width, 0)
                cr.rel_line_to(self.item_width, -self.item_height)
                cr.set_source_rgb(*color_hex_to_cairo("#FFFFFF"))
                cr.fill()
        cr.restore()
        return True
    
    def loop_animiation(self):
        pass
    
    
class CoverPopupNotify(Window):    
    
    def __init__(self, channel_info, default_width=300):
        Window.__init__(self, 
                        shadow_visible=False,
                        shadow_radius=0,
                        shape_frame_function=self.shape_panel_frame,
                        expose_frame_function=self.expose_panel_frame,
                        window_type=gtk.WINDOW_POPUP)
        
        self.set_can_focus(False)
        self.set_accept_focus(False)
        self.info_panel = gtk.EventBox()
        self.info_panel.set_visible_window(False)
        self.info_panel.connect("expose-event", self.on_panel_expose_event)
        self.connect("enter-notify-event", self.on_enter_notify_event)
        self.window_frame.add(self.info_panel)
        
        self.channel_info = channel_info
        self.padding_y = 15
        self.padding_x = 10
        self.line_height = 15
        self.default_width = default_width
        
        self.init_size()
        
    def on_enter_notify_event(self, widget, event):    
        self.hide_all()
        
    def init_size(self):    
        tw = self.default_width - self.padding_x * 2
        intro = self.channel_info.get("intro", "").strip()
        intro = utils.xmlescape(intro)
        intro_text = "<b>%s:</b> %s" % ("简介", intro)
        intro_th = 0
        if intro:
            intro_tw, intro_th = get_content_size(intro_text, wrap_width=tw,text_size=9)
            if intro_th > self.line_height * 2:
                intro_th = self.line_height * 2
                
        hotsongs =  " / ".join(self.channel_info.get("hot_songs", [])).strip()
        hotsongs = utils.xmlescape(hotsongs)
        hotsongs_text = "<b>%s:</b> %s" % ("热门歌曲", hotsongs)
        hotsongs_tw, hotsongs_th = get_content_size(hotsongs_text, wrap_width=tw,text_size=9)
        if hotsongs_th > self.line_height * 2:
            hotsongs_th = self.line_height * 2
        
        if intro_th == 0:    
            height = hotsongs_th + self.line_height * 2
        else:    
            height = hotsongs_th + intro_th + self.line_height * 4
        self.set_size_request(self.default_width, height)    
        
    def shape_panel_frame(self, widget, event):    
        pass
        
    def expose_panel_frame(self, widget, event):
        cr  = widget.window.cairo_create()
        rect = widget.allocation
        cr.set_source_rgb(1,1,1)
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
    def on_panel_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        draw_line(cr, (rect.x, rect.y + 1), (rect.x + rect.width, rect.y + 1), "#c7c7c7")
        draw_line(cr, (rect.x + rect.width, rect.y), (rect.x + rect.width, rect.y + rect.height), "#c7c7c7")
        draw_line(cr, (rect.x, rect.y + rect.height), (rect.x + rect.width, rect.y + rect.height), "#c7c7c7")
        draw_line(cr, (rect.x + 1, rect.y), (rect.x + 1, rect.y + rect.height), "#c7c7c7")
        
        tx = rect.x + self.padding_x        
        ty = rect.y + self.padding_y
        tw = rect.width - self.padding_x * 2
        intro = self.channel_info.get("intro", "").strip()
        intro = utils.xmlescape(intro)
        if intro:
            intro_text = "<b>%s:</b> %s" % ("简介", intro)
            intro_tw, intro_th = get_content_size(intro_text, wrap_width=tw,text_size=9)
            if intro_th > self.line_height * 2:
                intro_th = self.line_height * 2
            cr.save()
            cr.rectangle(tx, ty, intro_tw, intro_th)
            cr.clip()
            draw_text(cr, intro_text, tx, ty, intro_tw, intro_th, text_size=9,
                      text_color="#878787", wrap_width=tw)
            cr.restore()

            with cairo_disable_antialias(cr):
                cr.save()
                cr.move_to(tx, ty + intro_th + self.line_height)
                cr.rel_line_to(tw, 0)
                cr.set_dash([2.0, 2.0])
                cr.stroke()
                cr.restore()
            
        hotsongs =  " / ".join(self.channel_info.get("hot_songs", [])).strip()
        hotsongs = utils.xmlescape(hotsongs)
        hotsongs_text = "<b>%s:</b> %s" % ("热门歌曲", hotsongs)
        if intro:
            new_ty = ty + intro_th + self.line_height * 2
        else:    
            new_ty = ty
            
        hotsongs_tw, hotsongs_th = get_content_size(hotsongs_text, wrap_width=tw,text_size=9)
        if hotsongs_th > self.line_height * 2:
            hotsongs_th = self.line_height * 2
        cr.save()
        cr.rectangle(tx, new_ty, hotsongs_tw, hotsongs_th)
        cr.clip()
        draw_text(cr, hotsongs_text, tx, new_ty, hotsongs_tw, hotsongs_th, text_size=9,
                  text_color="#878787", wrap_width=tw)
        return True
    
    def show(self, x, y):
        self.move(x, y)
        self.show_all()
        
class SearchBox(gtk.HBox):        
    
    __gsignals__ = {"search" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))}
    
    def __init__(self):
        gtk.HBox.__init__(self)
        self.entry_box = CustomEntry()
        self.search_button = SearchButton()
        self.search_button.connect("button-press-event", self.on_search_button_press_event)
        
        self.pack_start(self.entry_box, False, True)
        self.pack_start(self.search_button, False, False)
        
        self.connect("realize", self.on_realize, 74)
        self.connect("size-allocate", self.on_size_allocate, 74)
        
    def on_realize(self, widget, size):    
        rect = widget.allocation
        self.entry_box.set_size(rect.width - size, 30)
        widget.show_all()
        
    def on_size_allocate(self, widget, rect, size):
        self.entry_box.set_size(rect.width - size, 30)
        widget.show_all()
        
    def on_search_button_press_event(self, widget, event):    
        keyword =  self.entry_box.get_text().strip()
        if keyword:
            self.emit("search", keyword)
