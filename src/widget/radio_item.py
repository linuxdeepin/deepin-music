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
import os

from dtk.ui.threads import post_gui
from dtk.ui.new_treeview import TreeItem
from dtk.ui.draw import draw_pixbuf, draw_text
from dtk.ui.utils import (get_content_size)

from dtk.ui.thread_pool import MissionThread

import utils
from widget.ui_utils import (draw_single_mask, render_text)
from widget.ui import CoverPopupNotify
from widget.skin import app_theme
from nls import _
from cover_manager import DoubanCover
from constant import LIST_WIDTH

class CategroyTreeItem(TreeItem):    
    def __init__(self, title, callback=None):
        TreeItem.__init__(self)
        self.column_index = 0
        self.side_padding = 5
        self.item_height = 37
        self.title = title
        self.item_width = 121
        self.press_callback = callback
        
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
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "simpleItemSelect")
        elif self.is_hover:
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "simpleItemHover")
        
        if self.is_select:
            text_color = "#FFFFFF"
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
        pass
    
    def single_click(self, column, offset_x, offset_y):
        if self.press_callback:
            self.press_callback()

    def double_click(self, column, offset_x, offset_y):
        pass        
    
    def draw_drag_line(self, drag_line, drag_line_at_bottom=False):
        pass

    
