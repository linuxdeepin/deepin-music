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
import pango

from dtk.ui.window import Window
from dtk.ui.titlebar import Titlebar
from dtk.ui.utils import (move_window, alpha_color_hex_to_cairo, 
                          color_hex_to_cairo, cairo_disable_antialias,
                          propagate_expose, get_content_size, set_cursor)
from dtk.ui.entry import InputEntry, Entry
from dtk.ui.button import ImageButton, RadioButton, CheckButton, Button
from dtk.ui.draw import draw_pixbuf, draw_text, draw_vlinear
from dtk.ui.dialog import DialogBox, DIALOG_MASK_SINGLE_PAGE
from widget.skin import app_theme
from widget.ui_utils import (draw_alpha_mask, draw_line, set_widget_gravity, is_in_rect,
                             set_widget_hcenter, set_widget_vcenter)

from constant import EMPTY_WEBCAST_ITEM, EMPTY_RADIO_ITEM
from nls import _
from config import config
from xdg_support import get_common_image

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
        icon_y = rect.y + (rect.height - pixbuf.get_height()) / 2 + 2
        
        draw_pixbuf(cr, pixbuf, icon_x, icon_y)    
        return True

class CustomEntry(gtk.VBox):
    __gsignals__ = {
        
        "clear" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "enter-press" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT,))
    }
    
    def __init__(self,  content=""):
        '''
        Initialize InputEntry class.
        '''
        # Init.
        gtk.VBox.__init__(self)
        clean_button = CleanButton()
        clean_button.connect("clicked", self.on_clean_clicked)
        self.entry = Entry(content, place_holder=_("Need some music? Find it here..."))
        entry_align = gtk.Alignment()
        entry_align.set(0.5, 0.5, 1, 1)
        entry_align.add(self.entry)
        
        clean_box = gtk.HBox(spacing=5)
        clean_box.pack_start(entry_align, True, True)
        self.clean_button_align = set_widget_gravity(clean_button, gravity=(0.5, 0.5, 0, 0),
                                                paddings=(0, 0, 0, 5))
        self.clean_button_align.set_no_show_all(True)
        
        clean_box.pack_start(self.clean_button_align, False, False)
        self.add(clean_box)
        
        self.entry.connect("press-return", self.__emit_enter_signal)
        self.entry.connect("changed", self.__on_entry_changed)
        self.connect("expose-event", self.expose_input_entry)
        
    def on_clean_clicked(self, widget):    
        # self.emit("clear")
        self.set_text("")
        
    def get_clean_visible(self):    
        return self.clean_button_align.get_visible()
    
    def hide_clean_button(self):
        self.clean_button_align.hide_all()
        self.clean_button_align.set_no_show_all(True)
        
        
    def __emit_enter_signal(self, widget):    
        self.hide_clean_button()
        self.emit("enter-press", self.get_text())
        
    def __on_entry_changed(self, widget, string):    
        if string:
            self.clean_button_align.set_no_show_all(False)
            self.clean_button_align.show_all()
        else:    
            self.clean_button_align.hide_all()
            self.clean_button_align.set_no_show_all(True)

        
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
        
        lang = utils.get_system_lang()
        if lang == "zh_CN":
            prefix = "cn"
        elif lang in ["zh_HK", "zh_TW"]:    
            prefix = "tw"
        else:    
            prefix = "en"
                
        if item_type == EMPTY_WEBCAST_ITEM:
            targets = [("text/deepin-webcasts", gtk.TARGET_SAME_APP, 1),]
            self.empty_dpixbuf = app_theme.get_pixbuf("empty/webcast_%s.png" % prefix)
        elif item_type == EMPTY_RADIO_ITEM:    
            targets = [("text/deepin-radios", gtk.TARGET_SAME_APP, 1),]
            self.empty_dpixbuf = app_theme.get_pixbuf("empty/radio_%s.png" % prefix)
            
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP,
                           targets, gtk.gdk.ACTION_COPY)
        self.connect("drag-data-received", drag_data_received_cb)
        
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        empty_pixbuf = self.empty_dpixbuf.get_pixbuf()
        
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutLeft")
        
        icon_x = rect.x + (rect.width - empty_pixbuf.get_width()) / 2 
        icon_y = rect.y + (rect.height - empty_pixbuf.get_height()) / 2 - 50
        
        draw_pixbuf(cr, empty_pixbuf, icon_x, icon_y)
        
        return True

    
