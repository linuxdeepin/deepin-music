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
import pango

from collections import namedtuple
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.treeview import TreeView, TreeItem
from dtk.ui.draw import draw_pixbuf, draw_text
from dtk.ui.utils import get_content_size, get_widget_root_coordinate, get_match_parent
from dtk.ui.constant import WIDGET_POS_TOP_RIGHT
from dtk.ui.popup_grab_window import PopupGrabWindow, wrap_grab_window
from dtk.ui.window import Window
from dtk.ui.paned import HPaned

import utils
from widget.ui_utils import (draw_single_mask, draw_alpha_mask, render_item_text,
                             switch_tab, draw_range, draw_line)
from widget.skin import app_theme
from widget.webcast_view import MultiDragWebcastView
from collections import OrderedDict
from constant import DEFAULT_FONT_SIZE
from webcasts import WebcastsDB
from xdg_support import get_config_file
from helper import Dispatcher
from song import Song
from nls import _

class WebcastTreeItem(TreeItem):
    
    def __init__(self, column_index=0):
        TreeItem.__init__(self)
        self.column_index = column_index
        self.side_padding = 5
        self.webcast_icon = app_theme.get_pixbuf("filter/album_normal.png")
        
    def get_height(self):    
        pass
    
    def get_column_widths(self):
        pass
    
    def get_column_renders(self):
        pass
    
    def unselect(self):
        self.is_select = False
        if self.redraw_request_callback:
            self.radraw_request_callback(self)
            
    def select(self):        
        self.is_select = True
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def render_webcast_icon(self, cr, rect):        
        # Draw select background.
        if self.is_select:
            draw_single_mask(cr, rect.x, rect.y, rect.w, rect.h, "simpleItemHighlight")
            
        draw_pixbuf(cr, self.webcast_icon, rect.x + self.side_padding, 
                    rect.y + (rect.height - self.webcast_icon.get_height()) / 2)
        
    def render_webcast_name(self, cr, rect):    
        if self.is_select:
            draw_single_mask(cr, rect.x, rect.y, rect.w, rect.h, "simpleItemHighlight")
            
        draw_text(cr, self.name, rect.x, rect.y, rect.w, rect.h)    
        
        