class CommonIconItem(gobject.GObject, MissionThread):    
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, chl):
        '''
        Initialize ItemIcon class.
        
        @param pixbuf: Icon pixbuf.
        '''
        gobject.GObject.__init__(self)
        MissionThread.__init__(self)
        self.padding_x = 10
        self.padding_y = 10
        self.default_width = 110
        self.default_height = 95
        self.pixbuf_rect = None
        self.hover_flag = False
        self.mask_flag = False
        self.highlight_flag = False
        self.pixbuf = None
        self.mask_pixbuf = None
        self.chl = chl
        self.title = chl.get("name", "")
        self.description = "%s首歌曲" % chl.get("song_num")
        self.create_cover_pixbuf()
        self.notify = CoverPopupNotify(chl)

    def create_cover_pixbuf(self):    
        cover_path = DoubanCover.get_cover(self.chl, try_web=False)
        if cover_path:
            try:
                self.pixbuf = gtk.gdk.pixbuf_new_from_file(cover_path)
                self.is_loaded_cover = True                
            except gobject.GError:    
                self.pixbuf = app_theme.get_pixbuf("slide/default_cover.png").get_pixbuf()
                self.is_loaded_cover = False
        else:    
            self.pixbuf = app_theme.get_pixbuf("slide/default_cover.png").get_pixbuf()
            self.is_loaded_cover = False
            
    def create_mask_pixbuf(self):        
        if self.mask_pixbuf is None:
            self.mask_pixbuf = app_theme.get_pixbuf("radio/covermask_play.png").get_pixbuf()
            
    def start_mission(self):    
        cover_path = DoubanCover.get_cover(self.chl,try_web=True)
        if cover_path:
            self.render_cover(cover_path)
            
    @post_gui    
    def render_cover(self,cover_path):
        self.pixbuf = gtk.gdk.pixbuf_new_from_file(cover_path)
        # self.emit_redraw_request()
        
    def emit_redraw_request(self):
        '''
        Emit `redraw-request` signal.
        
        This is IconView interface, you should implement it.
        '''
        self.emit("redraw-request")
        
    def get_width(self):
        '''
        Get item width.
        
        This is IconView interface, you should implement it.
        '''
        return self.default_width
        
    def get_height(self):
        '''
        Get item height.
        
        This is IconView interface, you should implement it.
        '''
        return self.default_height
    
    def render(self, cr, rect):
        '''
        Render item.
        
        This is IconView interface, you should implement it.
        '''
        # Draw cover.
            
        if self.pixbuf is None:
            self.create_cover_pixbuf()
             
        pixbuf_x = rect.x + (rect.width - self.pixbuf.get_width()) / 2
            
        draw_pixbuf(cr, self.pixbuf, pixbuf_x, rect.y)
        
        if self.pixbuf_rect is None:
            self.pixbuf_rect = gtk.gdk.Rectangle((rect.width - self.pixbuf.get_width()) / 2, 
                                                 0, self.pixbuf.get_width(), self.pixbuf.get_height())
            
        if self.mask_flag:    
            if self.mask_pixbuf is None:
                self.create_mask_pixbuf()
            draw_pixbuf(cr, self.mask_pixbuf, pixbuf_x, rect.y)    
        
            
        title_rect = gtk.gdk.Rectangle(rect.x + self.padding_x, 
                                       rect.y + self.pixbuf.get_height() + 5,
                                       rect.width - self.padding_x * 2, 11)
        total_rect = gtk.gdk.Rectangle(title_rect.x, title_rect.y + 16, title_rect.width, 9)
        
        render_text(cr, utils.xmlescape(self.title), title_rect, 
                    # app_theme.get_color("labelText").get_color(),
                    "#444444",
                    10)
        render_text(cr, utils.xmlescape(self.description), total_rect,
                    app_theme.get_color("labelText").get_color(),
                    8)
        
    def icon_item_motion_notify(self, x, y):
        '''
        Handle `motion-notify-event` signal.
        
        This is IconView interface, you should implement it.
        '''
        if self.pointer_in_pixbuf(x, y):
            self.mask_flag = True
        else:    
            self.mask_flag = False
            self.notify.hide_all()
        
        self.hover_flag = True
        self.emit_redraw_request()
        
    def try_show_notify(self, x, y):    
        if self.mask_flag:
            show_x = self.pixbuf_rect.x + self.pixbuf_rect.width + x
            show_y = self.pixbuf_rect.y + self.pixbuf_rect.height + y
            self.notify.show(show_x, show_y)
        else:    
            self.notify.hide_all()
        
    def pointer_in_pixbuf(self, x, y):    
        if self.pixbuf_rect is None: return False
        if self.pixbuf_rect.x <= x <= self.pixbuf_rect.x + self.pixbuf_rect.width and \
              self.pixbuf_rect.y <= y <= self.pixbuf_rect.y + self.pixbuf_rect.height:  
            return True
        else:
            return False
        
    def icon_item_lost_focus(self):
        '''
        Lost focus.
        
        This is IconView interface, you should implement it.
        '''
        self.hover_flag = False
        self.mask_flag = False
        self.emit_redraw_request()        
        self.notify.hide_all()
        
    def hide_notify(self):    
        self.notify.hide_all()
                
    def icon_item_highlight(self):
        '''
        Highlight item.
        
        This is IconView interface, you should implement it.
        '''
        self.highlight_flag = True

        self.emit_redraw_request()
        
    def icon_item_normal(self):
        '''
        Set item with normal status.
        
        This is IconView interface, you should implement it.
        '''
        self.highlight_flag = False
        
        self.emit_redraw_request()
    
    def icon_item_button_press(self, x, y):
        '''
        Handle button-press event.
        
        This is IconView interface, you should implement it.
        '''
        pass        
    
    def icon_item_button_release(self, x, y):
        '''
        Handle button-release event.
        
        This is IconView interface, you should implement it.
        '''
        pass
    
    def icon_item_single_click(self, x, y):
        '''
        Handle single click event.
        
        This is IconView interface, you should implement it.
        '''
        pass

    def icon_item_double_click(self, x, y):
        '''
        Handle double click event.
        
        This is IconView interface, you should implement it.
        '''
        pass
    
    def icon_item_release_resource(self):
        '''
        Release item resource.

        If you have pixbuf in item, you should release memory resource like below code:

        >>> if self.pixbuf:
        >>>     del self.pixbuf
        >>>     self.pixbuf = None
        >>>
        >>> return True

        This is IconView interface, you should implement it.
        
        @return: Return True if do release work, otherwise return False.
        
        When this function return True, IconView will call function gc.collect() to release object to release memory.
        '''
        if self.pixbuf:
            del self.pixbuf
            self.pixbuf = None
        if self.mask_pixbuf:    
            del self.mask_pixbuf
            self.mask_pixbuf = None
            
        return True
    