class LocalEmpty(gtk.EventBox):
    
    def __init__(self, drag_data_received_cb, callback):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                        gtk.gdk.BUTTON_RELEASE_MASK |
                        gtk.gdk.POINTER_MOTION_MASK |
                        gtk.gdk.ENTER_NOTIFY_MASK |
                        gtk.gdk.LEAVE_NOTIFY_MASK
                        )

        
        self.connect("expose-event", self.on_expose_event)
        self.empty_dpixbuf = app_theme.get_pixbuf("empty/local.png")
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2), ("text/plain", 0, 3)]        
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP,
                           targets, gtk.gdk.ACTION_COPY)
        self.connect("drag-data-received", drag_data_received_cb)
        self.connect("motion-notify-event", self.on_motion_notify)
        self.connect("button-press-event", self.on_button_press)
        
        self.add_normal_dpixbuf = app_theme.get_pixbuf("empty/add_normal.png")
        self.add_hover_dpixbuf = app_theme.get_pixbuf("empty/add_hover.png")
        
        self.normal_text_dcolor = app_theme.get_color("labelText")
        self.hover_text_dcolor = app_theme.get_color("globalItemHighlight")
        self.text_padding_y = 5
        self.text_padding_x = 5
        self.prompt_offset = 15
        self.text_rect = None
        self.is_hover = False
        self.press_callback = callback
        self.prompt_text = _("Add Local Music")
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        empty_pixbuf = self.empty_dpixbuf.get_pixbuf()
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutLeft")
        pixbuf_offset_x = (rect.width - empty_pixbuf.get_width()) / 2 
        pixbuf_offset_y = (rect.height - empty_pixbuf.get_height()) / 2 - 50
        icon_x = rect.x + pixbuf_offset_x
        icon_y = rect.y + pixbuf_offset_y
        draw_pixbuf(cr, empty_pixbuf, icon_x, icon_y)
        
        add_pixbuf = self.add_normal_dpixbuf.get_pixbuf()                
        if self.is_hover:        
            text_color = self.hover_text_dcolor.get_color()
            add_pixbuf = self.add_hover_dpixbuf.get_pixbuf()
        else:    
            text_color = self.normal_text_dcolor.get_color()
        
        _width, _height = get_content_size(self.prompt_text)
        text_y = icon_y + empty_pixbuf.get_height() + self.text_padding_y        
            
        add_icon_x = icon_x + self.prompt_offset    
        add_icon_y = text_y + (_height - add_pixbuf.get_height()) / 2

        text_x = add_icon_x + add_pixbuf.get_width() + self.text_padding_x
        
        
        self.text_rect = gtk.gdk.Rectangle(text_x - rect.x, text_y - rect.y,
                                           rect.x + rect.width -  text_x - pixbuf_offset_x,
                                           _height)
            
        
        draw_pixbuf(cr, add_pixbuf, add_icon_x, add_icon_y)            
        draw_text(cr, self.prompt_text, text_x, text_y, self.text_rect.width, _height,
                  text_color=text_color, underline=True, text_size=10)
        return True
    
    def on_motion_notify(self, widget, event):
        if self.text_rect is not None:
            if is_in_rect((event.x, event.y), self.text_rect):
                self.is_hover = True
            else:    
                self.is_hover = False
            self.queue_draw()        
            
    def on_button_press(self, widget, event):        
        if self.is_hover:
            if self.press_callback:
                self.press_callback()
                self.is_hover = False
                self.queue_draw()
                
                