class WebcastListItem(gobject.GObject):
    
    __gsignals__ = {"redraw-request" : ( gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, tags, draw_collect=False):
        gobject.GObject.__init__(self)
        
        self.webcast = Song()
        self.webcast.init_from_dict(tags)
        self.webcast.set_type("webcast")
        if draw_collect:
            self.is_collected = True
        else:    
            self.is_collected = WebcastsDB.is_collected(tags["uri"])        

        self.webcast["collect"] = self.is_collected
        self.index = None
        self.webcast_normal_pixbuf = app_theme.get_pixbuf("webcast/webcast_normal.png").get_pixbuf()
        self.webcast_press_pixbuf = app_theme.get_pixbuf("webcast/webcast_press.png").get_pixbuf()
        self.collect_normal_pixbuf = app_theme.get_pixbuf("webcast/collect_normal.png").get_pixbuf()
        self.collect_press_pixbuf = app_theme.get_pixbuf("webcast/collect_press.png").get_pixbuf()
        
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
        self.title_w, self.title_h = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
        self.collect_icon_padding_x = 2
        self.collect_icon_padding_y = 5
        self.collect_icon_w = self.collect_normal_pixbuf.get_width()
        self.collect_icon_h = self.collect_normal_pixbuf.get_height()
        
    def render_webcast_icon(self, cr, rect, in_select, in_highlight):    
        icon_x = rect.x + self.webcast_icon_padding_x 
        icon_y = rect.y + (rect.height - self.webcast_icon_h) / 2
        if in_select:
            pixbuf = self.webcast_press_pixbuf
        else:    
            pixbuf = self.webcast_normal_pixbuf
        draw_pixbuf(cr, pixbuf, icon_x, icon_y)
        
    def render_title(self, cr, rect, in_select, in_highlight):    
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_item_text(cr, self.title, rect, in_select, in_highlight)
        
    def render_collect_icon(self, cr, rect, in_select, in_highlight):    
        icon_y = rect.y + (rect.height - self.collect_icon_h) / 2
        rect.x += self.collect_icon_padding_x
        if self.is_collected:
            pixbuf = self.collect_press_pixbuf
        else:    
            pixbuf = self.collect_normal_pixbuf
        draw_pixbuf(cr, pixbuf, rect.x , icon_y)
        
    def render_block(self, cr, rect, in_select, in_highlight):    
        pass
    
    def get_column_sizes(self):
        return [
            (self.webcast_icon_w + self.webcast_icon_padding_x * 2, self.webcast_icon_h + self.webcast_icon_padding_y * 2),
            (360, self.title_h + self.title_padding_y * 2),
            (self.collect_icon_w + self.collect_icon_padding_x * 2, self.collect_icon_h + self.collect_icon_padding_y * 2),
            (50, 1)
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
        
    def get_tags(self):    
        return self.webcast.get_dict()
        
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
    
    
class CategroyItem(TreeItem):    
    def __init__(self, title, webcast_key, sourcebar, index, show_icon=False):
        TreeItem.__init__(self)
        self.column_index = 0
        self.side_padding = 5
        self.item_height = 37
        self.title = utils.xmlescape(title)
        self.item_width = 121
        self.webcast_key = webcast_key
        self.hover_bg = app_theme.get_pixbuf("webcast/categroy_bg.png").get_pixbuf()
        self.selected_pixbuf = app_theme.get_pixbuf("webcast/source_sselect.png").get_pixbuf()
        owner_keys = WebcastsDB.get_keys_from_categroy(webcast_key)
        panel_items = [PanelItem(webcast_key, key, sourcebar) for key in owner_keys]
        self.popup_panel = PopupPanel()
        self.popup_panel.add_items(panel_items)
        self.parent_widget = sourcebar
        self.item_index = index
        self.has_icon = show_icon
        
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
            draw_pixbuf(cr, self.hover_bg, rect.x, rect.y)
            text_color = app_theme.get_color("simpleItemHighlight").get_color()
        elif self.is_hover:    
            text_color = app_theme.get_color("simpleItemHighlight").get_color()
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        draw_text(cr, self.title, rect.x, rect.y, rect.width, rect.height, text_size=10, 
                  text_color = text_color,
                  alignment=pango.ALIGN_CENTER)    
        
        if self.has_icon:
            draw_pixbuf(cr, self.selected_pixbuf, rect.x + 10,
                        rect.y + (rect.height - self.selected_pixbuf.get_height()) / 2)
        
    def hide_icon(self):
        self.has_icon = False
        self.emit_redraw_request()
        
    def show_icon(self):    
        self.has_icon = True
        self.emit_redraw_request()
        
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
        
    def adjust_popup_coord(self):    
        origin_x, origin_y = get_widget_root_coordinate(self.parent_widget, WIDGET_POS_TOP_RIGHT)        
        origin_y += self.item_height * self.item_index
        return origin_x, origin_y
    
    def button_press(self, column, offset_x, offset_y):
        self.popup_panel.show(*self.adjust_popup_coord())                
        popup_grab_window.popup_grab_window_focus_in()
    
    def single_click(self, column, offset_x, offset_y):
        pass        

    def double_click(self, column, offset_x, offset_y):
        pass        
    
    def draw_drag_line(self, drag_line, drag_line_at_bottom=False):
        pass
    
class CustomItem(TreeItem):    
    def __init__(self, title, callback=None):
        TreeItem.__init__(self)
        self.column_index = 0
        self.side_padding = 5
        self.item_height = 37
        self.title = utils.xmlescape(title)
        self.item_width = 121
        self.hover_bg = app_theme.get_pixbuf("webcast/categroy_bg.png").get_pixbuf()
        self.press_callback = callback
        
        self.parent_widget = None
        self.item_index = -1
        self.has_icon = False
        self.webcast_key = ""
        
        
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
        # if self.is_select:
        #     draw_pixbuf(cr, self.hover_bg, rect.x, rect.y)
        #     text_color = app_theme.get_color("simpleItemSelect").get_color()
        if self.is_hover:    
            text_color = app_theme.get_color("simpleItemHighlight").get_color()
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        draw_text(cr, self.title, rect.x, rect.y, rect.width, rect.height, text_size=10, 
                  text_color = text_color,
                  alignment=pango.ALIGN_CENTER)    
        
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
        if self.press_callback:
            self.press_callback()
    
    def single_click(self, column, offset_x, offset_y):
        pass        

    def double_click(self, column, offset_x, offset_y):
        pass        
    
    def draw_drag_line(self, drag_line, drag_line_at_bottom=False):
        pass
    
    def hide_icon(self):
        pass
    
    def show_icon(self):
        pass
    
class PanelItem(gobject.GObject):    
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }
    
    def __init__(self, parent, title, allocation_widget):
        gobject.GObject.__init__(self)
        self.hover_flag = False
        self.allocation_widget = allocation_widget
        self.highlight_flag = False
        self.owner_key = self.title = utils.xmlescape(title)
        self.parent_key = parent
        self.has_underline = True
        
    def get_title(self):    
        return self.title
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
    
    def render(self, cr, rect):    
        if not self.hover_flag:
            color = "#333333"
            self.underline = False
        else:    
            if self.has_underline:            
                self.underline = True
            color = app_theme.get_color("simpleItemHighlight").get_color()
        draw_text(cr, self.title, rect.x, rect.y, rect.width, rect.height, text_color=color, underline=self.underline)
        
    def get_size(self):    
        return get_content_size(self.title, DEFAULT_FONT_SIZE)
        
    def motion_notify(self, x, y):    
        self.has_underline =True
        self.hover_flag = True
        self.emit_redraw_request()
        
    def lost_focus(self):    
        self.hover_flag = False
        self.emit_redraw_request()
        
    def button_press(self):    
        self.underline = False
        self.has_underline = False
        self.emit_redraw_request()
        self.show_selected_icon()
        
    def show_selected_icon(self):    
        items = self.allocation_widget.visible_items
        for item in items:
            if item.webcast_key != self.parent_key:
                item.hide_icon()
            else:    
                item.show_icon()
    
class PopupPanel(Window):
    
    def __init__(self, default_width=320 ,separate_text=" | "):
        Window.__init__(self, 
                        shadow_visible=False,
                        shadow_radius=0,
                        shape_frame_function=self.shape_panel_frame,
                        expose_frame_function=self.expose_panel_frame,
                        window_type=gtk.WINDOW_POPUP)
        
        self.set_size_request(300, 400)
        self.panel = gtk.Button()
        self.panel.tag_by_poup_panel_grab_window = True
        self.panel.add_events(gtk.gdk.POINTER_MOTION_MASK |
                              gtk.gdk.BUTTON_PRESS_MASK |
                              gtk.gdk.BUTTON_RELEASE_MASK)
        
        self.panel.connect("expose-event", self.on_panel_expose_event)
        self.panel.connect("motion-notify-event", self.on_panel_motion_notify_event)
        self.panel.connect("button-press-event", self.on_panel_button_press_event)
        self.panel.connect("button-release-event", self.on_panel_button_release_event)

        # Init.
        self.padding_x = 10
        self.padding_y = 5
        self.separate_text = separate_text
        
        self.default_width = default_width
        self.separate_width, self.separate_height = get_content_size(self.separate_text)
        self.coords = {}
        self.range = namedtuple("coord", "start_x end_x start_y end_y")
        self.hover_item = None
        self.items = []
        self.block_height = 5
        self.item_interval_height = 12
        
        # Redraw.
        self.redraw_delay = 100 # 100 milliseconds should be enough for redraw
        self.redraw_request_list = []
        gobject.timeout_add(self.redraw_delay, self.update_redraw_request_list)
        
        self.window_frame.add(self.panel)
        
        # Wrap self in poup grab window.
        wrap_grab_window(popup_grab_window, self)
        
    def shape_panel_frame(self, widget, event):    
        pass
        
    def expose_panel_frame(self, widget, event):
        cr  = widget.window.cairo_create()
        rect = widget.allocation
        cr.set_source_rgb(1,1,1)
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
    def on_popup_panel_realiz(self, widget):    
        pass
        
    def on_panel_expose_event(self, widget, event):    
        if not self.items:
            return 
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        draw_line(cr, (rect.x, rect.y + 1), (rect.x + rect.width, rect.y + 1), "#c7c7c7")
        draw_line(cr, (rect.x + rect.width, rect.y), (rect.x + rect.width, rect.y + rect.height), "#c7c7c7")
        draw_line(cr, (rect.x, rect.y + rect.height), (rect.x + rect.width, rect.y + rect.height), "#c7c7c7")
        draw_line(cr, (rect.x + 1, rect.y + 32), (rect.x + 1, rect.y + rect.height), "#c7c7c7")
        rect.y += self.block_height        
        
        start_x, start_y = self.padding_x, self.padding_y
        
        for index, item in enumerate(self.items):
            item_width, item_height = item.get_size()
            if rect.width - start_x < item_width + self.separate_width:
                start_y += item_height + self.item_interval_height
                start_x = self.padding_x
                
            item.render(cr, gtk.gdk.Rectangle(rect.x + start_x, rect.y + start_y,
                                              item_width, item_height))    
            
            self.coords[index] = self.range(rect.x + start_x, rect.x + start_x + item_width,
                                            rect.y + start_y, rect.y + start_y + item_height)
            start_x += item_width            
            
            
            draw_text(cr, self.separate_text, rect.x + start_x, rect.y + start_y, self.separate_width, 
                      self.separate_height, text_color=app_theme.get_color("labelText").get_color())
            start_x += self.separate_width
            
        return True    
    
    def adjust_window_height(self):
        start_x, start_y = self.padding_x, self.padding_y
        item_height = 0
        for item in self.items:
            item_width, item_height = item.get_size()
            if self.default_width - start_x < item_width + self.separate_width:
                start_y += item_height + self.item_interval_height
                start_x = self.padding_x
                
            start_x += item_width        
            start_x += self.separate_width
        return start_y + item_height + self.block_height * 2
    
    def is_visible_area(self, event):
        for key, coord in self.coords.iteritems():
            if coord.start_x < event.x < coord.end_x and coord.start_y < event.y < coord.end_y:
                return True
        return False    
        
    
    def on_panel_motion_notify_event(self, widget, event):
        if not self.coords:
            return 
        for key, coord in self.coords.iteritems():
            if coord.start_x < event.x < coord.end_x and coord.start_y < event.y < coord.end_y:
                current_item = self.items[key]
                if self.hover_item and current_item != self.hover_item:
                    self.hover_item.lost_focus()
                current_item.motion_notify(event.x, event.y)    
                self.hover_item = current_item
                return 
            
        if self.hover_item:    
            self.hover_item.lost_focus()
            self.hover_item = None
            
    def on_panel_button_press_event(self, widget, event):        
        if self.hover_item:
            self.hover_item.button_press()
            Dispatcher.emit_webcast_info(self.hover_item.parent_key, self.hover_item.owner_key)
    
    def on_panel_button_release_event(self, widget, event):
        pass
            
    def add_items(self, items):        
        for item in items:
            item.connect("redraw-request", self.redraw_item)
        self.items = items    
        adjust_height = self.adjust_window_height()
        self.set_default_size(self.default_width, adjust_height)
        self.set_geometry_hints(
            None,
            self.default_width, adjust_height,
            self.default_width, adjust_height,
            -1, -1, -1, -1, -1, -1)
            
    def redraw_item(self, item):        
        self.redraw_request_list.append(item)
        
    def update_redraw_request_list(self):    
        if len(self.redraw_request_list) > 0:
            self.panel.queue_draw()
            
        self.redraw_request_list = []    
        return True
    
    def show(self, x, y):
        self.move(x, y)
        self.show_all()
            
gobject.type_register(PopupPanel)        

class PopupPanelGrabWindow(PopupGrabWindow):
    '''
    class docs
    '''
	
    def __init__(self):
        '''
        init docs
        '''
        PopupGrabWindow.__init__(self, PopupPanel)
        self.button_press_callback = None
        
    def popup_grab_window_motion_notify(self, widget, event):
        '''
        Handle `motion-notify` signal of popup_grab_window.
    
        @param widget: Popup_Window widget.
        @param event: Motion notify signal.
        '''
        if event and event.window:
            event_widget = event.window.get_user_data()
            if isinstance(event_widget, gtk.DrawingArea) and hasattr(event_widget, "tag_by_poup_panel_grab_window"):
                event_widget.event(event)
            elif isinstance(event_widget, gtk.Button) and hasattr(event_widget, "tag_by_poup_panel_grab_window"):
                event_widget.event(event)

    def popup_grab_window_leave_notify(self, widget, event):
        '''
        Handle `motion-notify` signal of popup_grab_window.
    
        @param widget: Popup_Window widget.
        @param event: Motion notify signal.
        '''
        if event and event.window:
            event_widget = event.window.get_user_data()
            if isinstance(event_widget, gtk.DrawingArea) and hasattr(event_widget, "tag_by_poup_panel_grab_window"):
                event_widget.event(event)
            elif isinstance(event_widget, gtk.Button) and hasattr(event_widget, "tag_by_poup_panel_grab_window"):
                event_widget.event(event)

    def popup_grab_window_button_press(self, widget, event):
        '''
        Handle `button-press-event` signal of popup_grab_window.
    
        @param widget: Popup_Window widget.
        @param event: Button press event.
        '''
        self.press_flag = True
        
        if event and event.window:
            event_widget = event.window.get_user_data()
            # print "-------------"
            # print "%s\n" % (event_widget)
            if self.is_press_on_popup_grab_window(event.window):
                if self.button_press_callback:
                    self.button_press_callback()
                self.popup_grab_window_focus_out()
            elif isinstance(event_widget, ScrolledWindow) and hasattr(event_widget, "tag_by_popup_grab_window"):
                event_widget.event(event)
            elif isinstance(event_widget, self.wrap_window_type):
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
            elif isinstance(event_widget, gtk.DrawingArea) and hasattr(event_widget, "tag_by_poup_panel_grab_window"):
                parent_widget = get_match_parent(event_widget, "TreeView")
                if parent_widget.is_in_visible_area(event):
                    if self.button_press_callback:
                        self.button_press_callback()
                self.popup_grab_window_focus_out()
                event_widget.event(event)
                
            elif isinstance(event_widget, gtk.Button) and hasattr(event_widget, "tag_by_poup_panel_grab_window"):
                popup_panel_widget = get_match_parent(event_widget, "PopupPanel")
                if popup_panel_widget.is_visible_area(event):
                    event_widget.event(event)
                    if self.button_press_callback:
                        self.button_press_callback()
                    self.popup_grab_window_focus_out()
            else:
                if self.button_press_callback:
                    self.button_press_callback()
                event_widget.event(event)
                self.popup_grab_window_focus_out()
                
popup_grab_window = PopupPanelGrabWindow()                
popup_grab_window.button_press_callback = Dispatcher.clear_sourcebar_status

class WebcastsBrowser(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        # Init data.
        self.source_data = OrderedDict()
        self.source_data["internal"] = "国内广播"
        self.source_data["foreign"]  = "国外广播"
        self.source_data["network"] = "网络广播"
        self.source_data["genres"] = "流派广播"
        self.source_data["music"]  = "音乐广播"
        self.source_data["finance"] = "财经广播"
        self.source_data["sports"] = "体育广播"
        
        # Init sourcebar
        self.__init_sourcebar()
        
        # Init webcasts view.
        self.source_view, self.source_sw = self.get_webcasts_view()
        self.collect_view, self.collect_sw = self.get_webcasts_view()
        # self.custom_view, custom_view_sw = self.get_webcasts_view()
        
        self.source_view.connect("single-click-item", self.on_source_view_single_click_item)
        
        if WebcastsDB.isloaded():
            self.on_webcastsdb_loaded()
        else:    
            self.connect_to_webcastsdb()
            
        # Dispatcher
        Dispatcher.connect("webcast-info", self.on_dispatcher_webcast_info)    
        Dispatcher.connect("clear-sourcebar-status", lambda obj: self.clear_sourcebar_status())
        
        # Used to switch categroy view.
        self.switch_view_box = gtk.VBox()
        self.switch_view_box.add(self.source_sw)
        
        body_paned = HPaned(handle_color=app_theme.get_color("panedHandler"), enable_drag=True)
        body_paned.add1(self.sourcebar)
        body_paned.add2(self.switch_view_box)
        
        self.add(body_paned)
        self.show_all()
        
    def on_dispatcher_webcast_info(self, obj, parent, key):    
        items = WebcastsDB.get_items(parent, key)
        self.source_view.clear()
        if items:
            self.source_view.add_items([WebcastListItem(tag) for tag in items])        
            switch_tab(self.switch_view_box, self.source_sw)
        
    def connect_to_webcastsdb(self):    
        WebcastsDB.connect("loaded", self.on_webcastsdb_loaded)
        
    def on_webcastsdb_loaded(self, *args):    
        items = WebcastsDB.get_items("internal", "中央电台")
        self.source_view.add_items([WebcastListItem(tag) for tag in items])        
        
        # load collect webcasts.
        collect_taglist = WebcastsDB.get_favorite_items()
        if collect_taglist:
            self.collect_view.add_items([WebcastListItem(tag) for tag in collect_taglist])
        
    def __init_sourcebar(self):
        self.sourcebar = TreeView(enable_drag_drop=False, enable_multiple_select=False)
        items = []
        for index, (key, value) in enumerate(self.source_data.items()):
            if index == 0: show_icon = True
            else: show_icon = False
            items.append(CategroyItem(value, key, self.sourcebar, index, show_icon))
        items.append(CustomItem("我的收藏", lambda : switch_tab(self.switch_view_box, self.collect_sw)))    
                
        self.sourcebar.add_items(items)
        self.sourcebar.set_size_request(121, -1)
        self.sourcebar.draw_mask = self.on_sourcebar_draw_mask        
        self.sourcebar.draw_area.tag_by_poup_panel_grab_window = True
        popup_grab_window.focus_out_callback = self.handle_focus_out
        
    def handle_focus_out(self):
        self.sourcebar.unhover_row()
        self.sourcebar.left_button_press = False
        
    def get_webcasts_view(self):    
        webcast_view = MultiDragWebcastView()
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        scrolled_window.add_child(webcast_view)
        return webcast_view, scrolled_window
    
    def on_sourcebar_draw_mask(self, cr, x, y, w, h):    
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")

        return False
    
    def on_source_view_single_click_item(self, widget, item, column, x, y):
        if column == 2:
            item.toggle_is_collected()
            if item.is_collected:
                tags = item.webcast.get_dict()
                self.collect_view.add_items([WebcastListItem(tags, True)])                
            else:    
                for c_item in self.collect_view.items:
                    if c_item == item:
                        self.collect_view.delete_items([c_item])
                        del c_item
                        
    def save(self):                    
        if not self.collect_view.items:
            return
        items = []
        for item in self.collect_view.items:
            items.append(item.get_tags())
        utils.save_db(items, get_config_file("favorite_webcasts.db"))    
        
    def clear_sourcebar_status(self):    
        items = self.sourcebar.visible_items
        if items:
            [item.unselect() for item in items]
