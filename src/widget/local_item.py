#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Hou ShaoHui
# 
# Author:     Hou ShaoHui <houshao55@gmail.com>
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

from dtk.ui.draw import draw_pixbuf
# from dtk.ui.thread_pool import MissionThread

from widget.skin import app_theme
from widget.ui_utils import (render_text)
from cover_manager import CoverManager
from pinyin import TransforDB
from nls import _

import utils

class LocalItem(gobject.GObject):
    
    __gsignals__ = { "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),}
    
    def __init__(self, _tuple):
        
        # Init gobject.
        gobject.GObject.__init__(self)
        # MissionThread.__init__(self)
        
        # Get tuple values.
        self.key_name, self.value_name, nums, self.tag = _tuple
        
        # Set cell size.
        self.cell_width = 84                
        if self.tag == "folder": self.cell_width = 67
        
        # Set display label.
        if not self.key_name:
            self.name_label= _("Unknown")
        elif self.key_name == "deepin-all-songs":    
            self.name_label = _("All Tracks")
        else:    
            self.name_label = utils.xmlescape(self.key_name)
        
        # Just create pixbuf when need render it to save memory.
        self.pixbuf = None
        
        # Display track number 
        self.labels = "%d %s" % (nums, _("Track(s)"))
        
        self.pixbuf_offset_x = 4
        self.pixbuf_offset_y = 4
        self.padding_x = 6
        self.border_size = 4
        self.padding_y = 20
        self.hover_flag = False
        self.highlight_flag = False
        self.draw_side_flag = True        
        self.__draw_play_hover_flag = False
        self.__draw_play_press_flag = False
        
        self.cover_dpixbuf = app_theme.get_pixbuf("cover/default_cover.png")
        
        # normal side pixbuf
        if self.tag == "folder":
            self.__normal_side_pixbuf = app_theme.get_pixbuf("local/side_normal.png").get_pixbuf()
        else:    
            self.__normal_side_pixbuf =  app_theme.get_pixbuf("filter/side_normal.png").get_pixbuf()
            
        # normal play pixbuf
        self.__normal_play_pixbuf =  app_theme.get_pixbuf("filter/play_normal.png").get_pixbuf()
        
        self.play_rect = gtk.gdk.Rectangle(
            self.__normal_side_pixbuf.get_width() - self.__normal_play_pixbuf.get_width() - 2 - 6,
            self.__normal_side_pixbuf.get_height() - self.__normal_play_pixbuf.get_height() - 2 - 6,
            self.__normal_play_pixbuf.get_width(),
            self.__normal_play_pixbuf.get_height()
            )
        
        try:
            self.retrieve = TransforDB.convert(self.name_label.lower().replace(" ", "")) \
                + self.name_label.lower().replace(" ", "")
        except:    
            self.retrieve = ""
        
    def create_pixbuf(self):
        try:
            if self.pixbuf:
                del self.pixbuf
        except: pass        
            
        if not self.key_name:
            if self.tag == "genre":
                self.pixbuf = CoverManager.get_pixbuf_from_genre(self.name_label)
            else:    
                self.pixbuf = CoverManager.get_pixbuf_from_name(self.name_label, self.cell_width, self.cell_width,
                                                                return_default=False)            
            
        elif self.key_name == "deepin-all-songs":    
            # self.pixbuf = CoverManager.get_all_song_cover(self.cell_width, self.cell_width)
            pixbuf = CoverManager.get_combo_all_cover(self.tag)
            if pixbuf:
                self.pixbuf = pixbuf
            else:    
                self.pixbuf = app_theme.get_pixbuf("cover/all_song.png").get_pixbuf()
            # self.draw_side_flag = False
        else:    
            if self.tag == "genre":
                self.pixbuf = CoverManager.get_pixbuf_from_genre(self.name_label)
            elif self.tag == "album":    
                self.pixbuf = CoverManager.get_pixbuf_from_name("%s-%s" % (self.value_name, self.key_name), 
                                                                self.cell_width, self.cell_width,
                                                                return_default=False)            
            elif self.tag == "folder":
                   self.pixbuf = app_theme.get_pixbuf("local/music.png").get_pixbuf()
            else:    
                self.pixbuf = CoverManager.get_pixbuf_from_name(self.key_name, self.cell_width, self.cell_width,
                                                                return_default=False)
        
    def pointer_in_play_rect(self, x, y):    
        if self.play_rect.x < x < self.play_rect.x + self.play_rect.width and  \
                self.play_rect.y < y < self.play_rect.y + self.play_rect.height:
            return True
        else:
            return False
        
    def emit_redraw_request(self):    
        self.emit("redraw-request")
        
    def get_width(self):    
        return self.__normal_side_pixbuf.get_width() + self.padding_x * 2
    
    def get_height(self):
        return self.__normal_side_pixbuf.get_height() + self.padding_y * 2
    
    def render(self, cr, rect):
        # Create pixbuf resource if self.pixbuf is None.
        self.create_pixbuf()
            
        if not self.pixbuf:
            self.pixbuf = self.cover_dpixbuf.get_pixbuf()
            
        pixbuf_x =  rect.x + (rect.width - self.__normal_side_pixbuf.get_width()) / 2
            
        # Draw cover.
        if self.tag == "folder":    
            draw_pixbuf(cr, self.pixbuf, 
                        pixbuf_x,
                        rect.y)
        else:    
            pixbuf_rect = gtk.gdk.Rectangle(
                pixbuf_x + self.pixbuf_offset_x,
                rect.y + self.pixbuf_offset_y, 
                self.pixbuf.get_width(),
                self.pixbuf.get_height())
            
            draw_pixbuf(cr, self.pixbuf, 
                        pixbuf_x + self.pixbuf_offset_x,
                        rect.y + self.pixbuf_offset_y)
            
        
        if self.hover_flag or self.highlight_flag:
            if self.tag == "folder":
                hover_side_pixbuf = app_theme.get_pixbuf("local/side_hover.png").get_pixbuf()
            else:    
                hover_side_pixbuf = app_theme.get_pixbuf("filter/side_hover.png").get_pixbuf()
                
            draw_pixbuf(cr, hover_side_pixbuf, pixbuf_x, rect.y )            
        else:    
            if self.draw_side_flag:
                draw_pixbuf(cr, self.__normal_side_pixbuf, pixbuf_x, rect.y )            
                
        
        if self.hover_flag:
            # if self.tag != "folder" and self.draw_side_flag:
            #     cr.set_source_rgba(0, 0, 0, 0.3)
            #     cr.rectangle(*pixbuf_rect)
            #     cr.fill()
            
            if self.__draw_play_hover_flag:
                play_pixbuf = app_theme.get_pixbuf("filter/play_hover.png").get_pixbuf()
            elif self.__draw_play_press_flag:    
                play_pixbuf = app_theme.get_pixbuf("filter/play_press.png").get_pixbuf()
            else:    
                play_pixbuf = self.__normal_play_pixbuf
            draw_pixbuf(cr, play_pixbuf, pixbuf_x + self.play_rect.x, rect.y + self.play_rect.y)        
            
            
        # Draw text.    
        name_rect = gtk.gdk.Rectangle(rect.x + self.padding_x , 
                                      rect.y + self.__normal_side_pixbuf.get_height() + 5,
                                      self.cell_width, 11)
        num_rect = gtk.gdk.Rectangle(name_rect.x, name_rect.y + 16, name_rect.width, 9)
        
        render_text(cr, self.name_label, name_rect, 
                    app_theme.get_color("labelText").get_color(),
                    10)
        render_text(cr, self.labels, num_rect, 
                    app_theme.get_color("labelText").get_color(),
                    8)
        
    def icon_item_motion_notify(self, x, y):    
        self.hover_flag = True
        if self.pointer_in_play_rect(x, y):
            if self.__draw_play_press_flag:
                self.__draw_play_hover_flag =  False
            else:    
                self.__draw_play_hover_flag= True
        else:    
            self.__draw_play_hover_flag = False
            
        self.emit_redraw_request()
        
    def icon_item_lost_focus(self):    
        self.__draw_play_flag = False
        self.hover_flag = False
        self.emit_redraw_request()
        
    def icon_item_highlight(self):    
        self.highlight_flag = True
        self.emit_redraw_request()
        
    def icon_item_normal(self):    
        self.highlight_flag = False
        self.emit_redraw_request()
        
    def icon_item_button_press(self, x, y):    
        if self.pointer_in_play_rect(x, y):
            self.__draw_play_hover_flag =  False
            self.__draw_play_press_flag = True
        else:    
            self.__draw_play_press_flag = False
        self.emit_redraw_request()
    
    def icon_item_button_release(self, x, y):
        self.__draw_play_press_flag = False
        self.emit_redraw_request()
    
    def icon_item_single_click(self, x, y):
        pass
    
    def icon_item_double_click(self, x, y):
        pass
    
    def icon_item_release_resource(self):
        # Release pixbuf resource.
        del self.pixbuf
        self.pixbuf = None
        
        # Return True to tell IconView call gc.collect() to release memory resource.
        return True
    
    def change_cover_pixbuf(self, new_path):
        cover_name = None
        if self.tag == "album":
            cover_name = "%s-%s" % (self.value_name, self.key_name)
        elif self.tag == "artist":    
            cover_name = self.key_name
        if cover_name:    
            CoverManager.change_cover(cover_name, new_path)            
            
        
gobject.type_register(LocalItem)        
