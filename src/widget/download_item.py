#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2013 Deepin, Inc.
#               2011 ~ 2013 Hou Shaohui
# 
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou Shaohui <houshao55@gmail.com>
#             Zhai Xiang <zhaixiang@linuxdeepin.com>
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
from dtk.ui.treeview import TreeItem
from dtk.ui.threads import post_gui
from dtk.ui.utils import get_content_size
from dtk.ui.draw import draw_text, draw_pixbuf, draw_shadow
# from dtk.ui.button import CheckButtonBuffer
from dtk.ui.progressbar import ProgressBuffer

from download_manager import fetch_service, TaskObject
from pystorm.report import parse_bytes, parse_time
from widget.ui_utils import (draw_single_mask)
from widget.skin import app_theme
from xdg_support import get_song_save_path
from cover_manager import CoverManager

BUTTON_NORMAL = 1
BUTTON_HOVER = 2
BUTTON_PRESS = 3

class TaskItem(TreeItem):
    '''
    class docs
    '''
    
    STATUS_WAIT_DOWNLOAD = 2
    STATUS_IN_DOWNLOAD = 3
    STATUS_STOP = 4
    STATUS_FINISH = 5
    
    def __init__(self, song, finish_callback=None):
        '''
        init docs
        '''
        TreeItem.__init__(self)
        
        
        # Init sizes.
        self.item_height = 50        
        self.info_width = -1
        self.progressbar_width = 100
        self.progressbar_padding_x = 10
        self.progressbar_height = 12
        self.check_button_padding_x = 10
        self.info_padding_x = 5

        self.icon_pixbuf = None
        self.song = song        
        self.button_status = BUTTON_NORMAL
        
        # Init status.
        self.status = self.STATUS_WAIT_DOWNLOAD
        self.status_text = "等待下载"
        
        # Init buffers. 
        self.progress_buffer = ProgressBuffer()
        
        
        self.stop_pixbuf = app_theme.get_pixbuf("download/stop.png").get_pixbuf()
        self.stop_pixbuf_padding_x = 5
        self.block_width = 50
        self.download_task = TaskObject(song.get("uri"), get_song_save_path(song), output_temp=True)
        self.download_task.connect("update", self.download_update)
        self.download_task.connect("finish", self.download_finish)
        self.download_task.connect("error",  self.download_failed)
        self.download_task.connect("start",  self.download_start)
        
        self.finish_callback = finish_callback
        
        self.start_download()
        
    def start_download(self):    
        fetch_service.add_missions([self.download_task])
        
        
    def render_info(self, cr, rect):
        if self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
        if self.icon_pixbuf is None:
            self.icon_pixbuf = CoverManager.get_pixbuf_from_song(self.song, 37, 38, try_web=False)
            
            
        icon_width = self.icon_pixbuf.get_width()
        icon_height = self.icon_pixbuf.get_height()
        icon_x = rect.x + self.info_padding_x
        icon_y = rect.y + (rect.height - icon_height) / 2
        
        # Draw shadow.
        drop_shadow_padding = 3
        drop_shadow_radious = 3
        draw_shadow(
            cr,
            icon_x,
            icon_y,
            icon_width + drop_shadow_padding,
            icon_height + drop_shadow_padding,
            drop_shadow_radious,
            app_theme.get_shadow_color("window_shadow")
            )

        outside_shadow_padding = 2
        outside_shadow_radious = 3
        draw_shadow(
            cr,
            icon_x - outside_shadow_padding,
            icon_y - outside_shadow_padding,
            icon_width + outside_shadow_padding * 2,
            icon_height + outside_shadow_padding * 2,
            outside_shadow_radious,
            app_theme.get_shadow_color("window_shadow")
            )
        
        # Draw wallpaper.
        
        draw_pixbuf(cr, self.icon_pixbuf, icon_x, icon_y)
        rect.x = icon_x + self.icon_pixbuf.get_width() + self.info_padding_x
        rect.width -= self.info_padding_x * 2 - self.icon_pixbuf.get_width()
        _width, _height = get_content_size("%s" % self.song.get_str("title"))
        draw_text(cr, "<b>%s</b>" % self.song.get_str("title"), rect.x, icon_y, rect.width, _height,
                  text_size=10)                   
        
        rect.y = icon_y + icon_width - _height
        _width, _height = get_content_size(self.status_text)
        draw_text(cr, self.status_text, rect.x, rect.y, rect.width, _height)
        
    def render_progressbar(self, cr, rect):     
        if self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
       
        self.progress_buffer.render(cr, 
                                    gtk.gdk.Rectangle(rect.x + (rect.width - self.progressbar_width) / 2,
                                                      rect.y + (rect.height - self.progressbar_height)/ 2,
                                                      self.progressbar_width, self.progressbar_height))
        
    def render_stop(self, cr, rect):    
        if self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
        icon_x = rect.x + (rect.width - self.stop_pixbuf.get_width()) / 2
        icon_y = rect.y + (rect.height - self.stop_pixbuf.get_height()) / 2
        draw_pixbuf(cr, self.stop_pixbuf, icon_x, icon_y)
        
    def render_block(self, cr, rect):    
        if self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
            
    def get_column_widths(self):
        return [ 10,
                self.info_width, 
                self.progressbar_width + self.progressbar_padding_x * 2,
                self.stop_pixbuf.get_width() + self.stop_pixbuf_padding_x * 2,
                self.block_width
                ]
    
    
    def get_column_renders(self):
        return (self.render_block, self.render_info, self.render_progressbar, self.render_stop, self.render_block)
    
    def get_height(self):
        return self.item_height
    
    def emit_request_redraw(self):
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def unselect(self):
        pass
    
    def select(self):
        pass
    
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_request_redraw()

    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_request_redraw()
    
    # def motion_notify(self, column, offset_x, offset_y):
    #     if column == 0:
    #         if self.check_button_buffer.motion_button(offset_x, offset_y):
    #             self.emit_request_redraw()
    
    # def button_press(self, column, offset_x, offset_y):
    #     if column == 0:
    #         if self.check_button_buffer.press_button(offset_x, offset_y):
    #             self.emit_request_redraw()
                
    # def button_release(self, column, offset_x, offset_y):
    #     if column == 0 and self.check_button_buffer.release_button(offset_x, offset_y):
    #         self.emit_request_redraw()
                    
    def single_click(self, column, offset_x, offset_y):
        pass
        
    def double_click(self, column, offset_x, offset_y):
        pass        
    
    @post_gui        
    def download_update(self, obj, data):
        self.progress_buffer.progress = data.progress
        speed = parse_bytes(data.speed)
        remaining = parse_time(data.remaining)
        filesize = parse_bytes(data.filesize)
        downloaded = parse_bytes(data.downloaded)
        self.status_text = "%s/s - %s, 共%s,  还有 %s " % (speed, downloaded, filesize, remaining)
        self.emit_request_redraw()

    @post_gui        
    def download_finish(self, obj, data):
        self.progress_buffer.progress = 100
        self.status_text = "下载完成"
        self.emit_request_redraw()
        
        if self.finish_callback:
            self.finish_callback(self)
            
    @post_gui        
    def download_failed(self, obj, data):
        self.status_text = data
        self.emit_request_redraw()
        
    @post_gui    
    def download_start(self, obj, data):
        self.status_text = "开始下载"
        self.emit_request_redraw()

    def download_stop(self):
        pass
            
            
    def release_resource(self):
        '''
        Release item resource.

        If you have pixbuf in item, you should release memory resource like below code:

        >>> if self.pixbuf:
        >>>     del self.pixbuf
        >>>     self.pixbuf = None
        >>>
        >>> return True

        This is TreeView interface, you should implement it.
        
        @return: Return True if do release work, otherwise return False.
        
        When this function return True, TreeView will call function gc.collect() to release object to release memory.
        '''
        if self.icon_pixbuf:
            del self.icon_pixbuf
            self.icon_pixbuf = None

        return True    

