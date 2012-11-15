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
import cairo
import pangocairo
import pango
import gobject

from dtk.ui.utils import color_hex_to_cairo, get_content_size
from dtk.ui.draw import render_text
from dtk.ui.timeline import Timeline, CURVE_SINE

from widget.skin import app_theme

class SlideSwitcher(gtk.EventBox):
    
    def __init__(self):
        gtk.EventBox.__init__(self)
        self.set_visible_window(False)
        self.set_size_request(-1, 200)
        
        # Generate image and surfaces.
        self.text_contents = [("中国好声音", 
                               "曾经我暗恋着一个人，挣扎了许久后告白，却还是被拒绝。也是我的心情吧，谁知道接下来会发生什么", 
                               "热门歌曲：我的歌声里 / 洋葱 / 爱要坦荡荡", "210首歌曲 豆瓣FM制作"),
                              ("心灵后花园MHz", "在我的心灵后花园里，寻找属于你自己的心灵奇葩。每天更新，挺舒服的歌",
                               "热门歌曲：Halo / Lemon Tree (中文版) / Way Back Into Love", "432首歌曲 拯救予逍遥制作"),
                              ("偷偷塞进耳朵里MHz", "还记得那些年，在课堂上偷偷将耳机塞进耳朵里听得歌么？",
                               "热门歌曲：Right Here Waiting / Kiss The Rain / Hotel California", "168首歌曲 lancelot制作"),
                              ("聽說 Hear MeMHz", "握住你的心 紧紧紧紧的", "热门歌曲：寂寞寂寞就好 / 原谅 / 还是会", "392首歌曲 绿制作"),
                              ("正·能·量MHz", "無聊了就來聽聽歌吧。", 
                               "热门歌曲：Love The Way You Lie (Part II) feat. Eminem / Nothin' On You / Whistle",
                               "748首歌曲 绿绿桑制作")]    
        self.images = []
        for i in range(5):
            self.images.append(app_theme.get_theme_file_path("image/slide/%d.png" % i))
        self.generate_image_surfaces()     
        self.content_surfaces = None
        
        # Init signals.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        # self.connect("motion-notify-event", self.on_motion_notify)
        # self.connect("leave-notify-event", self.on_leave_notify)
        # self.connect("enter-notify-event", self.on_enter_notify)
        self.connect("expose-event", self.on_expose_event)        
        self.connect("size-allocate", self.on_size_allocate)
        
        
        # Init data.
        self.slide_number = 5
        self.active_index = 0
        self.active_alpha = 1.0
        self.target_alpha = 0.0
        self.target_index = None
        self.in_animiation = False
        self.animiation_time = 3000
        self.auto_slide_timeout = 4000
        self.auto_slide_timeout_id = None
        self.start_auto_slide()
            
    def generate_image_surfaces(self):
        self.image_surfaces = []
        for image_file in self.images:
            self.image_surfaces.append(cairo.ImageSurface.create_from_png(image_file))
            
    def generate_content_surfaces(self, rect):        
        self.content_surfaces = []
        for i, image_surface in enumerate(self.image_surfaces):
            self.content_surfaces.append(self.get_draw_surface(image_surface, self.text_contents[i], rect.width, 200))
            
    def on_size_allocate(self, widget, rect):        
        del self.content_surfaces[:]
        self.content_surfaces = None
        
    def get_draw_surface(self, image_surface, text_contents, width, height):    
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, int(width), int(height))
        cr = cairo.Context(surface)
        cr.set_source_surface(image_surface, 0, 0)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()
        
        image_w, image_h = image_surface.get_width(), image_surface.get_height()
        cr.set_source_rgb(*color_hex_to_cairo("#EFF5F2"))
        cr.rectangle(image_w, 0, width, height)
        cr.fill()
        
        t_x, t_y = image_w + 18, 0
        t_w = width - image_w - 18 * 2
        
        t_y += 16
        for index, content in enumerate(text_contents):
            if index == 0:
                c_w, c_h = get_content_size(content, 12)                
                render_text(cr, content, t_x, t_y, t_w, t_y + c_h, text_size=12, 
                            text_color="#2cc96f")
                t_y += c_h * 2
                t_y += 16
            elif index == 1 and content:
                c_w, c_h = get_content_size(content, 8)                
                render_text(cr, content, t_x, t_y, t_w, c_h * 2, text_size=8, 
                            text_color="#444444", wrap_width=t_w)
                t_y += c_h * 2
                t_y += 16
                
            elif index == 2 and content:    
                c_w, c_h = get_content_size(content, 8)                
                render_text(cr, content, t_x, t_y, t_w, c_h * 2, text_size=8, 
                            text_color="#878787", wrap_width=t_w)
                t_y += c_h * 2
                t_y += 5
                
            elif index == 3 and content:    
                c_w, c_h = get_content_size(content, 8)                
                render_text(cr, content, t_x, t_y, t_w, c_h * 2, text_size=8, 
                            text_color="#878787", wrap_width=t_w)
                t_y += c_h * 2
        return surface        
        
    def on_expose_event(self, widget, event):    

            
        cr = widget.window.cairo_create()
        rect = widget.allocation
        if self.content_surfaces == None:        
            self.generate_content_surfaces(rect)
        
        cr.set_source_rgb(1, 1, 1)
        cr.rectangle(rect.x, rect.y, rect.width,rect.height)
        cr.fill()
        
        cr.save()
        cr.set_operator(cairo.OPERATOR_OVER)
        
        if self.active_alpha > 0.0:
            cr.save()
            cr.set_source_surface(self.content_surfaces[self.active_index], rect.x ,rect.y)
            cr.paint_with_alpha(self.active_alpha)
            cr.restore()
            
        if self.target_index != None and self.target_alpha > 0.0:    
            cr.save()
            cr.set_source_surface(self.content_surfaces[self.target_index], rect.x, rect.y)
            cr.paint_with_alpha(self.target_alpha)
            cr.restore()
        cr.restore()    
        return True    
            
    def start_animation(self, index=None):        
        
        if index == None:
            if self.active_index >= self.slide_number - 1:
                index = 0
            else:    
                index = self.active_index + 1
                
        if not self.in_animiation:        
            self.in_animiation = True
            self.target_index = index
            
            timeline = Timeline(self.animiation_time, CURVE_SINE)
            timeline.connect("update", self.update_animation)
            timeline.connect("completed", lambda source: self.completed_animation(source, index))
            timeline.run()
            
        return True    
    
    def start_auto_slide(self):
        self.auto_slide_timeout_id = gtk.timeout_add(self.auto_slide_timeout, lambda : self.start_animation())
        
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