class NetworkConnectFailed(gtk.EventBox):
    
    def __init__(self, callback=None):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                        gtk.gdk.BUTTON_RELEASE_MASK |
                        gtk.gdk.POINTER_MOTION_MASK |
                        gtk.gdk.ENTER_NOTIFY_MASK |
                        gtk.gdk.LEAVE_NOTIFY_MASK
                        )

        
        self.connect("expose-event", self.on_expose_event)
        
        lang = utils.get_system_lang()
        if lang == "zh_CN":
            prefix = "cn"
        elif lang in ["zh_HK", "zh_TW"]:    
            prefix = "tw"
        else:    
            prefix = "en"
            
        self.failed_dpixbuf = app_theme.get_pixbuf("network/failed_%s.png" % prefix)
        self.connect("motion-notify-event", self.on_motion_notify)
        self.connect("button-press-event", self.on_button_press)
        
        self.normal_text_dcolor = app_theme.get_color("labelText")
        self.hover_text_dcolor = app_theme.get_color("globalItemHighlight")
        self.prompt_text = "点击此处刷新"
        self.text_padding_y = 5
        self.text_padding_x = 5
        self.text_rect = None
        self.is_hover = False
        self.press_callback = callback
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        failed_pixbuf = self.failed_dpixbuf.get_pixbuf()
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutLeft")
        pixbuf_offset_x = (rect.width - failed_pixbuf.get_width()) / 2 
        pixbuf_offset_y = (rect.height - failed_pixbuf.get_height()) / 2 - 50
        icon_x = rect.x + pixbuf_offset_x
        icon_y = rect.y + pixbuf_offset_y
        draw_pixbuf(cr, failed_pixbuf, icon_x, icon_y)
        
        text_y = icon_y + failed_pixbuf.get_height() + self.text_padding_y
        text_x = icon_x + self.text_padding_x
        
        _width, _height = get_content_size(self.prompt_text)
        
        self.text_rect = gtk.gdk.Rectangle(text_x - rect.x, text_y - rect.y,
                                           rect.x + rect.width -  text_x - pixbuf_offset_x,
                                           _height)
        
        if self.is_hover:        
            text_color = self.hover_text_dcolor.get_color()
        else:    
            text_color = self.normal_text_dcolor.get_color()
            
        draw_text(cr, self.prompt_text, text_x, text_y, self.text_rect.width, _height,
                  text_color=text_color, 
                  underline=True, 
                  alignment=pango.ALIGN_CENTER)
        return True
    
    def on_motion_notify(self, widget, event):
        if self.text_rect is not None:
            if is_in_rect((event.x, event.y), self.text_rect):
                self.is_hover = True
            else:    
                self.is_hover = False
            self.queue_draw()  
            
    def on_button_press(self, widget, event):        
        if self.is_hover:
            if self.press_callback:
                self.press_callback()
                self.is_hover = False
                self.queue_draw()
                
class NetworkConnectTimeout(gtk.EventBox):
    
    def __init__(self, callback=None):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                        gtk.gdk.BUTTON_RELEASE_MASK |
                        gtk.gdk.POINTER_MOTION_MASK |
                        gtk.gdk.ENTER_NOTIFY_MASK |
                        gtk.gdk.LEAVE_NOTIFY_MASK
                        )

        
        self.connect("expose-event", self.on_expose_event)
        
        lang = utils.get_system_lang()
        if lang == "zh_CN":
            prefix = "cn"
        elif lang in ["zh_HK", "zh_TW"]:    
            prefix = "tw"
        else:    
            prefix = "en"
            
        self.failed_dpixbuf = app_theme.get_pixbuf("network/timeout_%s.png" % prefix)
        self.connect("motion-notify-event", self.on_motion_notify)
        self.connect("button-press-event", self.on_button_press)
        
        self.normal_text_dcolor = app_theme.get_color("labelText")
        self.hover_text_dcolor = app_theme.get_color("globalItemHighlight")
        self.prompt_text = "点击此处刷新"
        self.text_padding_y = 5
        self.text_padding_x = 5
        self.text_rect = None
        self.is_hover = False
        self.press_callback = callback
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        failed_pixbuf = self.failed_dpixbuf.get_pixbuf()
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutLeft")
        pixbuf_offset_x = (rect.width - failed_pixbuf.get_width()) / 2 
        pixbuf_offset_y = (rect.height - failed_pixbuf.get_height()) / 2 - 50
        icon_x = rect.x + pixbuf_offset_x
        icon_y = rect.y + pixbuf_offset_y
        draw_pixbuf(cr, failed_pixbuf, icon_x, icon_y)
        
        text_y = icon_y + failed_pixbuf.get_height() + self.text_padding_y
        text_x = icon_x + self.text_padding_x
        
        _width, _height = get_content_size(self.prompt_text)
        
        self.text_rect = gtk.gdk.Rectangle(text_x - rect.x, text_y - rect.y,
                                           rect.x + rect.width -  text_x - pixbuf_offset_x,
                                           _height)
        
        if self.is_hover:        
            text_color = self.hover_text_dcolor.get_color()
        else:    
            text_color = self.normal_text_dcolor.get_color()
            
        draw_text(cr, self.prompt_text, text_x, text_y, self.text_rect.width, _height,
                  text_color=text_color, 
                  underline=True, 
                  alignment=pango.ALIGN_CENTER)
        return True
    
    def on_motion_notify(self, widget, event):
        if self.text_rect is not None:
            if is_in_rect((event.x, event.y), self.text_rect):
                self.is_hover = True
            else:    
                self.is_hover = False
            self.queue_draw()  
            
    def on_button_press(self, widget, event):        
        if self.is_hover:
            if self.press_callback:
                self.press_callback()
                self.is_hover = False
                self.queue_draw()
                
                