class MoreIconItem(gobject.GObject):    
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self):
        '''
        Initialize ItemIcon class.
        
        @param pixbuf: Icon pixbuf.
        '''
        gobject.GObject.__init__(self)
        self.padding_x = 10
        self.padding_y = 10
        self.default_width = 110
        self.default_height = 95
        self.pixbuf_rect = None
        self.hover_flag = False
        self.mask_flag = False
        self.highlight_flag = False
        self.pixbuf = None
        self.mask_pixbuf = None
        self.is_more = True
        
    def emit_redraw_request(self):
        '''
        Emit `redraw-request` signal.
        
        This is IconView interface, you should implement it.
        '''
        self.emit("redraw-request")
        
    def get_width(self):
        '''
        Get item width.
        
        This is IconView interface, you should implement it.
        '''
        return self.default_width
        
    def get_height(self):
        '''
        Get item height.
        
        This is IconView interface, you should implement it.
        '''
        return self.default_height
    
    def create_pixbuf(self):        
        if self.pixbuf is None:
            self.pixbuf = app_theme.get_pixbuf("slide/default_cover.png").get_pixbuf()
            
    def create_mask_pixbuf(self):       
        if self.mask_pixbuf is None:
            self.mask_pixbuf = app_theme.get_pixbuf("radio/covermask_play.png").get_pixbuf()
    
    def render(self, cr, rect):
        '''
        Render item.
        
        This is IconView interface, you should implement it.
        '''
        if self.pixbuf is None:
            self.create_pixbuf()
             
        pixbuf_x = rect.x + (rect.width - self.pixbuf.get_width()) / 2
        draw_pixbuf(cr, self.pixbuf, pixbuf_x, rect.y)
        
        if self.pixbuf_rect is None:
            self.pixbuf_rect = gtk.gdk.Rectangle((rect.width - self.pixbuf.get_width()) / 2, 
                                                 0, self.pixbuf.get_width(), self.pixbuf.get_height())
        if self.mask_flag:    
            if self.mask_pixbuf is None:
                self.create_mask_pixbuf()
            draw_pixbuf(cr, self.mask_pixbuf, pixbuf_x, rect.y)    
        
            
        title_rect = gtk.gdk.Rectangle(rect.x + self.padding_x, 
                                       rect.y + self.pixbuf.get_height() + 5,
                                       rect.width - self.padding_x * 2, 11)
        
        render_text(cr, _("更多"), title_rect, 
                    app_theme.get_color("labelText").get_color(),
                    10)
        
    def icon_item_motion_notify(self, x, y):
        '''
        Handle `motion-notify-event` signal.
        
        This is IconView interface, you should implement it.
        '''
        if self.pointer_in_pixbuf(x, y):
            self.mask_flag = True
        else:    
            self.mask_flag = False
        
        self.hover_flag = True
        self.emit_redraw_request()
        
    def pointer_in_pixbuf(self, x, y):    
        if self.pixbuf_rect is None: return False
        if self.pixbuf_rect.x <= x <= self.pixbuf_rect.x + self.pixbuf_rect.width and \
              self.pixbuf_rect.y <= y <= self.pixbuf_rect.y + self.pixbuf_rect.height:  
            return True
        else:
            return False
        
    def icon_item_lost_focus(self):
        '''
        Lost focus.
        
        This is IconView interface, you should implement it.
        '''
        self.hover_flag = False
        self.mask_flag = False
        
        self.emit_redraw_request()
        
    def icon_item_highlight(self):
        '''
        Highlight item.
        
        This is IconView interface, you should implement it.
        '''
        self.highlight_flag = True

        self.emit_redraw_request()
        
    def icon_item_normal(self):
        '''
        Set item with normal status.
        
        This is IconView interface, you should implement it.
        '''
        self.highlight_flag = False
        
        self.emit_redraw_request()
    
    def icon_item_button_press(self, x, y):
        '''
        Handle button-press event.
        
        This is IconView interface, you should implement it.
        '''
        pass        
    
    def icon_item_button_release(self, x, y):
        '''
        Handle button-release event.
        
        This is IconView interface, you should implement it.
        '''
        pass
    
    def icon_item_single_click(self, x, y):
        '''
        Handle single click event.
        
        This is IconView interface, you should implement it.
        '''
        pass

    def icon_item_double_click(self, x, y):
        '''
        Handle double click event.
        
        This is IconView interface, you should implement it.
        '''
        pass
    
    def icon_item_release_resource(self):
        '''
        Release item resource.

        If you have pixbuf in item, you should release memory resource like below code:

        >>> if self.pixbuf:
        >>>     del self.pixbuf
        >>>     self.pixbuf = None
        >>>
        >>> return True

        This is IconView interface, you should implement it.
        
        @return: Return True if do release work, otherwise return False.
        
        When this function return True, IconView will call function gc.collect() to release object to release memory.
        '''
        if self.pixbuf:
            del self.pixbuf
            self.pixbuf = None
        if self.mask_pixbuf:    
            del self.mask_pixbuf
            self.mask_pixbuf = None
            
        return True
    
