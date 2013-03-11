#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Hou Shaohui
# 
# Author:     Wang Yong <lazycat.manatee@gmail.com>
#             Hou Shaohui <houshao55@gmail.com>
#
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
import math

from dtk.ui.utils import (color_hex_to_cairo, get_content_size, 
                          alpha_color_hex_to_cairo, set_cursor)

from dtk.ui.draw import render_text, draw_pixbuf
from dtk.ui.timeline import Timeline, CURVE_SINE

from widget.skin import app_theme
from widget.ui_utils import is_in_rect
from cover_manager import DoubanCover
from helper import Dispatcher
import utils

class SlideSwitcher(gtk.EventBox):
    
    def __init__(self, channels=None):
        gtk.EventBox.__init__(self)
        # self.set_visible_window(False)
        self.set_size_request(-1, 200)
        self.channel_infos =[]        # Init signals.
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                        gtk.gdk.BUTTON_RELEASE_MASK |
                        gtk.gdk.POINTER_MOTION_MASK |
                        gtk.gdk.ENTER_NOTIFY_MASK |
                        gtk.gdk.LEAVE_NOTIFY_MASK
                        )
        
        self.connect("expose-event", self.on_expose_event)                
        self.connect("motion-notify-event", self.on_motion_notify)
        self.connect("leave-notify-event", self.on_leave_notify)
        self.connect("enter-notify-event", self.on_enter_notify)
        self.connect("button-press-event", self.on_button_press)
             
        # Init data.
        self.slide_number = 5
        self.active_index = 0
        self.active_alpha = 1.0
        self.target_alpha = 0.0
        self.target_index = None
        self.motion_index = None
        self.in_animiation = False
        self.auto_animiation_time = 2000
        self.hover_animation_time = 500
        self.auto_slide_timeout = 4500
        self.auto_slide_timeout_id = None
        self.pointer_radious = 8
        self.pointer_padding = 20
        self.pointer_offset_x = -105
        self.pointer_offset_y = 20
        self.pointer_coords = {}
        self.cover_size = 200

        self.default_cover = app_theme.get_pixbuf("radio/default_banner.png").get_pixbuf()
        self.mask_pixbuf = app_theme.get_pixbuf("radio/play_mask.png").get_pixbuf()
        self.prompt_text = "正在加载数据..."
        
        self.text_padding_x = 18
        self.text_start_y = 30
        self.text_interval_y = 16
        self.cover_pixbufs = {}
        self.mask_flag = False
        
    def __init_cover_pixbufs(self):
        for channel in self.channel_infos:
            self.cover_pixbufs[channel.get("id")] = self.get_channel_cover(channel)
            
    def update_channel_cover(self, channel):        
        self.cover_pixbufs[channel.get("id")] = self.get_channel_cover(channel, emit_fetch=False)
                
    def get_channel_cover(self, channel_info, emit_fetch=True):
        cover_path = DoubanCover.get_banner(channel_info, try_web=False)
        if cover_path:
            try:
                pixbuf = gtk.gdk.pixbuf_new_from_file(cover_path)
            except:    
                return self.default_cover
            else:
                return pixbuf
        else:
            if emit_fetch:
                self.start_fetch_cover(channel_info)
            return self.default_cover
        
    def fetch_channel_cover(self, channel_info):    
        cover_path = DoubanCover.get_banner(channel_info, try_web=True)
        if cover_path:
            self.update_channel_cover(channel_info)
        
    def start_fetch_cover(self, channel_info):    
        utils.ThreadFetch(
            fetch_funcs=(self.fetch_channel_cover, (channel_info,))
            ).start()
        
    def get_channel_pixbuf(self, channel_info):    
        channel_id = channel_info.get("id")
        if channel_id in self.cover_pixbufs.keys():
            return self.cover_pixbufs[channel_id]
        return self.default_cover
        
    def get_channel_contents(self, channel_info):    
        name = utils.xmlescape(channel_info.get("name", ""))
        intro = utils.xmlescape(channel_info.get("intro", ""))
        hot_songs = "%s: %s" % ("热门歌曲", utils.xmlescape(" / ".join(channel_info.get("hot_songs", []))))
        songs = "%s%s" % (str(channel_info.get("song_num", "0")), "首歌曲")
        return (name, intro, hot_songs, songs)
        
    def draw_channel_info(self, cr, allocation, index, alpha):    
        rect = gtk.gdk.Rectangle(*allocation)
        cr.push_group()
        
        # get channel_info
        channel_info = self.channel_infos[index]
        
        # get_pixbuf
        pixbuf = self.get_channel_pixbuf(channel_info)
        pixbuf_x = rect.x + (self.cover_size - pixbuf.get_width()) / 2
        pixbuf_y = rect.y + (self.cover_size - pixbuf.get_height()) / 2
        
        draw_pixbuf(cr, pixbuf, pixbuf_x, pixbuf_y)
        
        # get_contents
        text_contents = self.get_channel_contents(channel_info)
        
        pixbuf_width = self.cover_size
        cr.set_source_rgb(*color_hex_to_cairo("#EFF5F2"))
        cr.rectangle(rect.x + pixbuf_width, rect.y, rect.width - pixbuf_width, rect.height)
        cr.fill()
        
        text_x = rect.x + pixbuf_width + self.text_padding_x 
        text_y = rect.y + self.text_start_y
        text_width = rect.width - pixbuf_width - self.text_padding_x * 2
        
        for index, content in enumerate(text_contents):
            if index == 0:
                _width, _height = get_content_size(content, 12)                
                render_text(cr, content, text_x, text_y, text_width, _height, text_size=12, 
                            text_color="#79af8e")
                text_y += _height + self.text_interval_y
                
            elif index == 1 and content:
                _width, _height = get_content_size(content, 8)                
                _height = _height * 2
                cr.save()
                cr.rectangle(text_x, text_y, text_width, _height)
                cr.clip()
                render_text(cr, content, text_x, text_y, text_width, _height, text_size=8, 
                            text_color="#444444", wrap_width=text_width)
                cr.restore()
                
                text_y += _height + self.text_interval_y
                
            elif index == 2 and content:    
                _width, _height = get_content_size(content, 8)                
                _height = _height * 2
                cr.save()
                cr.rectangle(text_x, text_y, text_width, _height)
                cr.clip()
                render_text(cr, content, text_x, text_y, text_width, _height, text_size=8, 
                            text_color="#878787", wrap_width=text_width)
                cr.restore()

                text_y += _height + self.text_interval_y
                
            elif index == 3 and content:    
                _width, _height = get_content_size(content, 8)                
                _height = _height * 2
                cr.save()
                cr.rectangle(text_x, text_y, text_width, _height)
                cr.clip()
                render_text(cr, content, text_x, text_y, text_width, _height, text_size=8, 
                            text_color="#878787", wrap_width=text_width)
                cr.restore()
                
        cr.pop_group_to_source()
        cr.paint_with_alpha(alpha)
        
    def on_expose_event(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        rect.x = rect.y = 0
        
        cr.set_source_rgb(1, 1, 1)
        cr.rectangle(rect.x, rect.y, rect.width,rect.height)
        cr.fill()
        
        if not self.channel_infos:
            render_text(cr, self.prompt_text, rect.x, rect.y, rect.width, rect.height,
                        text_color=app_theme.get_color("labelText").get_color(),
                        alignment=pango.ALIGN_CENTER)
            return True
            
        
        if self.active_alpha > 0.0:
            self.draw_channel_info(cr, rect, self.active_index, self.active_alpha)
            
        if self.target_index != None and self.target_alpha > 0.0:    
            self.draw_channel_info(cr, rect, self.target_index, self.target_alpha)
            
        if self.mask_flag:    
            draw_pixbuf(cr, self.mask_pixbuf, rect.x, rect.y)
        
        
        # Draw select pointer.
        for index in range(0, self.slide_number):
            if self.target_index == None:
                if self.active_index == index:
                    cr.set_source_rgba(*alpha_color_hex_to_cairo(("#9DD6C5", 0.9)))                    
                else:
                    cr.set_source_rgba(*alpha_color_hex_to_cairo(("#D4E1DC", 0.9)))
            else:
                if self.target_index == index:
                    cr.set_source_rgba(*alpha_color_hex_to_cairo(("#9DD6C5", 0.9)))                    
                else:
                    cr.set_source_rgba(*alpha_color_hex_to_cairo(("#D4E1DC", 0.9)))

            p_x =  rect.x + rect.width + self.pointer_offset_x + index * self.pointer_padding       
            p_y =  rect.y + self.pointer_offset_y
            cr.arc(p_x, p_y,
                   self.pointer_radious,
                   0, 
                   2 * math.pi)
            
            cr.fill()
            
            pointer_rect = gtk.gdk.Rectangle(p_x - self.pointer_radious,
                                             p_y - self.pointer_radious,
                                             self.pointer_radious * 2,
                                             self.pointer_radious * 2)
            # Draw index number.
            render_text(cr, str(index + 1), pointer_rect.x, pointer_rect.y,
                        pointer_rect.width, pointer_rect.height, text_size=9, text_color="#444444",
                        alignment=pango.ALIGN_CENTER
                        )
            pointer_rect.x -= rect.x
            pointer_rect.y -= rect.y
            self.pointer_coords[index] = pointer_rect
            
        return True
            
    def start_animation(self, animiation_time, target_index=None):        
        if target_index == None:
            if self.active_index >= self.slide_number - 1:
                target_index = 0
            else:    
                target_index = self.active_index + 1
                
        if not self.in_animiation:        
            self.in_animiation = True
            self.target_index = target_index
            timeline = Timeline(animiation_time, CURVE_SINE)
            timeline.connect("update", self.update_animation)
            timeline.connect("completed", lambda source: self.completed_animation(source, target_index))
            timeline.run()
        return True    
    
    def start_auto_slide(self):
        self.auto_slide_timeout_id = gtk.timeout_add(self.auto_slide_timeout, lambda : self.start_animation(self.auto_slide_timeout))
        
    def update_animation(self, source, status):   
        self.active_alpha = 1.0 - status
        self.target_alpha = status
        self.queue_draw()
        
    def completed_animation(self, source, index):    
        self.active_index = index
        self.active_alpha = 1.0
        self.target_index = None
        self.target_alpha = 0.0
        self.in_animiation = False
        self.queue_draw()
        
        # Start new animiation when cursor at new index when animiation completed.
        if self.motion_index:
            if self.active_index != self.motion_index:
                self.start_animation(self.hover_animation_time, self.motion_index)
        
    def handle_animation(self, widget, event):    
        self.motion_index = None
        
        for index, rect in self.pointer_coords.items():
            if rect.x <= event.x <= rect.x + rect.width and rect.y <= event.y <= rect.y + rect.height:
                # set_cursor(widget, gtk.gdk.HAND2)
                self.motion_index = index
                if self.active_index != index:
                    self.start_animation(self.hover_animation_time, index)
                break    
        # else:    
        #     set_cursor(widget, None)
            
            
    def on_motion_notify(self, widget, event):        
        self.handle_animation(widget, event)
        
        pixbuf_rect = gtk.gdk.Rectangle(15, 15, 170, 170)
        
        if self.channel_infos:
            if is_in_rect((event.x, event.y), pixbuf_rect):
                new_mask_flag = True
                set_cursor(widget, gtk.gdk.HAND2)
            else:    
                new_mask_flag = False
                set_cursor(widget, None)
                
            if new_mask_flag != self.mask_flag:    
                self.mask_flag = new_mask_flag
                self.queue_draw()    
        

    def on_leave_notify(self, widget, event):    
        self.start_auto_slide()
        self.mask_flag = False
        self.queue_draw()
        set_cursor(widget, None)    
    
    def on_enter_notify(self, widget, event):
        if self.auto_slide_timeout_id is not None:
            gobject.source_remove(self.auto_slide_timeout_id)
            self.auto_slide_timeout_id = None
            
    def on_button_press(self, widget, event):        
        if self.mask_flag:
            Dispatcher.emit("play-radio", self.channel_infos[self.active_index])
            
            
    def set_infos(self, channel_infos):        
        self.channel_infos = channel_infos
        self.__init_cover_pixbufs()
        self.start_auto_slide()
        
gobject.type_register(SlideSwitcher)