class AutoLabel(gtk.VBox):                
    
    def __init__(self, text):
        gtk.VBox.__init__(self)
        self.text = text
        self.connect("expose-event", self.on_expose_event)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        draw_text(cr, self.text, rect.x, rect.y, rect.width, rect.height, 
                  text_color=app_theme.get_color("labelText").get_color(),
                  alignment=pango.ALIGN_CENTER)

class WaitBox(gtk.EventBox):
    
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        self.max_number = 6
        self.item_width = self.item_height = 5
        self.padding_x = 8
        self.padding_y = 5
        self.auto_animiation_time = 100
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
    
class LoadingBox(gtk.VBox):    
    
    def __init__(self, prompt_text, click_text, callback=None):
        super(LoadingBox, self).__init__()
        
        loading_pixbuf = gtk.gdk.PixbufAnimation(get_common_image("loading.gif"))
        loading_image = gtk.Image()
        loading_image.set_from_animation(loading_pixbuf)
        
        self.click_label = ClickLabel(prompt_text, click_text, callback)
        self.update_prompt_text = self.click_label.update_prompt_text
        main_box = gtk.VBox(spacing=5)
        main_box.pack_start(loading_image)
        main_box.pack_start(set_widget_hcenter(self.click_label))
        self.add(set_widget_vcenter(main_box))                
        
        