class RadioListItem(TreeItem):
    
    def __init__(self, channel_info):
        TreeItem.__init__(self)
        self.index = 0
        self.column_index = 0
        self.side_padding = 5
        self.item_height = 55
        self.item_width = LIST_WIDTH
        self.is_highlight = False
        self.channel_info = channel_info
        cover_path = DoubanCover.get_cover(channel_info, try_web=False)
        if cover_path:
            self.normal_pixbuf = gtk.gdk.pixbuf_new_from_file(cover_path)
        else:    
            self.normal_pixbuf = app_theme.get_pixbuf("slide/default_cover.png").get_pixbuf()
        self.update_size()    
    
    @property
    def channel_id(self):
        return self.channel_info.get("id", "")
        
    def emit_redraw_request(self):
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
        
    def update_size(self):    
        self.channel_name = utils.xmlescape(self.channel_info.get("name", ""))
        __, self.name_h = get_content_size(self.channel_name, text_size=9)
        
        self.detail_info = "%s首歌曲 %s制作" % (self.channel_info.get("song_num"),
                                       utils.xmlescape(self.channel_info.get("creator", {}).get("name", "")))
        __, self.detail_h = get_content_size(self.detail_info, text_size=8)
    
    
    def render_content(self, cr, rect):
        if self.is_highlight:    
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "simpleItemHighlight")
        elif self.is_select:    
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "simpleItemSelect")
        elif self.is_hover:
            draw_single_mask(cr, rect.x + 1, rect.y, rect.width, rect.height, "simpleItemHover")
        
        if self.is_select or self.is_highlight:
            text_color = "#ffffff"
        else:    
            text_color = app_theme.get_color("labelText").get_color()
            
        icon_pixbuf = self.normal_pixbuf    
            
        icon_y = rect.y + (rect.height - icon_pixbuf.get_height()) / 2    
        padding_x = 10
        padding_y = 10
        draw_pixbuf(cr, self.normal_pixbuf, rect.x + padding_x, icon_y)
        
        draw_text(cr, self.channel_name, rect.x + icon_pixbuf.get_width() + padding_x * 2, 
                  rect.y + padding_y, rect.width - icon_pixbuf.get_width() - padding_x * 2, self.name_h, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT, text_size=9)    
        
        draw_text(cr, self.detail_info, rect.x + icon_pixbuf.get_width() + padding_x * 2, 
                  rect.y + (rect.height - self.detail_h - padding_y), 
                  rect.width - icon_pixbuf.get_width() - padding_x * 2, self.detail_h, 
                  text_color = text_color,
                  alignment=pango.ALIGN_LEFT, text_size=8)    
        
    def get_height(self):    
        return self.item_height
    
    def get_column_widths(self):
        return (self.item_width,)
    
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
    
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
    
    def select(self):    
        self.is_select = True
        self.emit_redraw_request()
        
    def highlight(self):    
        self.is_highlight = True
        # self.is_select = False
        self.emit_redraw_request()
        
    def unhighlight(self):    
        self.is_highlight = False
        self.emit_redraw_request()
        
    def get_column_renders(self):
        return (self.render_content,)
        
    def __hash__(self):
        return hash(self.channel_info.get("id"))
    
    def __repr__(self):
        return "<RadioItem %s>" % self.channel_info.get("id")
    
    def __cmp__(self, other_item):
        if not other_item:
            return -1
        try:
            return cmp(self.channel_info, other_item.channel_info)
        except AttributeError: return -1
        
    def __eq__(self, other_item):    
        try:
            return self.channel_info == other_item.channel_info
        except:
            return False