class ClickLabel(gtk.EventBox):
    
    def __init__(self, prompt_text, click_text, callback=None, auto_size=True):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                        gtk.gdk.BUTTON_RELEASE_MASK |
                        gtk.gdk.POINTER_MOTION_MASK |
                        gtk.gdk.ENTER_NOTIFY_MASK |
                        gtk.gdk.LEAVE_NOTIFY_MASK
                        )
        
        self.normal_text_dcolor = app_theme.get_color("labelText")
        self.hover_text_dcolor = app_theme.get_color("globalItemHighlight")
        self.click_text = click_text
        self.prompt_text = prompt_text
        
        self.text_padding_y = 5
        self.text_padding_x = 5
        self.text_rect = None
        self.is_hover = False
        self.press_callback = callback

        self.adjust_size()        
        
        self.connect("expose-event", self.on_expose_event)
        self.connect("motion-notify-event", self.on_motion_notify)
        self.connect("button-press-event", self.on_button_press)
        
    def update_prompt_text(self, prompt_text):    
        self.prompt_text = prompt_text
        self.adjust_size()
        self.queue_draw()
        
    def adjust_size(self):    
        _w, _h = get_content_size(self.prompt_text)
        w = _w + self.text_padding_x * 2
        h = _h + self.text_padding_y * 2
        self.set_size_request(w, h)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        text_y = rect.y + self.text_padding_y
        text_x = rect.x + self.text_padding_x
        
        left_text, click_text, right_text = self.prompt_text.partition(self.click_text)
        
        if left_text:
            _w, _h = get_content_size(left_text)
            draw_text(cr, left_text, text_x, text_y, _w, _h, 
                      text_color=self.normal_text_dcolor.get_color())
            text_x += _w
            
        _w, _h = get_content_size(click_text)    
        self.text_rect = gtk.gdk.Rectangle(text_x - rect.x, text_y - rect.y, _w, _h)
        if self.is_hover:        
            text_color = self.hover_text_dcolor.get_color()
        else:    
            text_color = "#66B0E2" # self.normal_text_dcolor.get_color()
        draw_text(cr, click_text, text_x, text_y, _w, _h,
                  text_color=text_color, underline=True)
        text_x += _w
        
        if right_text:                
            _w, _h = get_content_size(left_text)
            draw_text(cr, right_text, text_x, text_y, _w, _h, 
                      text_color=self.normal_text_dcolor.get_color())
            
        return True
    
    def on_motion_notify(self, widget, event):
        if self.text_rect is not None:
            if is_in_rect((event.x, event.y), self.text_rect):
                self.is_hover = True
                set_cursor(widget, gtk.gdk.HAND2)
            else:    
                self.is_hover = False
                set_cursor(widget, None)
            self.queue_draw()  
            
    def on_button_press(self, widget, event):        
        if self.is_hover:
            if self.press_callback:
                self.press_callback()
                self.is_hover = False
                self.queue_draw()
        
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
                        # window_type=gtk.WINDOW_POPUP,
                        )
        
        self.set_can_focus(False)
        self.set_accept_focus(False)
        self.set_skip_taskbar_hint(True)
        self.set_skip_pager_hint(True)        
        self.info_panel = gtk.EventBox()
        self.info_panel.set_visible_window(False)
        self.set_keep_above(True)
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
        search_button_align = gtk.Alignment()
        search_button_align.set_padding(0, 0, 0, 1)
        search_button_align.set(1, 1, 1, 1)
        search_button_align.add(self.search_button)
        self.pack_start(search_button_align, False, False)
        self.entry = self.entry_box.entry
        
        self.connect("realize", self.on_realize, 74)
        self.connect("size-allocate", self.on_size_allocate, 74)
        
        self.get_text = self.entry_box.get_text
        self.set_text = self.entry_box.set_text
        
    def clear(self):    
        if self.entry_box.get_text():
            self.entry_box.set_text("")
        
    def on_realize(self, widget, size):    
        rect = widget.allocation
        if self.entry_box.get_clean_visible():
            size += 21
        self.entry_box.set_size(rect.width - size, 30)
        widget.show_all()
        
    def on_size_allocate(self, widget, rect, size):

        if self.entry_box.get_clean_visible():
            size += 21
        self.entry_box.set_size(rect.width - size, 30)            
        widget.show_all()
        
    def on_search_button_press_event(self, widget, event):    
        keyword =  self.entry_box.get_text().strip()
        self.entry_box.hide_clean_button()
        if keyword:
            self.emit("search", keyword)
            
class QuellButton(ImageButton):            
    
    def __init__(self):
        ImageButton.__init__(self,
                             app_theme.get_pixbuf("mode/quell_normal.png"),
                             app_theme.get_pixbuf("mode/quell_hover.png"),
                             app_theme.get_pixbuf("mode/quell_press.png"),
                             )
        
        
class PlaymodeButton(gtk.Button):        
    
    def __init__(self, name):
        gtk.Button.__init__(self)
        
        # Init.
        self.update_dpixbufs(name)
        
        # Set size.
        self.set_button_size()
        self.connect("expose-event", self.expose_button)
        
    def update_dpixbufs(self, name, emit_draw=False):    
        self.normal_dpixbuf = app_theme.get_pixbuf("playmode/%s_normal.png" % name)
        self.hover_dpixbuf = app_theme.get_pixbuf("playmode/%s_hover.png" % name)
        self.press_dpixbuf = app_theme.get_pixbuf("playmode/%s_press.png" % name)
        
        if emit_draw:
            self.queue_draw()
        
    def set_button_size(self):    
        request_width  = self.normal_dpixbuf.get_pixbuf().get_width()
        request_height = self.normal_dpixbuf.get_pixbuf().get_height()
        self.set_size_request(request_width, request_height)
        
    def expose_button(self, widget, event):    
        
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        
            
        if widget.state == gtk.STATE_PRELIGHT:    
            pixbuf = self.hover_dpixbuf.get_pixbuf()
        elif widget.state == gtk.STATE_ACTIVE:    
            pixbuf = self.press_dpixbuf.get_pixbuf()
        else:    
            pixbuf = self.normal_dpixbuf.get_pixbuf()            

        draw_pixbuf(cr, pixbuf, rect.x, rect.y)
        return True
        
    
class BackButton(ImageButton):    
    
    def __init__(self):
        ImageButton.__init__(self,
                             app_theme.get_pixbuf("filter/back_normal.png"),
                             app_theme.get_pixbuf("filter/back_hover.png"),
                             app_theme.get_pixbuf("filter/back_press.png")
                             )

class CleanButton(ImageButton):        
    def __init__(self):
        ImageButton.__init__(self,
                             app_theme.get_pixbuf("entry/clean_normal.png"),
                             app_theme.get_pixbuf("entry/clean_hover.png"),
                             app_theme.get_pixbuf("entry/clean_press.png")
                             )
    
class SearchCloseButton(ImageButton):
    def __init__(self):
        ImageButton.__init__(self,
                             app_theme.get_pixbuf("entry/close_normal.png"),
                             app_theme.get_pixbuf("entry/close_hover.png"),
                             app_theme.get_pixbuf("entry/close_press.png")
                             )

    
        
class SearchPrompt(gtk.EventBox):        
    
    def __init__(self, from_keyword=""):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        self.connect("expose-event", self.on_expose_event)
        self.keyword = ""
        self.from_keyword = from_keyword
        self.padding_x = 10
        self.padding_y = 8
        self.red_span = "\"<span foreground=\"red\">%s</span>\""
        self.black_span = "<span foreground=\"black\">%s</span>"
        self.prompt_format_text  = _("Your search {keyword} did not match any items in {from}")
        self.suggest_title = "<span foreground=\"black\"><b>%s:</b></span>" % _("Suggestions")
        self.suggest_first_line = "· %s" % _("Make sure all words are spelled correctly.")
        self.suggest_second_line = "· %s" % _("Try more general keywords, such as a title, an artist or the name of an album.")
        
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        rect.x += self.padding_x
        rect.width -= self.padding_x * 2
        
        keyword_span = self.red_span % self.keyword
        from_keyword_span = self.black_span % self.from_keyword
        from_keyword_dict = {"keyword" : keyword_span, "from" : from_keyword_span}
        self.prompt_text = self.prompt_format_text.format(**from_keyword_dict)
        _width, _height = get_content_size(self.prompt_text)

        draw_text(cr, self.prompt_text, rect.x, rect.y, rect.width, _height, 
                  text_color=app_theme.get_color("labelText").get_color())
        
        
        # draw dash
        rect.y += _height + self.padding_y
        dash_line_width = 1
        with cairo_disable_antialias(cr):
            cr.set_source_rgb(*color_hex_to_cairo("#D6D6D6"))            
            cr.set_line_width(dash_line_width)
            cr.set_dash([4.0, 4.0])
            cr.move_to(rect.x, rect.y)
            cr.rel_line_to(rect.width, 0)
            cr.stroke()
            
        rect.y += self.padding_y + dash_line_width
        
        _width, _height = get_content_size(self.suggest_title)
        draw_text(cr, self.suggest_title, rect.x, rect.y, rect.width, _height)
        
        rect.y += _height + self.padding_y
        _width, _height = get_content_size(self.suggest_first_line)
        draw_text(cr, self.suggest_first_line, rect.x, rect.y, rect.width, _height,
                  text_color=app_theme.get_color("labelText").get_color())

        rect.y += _height + self.padding_y
        _width, _height = get_content_size(self.suggest_second_line)
        draw_text(cr, self.suggest_second_line, rect.x, rect.y, rect.width, _height,
                  text_color=app_theme.get_color("labelText").get_color())
        return True
        
    
    def update_keyword(self, keyword):
        
        unicode_keyword = keyword.decode("utf-8")
        if len(unicode_keyword) > 30:
            new_keyword = "%s..." % unicode_keyword[:30].encode("utf-8")
        else:    
            new_keyword = keyword
        self.keyword = new_keyword
        self.queue_draw()

        
class PluginInfos(gtk.EventBox):        
    
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        
        self.connect("expose-event", self.on_expose_event)
        # self.title = "插件信息"
        self.plugin_info = None
        self.padding_x = 5
        self.padding_y = 5
        # self.custom_span = "<span foreground=\"#4D5154\">%s</span>"
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        rect.x += self.padding_x
        rect.width -= self.padding_x * 2
        
        # Draw title.
        # _width, _height = get_content_size(self.title)
        # draw_text(cr, self.title, rect.x, rect.y, rect.width, _height)
        # rect.y += self.padding_y + _height
        
        # Draw dashed.
        dash_line_width = 1
        with cairo_disable_antialias(cr):
            cr.set_source_rgb(*color_hex_to_cairo("#D6D6D6"))            
            cr.set_line_width(dash_line_width)
            cr.set_dash([4.0, 4.0])
            cr.move_to(rect.x, rect.y)
            cr.rel_line_to(rect.width, 0)
            cr.stroke()
            
        rect.y += self.padding_y + dash_line_width    
        
        # Draw plugin name.
        if self.plugin_info:
            plugin_name = "%s: %s" % (_("Name"), utils.xmlescape(self.plugin_info.get("Name", "")))    
            _width, _height = get_content_size(plugin_name)
            draw_text(cr, plugin_name, rect.x, rect.y, rect.width, _height)        
            
            rect.y += self.padding_y + _height
            

            # Draw plugin Authors.
            plugin_authors = plugin_authors = "%s: %s" % (_("Author(s)"), 
                                                         utils.xmlescape(self.plugin_info.get("Authors", "")))
            _width, _height = get_content_size(plugin_authors)            
            draw_text(cr, plugin_authors, rect.x, rect.y, rect.width, _height)
            
            rect.y += self.padding_y + _height
            
            # Draw plugin description
            plugin_description = self.plugin_info.get("Description", "")            
            _width, _height = get_content_size(plugin_description)            
            draw_text(cr, plugin_description, rect.x, rect.y, rect.width, _height)
            
        return True    
    
    def update_info(self, plugin_info):
        self.plugin_info = plugin_info
        self.queue_draw()
        
class QuitDialog(DialogBox):        
    
    def __init__(self, confirm_callback=None):
        DialogBox.__init__(self, 
                           title=_("Close"),
                           default_width=360,
                           default_height=145,
                           mask_type=DIALOG_MASK_SINGLE_PAGE,
                           )
        
        self.confirm_callback = confirm_callback
        radio_group = gtk.HBox(spacing=50)
        self.minimize_radio = RadioButton(_("Minimize to tray"))
        self.minimize_radio.set_active(True)
        self.quit_radio = RadioButton(_("Quit"))
        
        radio_group.pack_start(self.minimize_radio, False, True)
        radio_group.pack_start(self.quit_radio, False, True)
        self.remembar_button = CheckButton(_("Don't prompt again"))
        self.remembar_button.set_active(True)
        
        radio_group_align = gtk.Alignment()
        radio_group_align.set_padding(30, 0, 10, 0)
        radio_group_align.add(radio_group)
                
        confirm_button = Button(_("OK"))
        confirm_button.connect("clicked", self.on_confirm_button_clicked)

        cancel_button = Button(_("Cancel"))
        cancel_button.connect("clicked", self.on_cancel_button_clicked)        
        
        # Connect widgets.
        self.body_box.pack_start(radio_group_align, False, True)
        self.left_button_box.set_buttons([self.remembar_button,])
        self.right_button_box.set_buttons([confirm_button, cancel_button])
        
    def on_confirm_button_clicked(self, widget):    
        self.change_quit_status()
        if self.confirm_callback != None:
            self.confirm_callback()
        self.destroy()    
    
    def on_cancel_button_clicked(self, widget):
        self.destroy()
        
    def change_quit_status(self):    
        status = "false"
        if self.minimize_radio.get_active():
            status = "true"
        elif self.quit_radio.get_active():    
            status = "false"
        config.set("setting", "close_to_tray", status)
        
        if self.remembar_button.get_active():
            status = "true"
        else:    
            status = "false"
        config.set("setting", "close_remember", status)    

        
        
        
