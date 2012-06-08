#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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
import gio

from dtk.ui.label import Label
from dtk.ui.window import Window
from dtk.ui.button import Button
from dtk.ui.utils import move_window
from dtk.ui.line import draw_vlinear
from dtk.ui.entry import TextEntry
from dtk.ui.tab_window import TabBox


from widget.ui_utils import create_separator_box, create_right_align
from widget.skin import app_theme
from cover_manager import CoverManager
from lrc_manager import LrcManager

class SongInfo(gtk.VBox):
    def __init__(self, song):
        super(SongInfo, self).__init__()
        self.song = song        
        self.set_spacing(10)
        block_box = gtk.EventBox()
        block_box.set_visible_window(False)
        block_box.set_size_request(-1, 10)
        self.detail_table = gtk.Table(5, 4)
        self.detail_table.set_col_spacing(2, 20)
        self.detail_table.set_row_spacings(5)
        self.create_detail_box()
        self.pack_start(block_box, False, True)
        self.pack_start(self.create_simpler_box(), False, True)
        self.pack_start(create_separator_box(), False, True)
        self.pack_start(self.detail_table, False, True)
        self.pack_start(create_separator_box(), False, True)
        self.pack_start(self.create_location_box(), False, True)
        self.connect("expose-event", self.expose_mask_cb)

        
    def create_simpler_box(self):    
        simpler_box = gtk.HBox(spacing=10)
        self.cover_image = gtk.Image()
        self.cover_image.set_size_request(92, 92)
        self.cover_image.set_from_pixbuf(CoverManager.get_pixbuf_from_song(self.song, 92, 92))
        

        content_box = gtk.VBox(spacing=5)
        content_box_align = gtk.Alignment()        
        content_box_align.set(0.5, 0.5, 1.0, 0.0)
        content_box_align.add(content_box)
        title_box = self.create_combo_label("歌名:", self.song.get_str("title"))
        content_box.pack_start(title_box, False, False)
        artist_box = self.create_combo_label("歌手:", self.song.get_str("artist"))
        content_box.pack_start(artist_box, False, False)
        album_box  = self.create_combo_label("专辑:", self.song.get_str("album"))
        content_box.pack_start(album_box, False, False)
        simpler_box.pack_start(self.cover_image)
        simpler_box.pack_start(content_box_align)
        return simpler_box

    def create_combo_label(self, title, content):   
        hbox = gtk.HBox(spacing=5)
        title_label  = Label(title)
        content_label = Label(content)
        hbox.pack_start(title_label, False, False)
        hbox.pack_start(content_label, False, False)
        return hbox
    
    def create_detail_box(self):
        self.create_double_combo_label(0, 1, "文件类型:", "文件格式:", self.get_song_attr(), self.get_song_attr(False))
        self.create_double_combo_label(1, 2, "文件大小:", "歌曲时长:", self.song.get_str("#size"), self.song.get_str("#duration"))
        self.create_double_combo_label(2, 3, "比特率:", "采样速率:", self.song.get_str("#bitrate"), self.song.get_str("#rate"))
        self.create_double_combo_label(3, 4, "添加时间:", "最近播放时间:", self.song.get_str("#added"), self.song.get_str("#lastplayed"))
        self.create_double_combo_label(4, 5, "播放次数:", "年代:", self.song.get_str("#playcount"), self.song.get_str("date"))
        
    def get_song_attr(self, song_type=True):    
        gio_file = gio.File(self.song.get_path())
        gio_file_info = gio_file.query_info(",".join([gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                                      gio.FILE_ATTRIBUTE_STANDARD_TYPE, 
                                                      gio.FILE_ATTRIBUTE_STANDARD_NAME,
                                                      gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                                                      gio.FILE_ATTRIBUTE_STANDARD_SIZE,
                                                      gio.FILE_ATTRIBUTE_STANDARD_ICON,
                                                      gio.FILE_ATTRIBUTE_TIME_MODIFIED,
                                                      gio.FILE_ATTRIBUTE_TIME_CHANGED,]))
        
        info_attr = gio_file_info.get_attribute_as_string(gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE)                
        
        if song_type:
            return gio.content_type_get_description(info_attr)
        else:
            return info_attr
        
        
        
    def create_double_combo_label(self, top_attach, bottom_attach, first_label, second_label, first_content="", second_content=""):
        first_label_box = gtk.HBox()
        first_title_label = Label(first_label)
        first_label_box.pack_start(create_right_align(), True, True)        
        first_label_box.pack_start(first_title_label, False, False)
        first_content_label = Label(first_content)
        
        second_label_box = gtk.HBox()
        second_title_label = Label(second_label)
        second_label_box.pack_start(create_right_align(), True, True)
        second_label_box.pack_start(second_title_label, False, False)
        second_content_label = Label(second_content)

        self.detail_table.attach(first_label_box, 0, 1, top_attach, bottom_attach)
        self.detail_table.attach(first_content_label, 1, 2, top_attach, bottom_attach, xpadding=5)
        self.detail_table.attach(second_label_box, 2, 3, top_attach, bottom_attach)
        self.detail_table.attach(second_content_label, 3, 4, top_attach, bottom_attach)
    
    def create_location_box(self):
        location_align = gtk.Alignment()
        location_align.set(1.0, 1.0, 0.5, 0.5)
        location_box = gtk.HBox(spacing=5)
        location_label = Label("文件位置:")
        location_entry = TextEntry(self.song.get_path())
        location_entry.set_size(240, 25)
        open_button = Button("打开目录")
        location_box.pack_start(location_label, False, True)
        location_box.pack_start(location_entry, False, True)
        location_box.pack_start(open_button, False, True)
        location_align.add(location_box)
        return location_align
    
    def expose_mask_cb(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
    
    
class InfoSetting(gtk.VBox):    
    
    def __init__(self, song):
        super(InfoSetting, self).__init__()
        self.song = song
        main_align = gtk.Alignment()
        main_align.set_padding(20, 0, 50, 0)
        main_align.set(0, 0, 0.5, 0.5)
        self.main_table = gtk.Table(6, 3)
        self.main_table.set_col_spacings(10)
        self.main_table.set_row_spacings(10)
        self.create_combo_entry(0, 1, "歌名:", song.get_str("title"))
        self.create_combo_entry(1, 2, "歌手:", song.get_str("artist"))
        self.create_combo_entry(2, 3, "专辑:", song.get_str("album"))
        self.create_combo_entry(3, 4, "流派:", song.get_str("genre"))
        self.create_combo_entry(4, 5, "年代:", song.get_str("date"))
        self.create_combo_entry(5, 6, "歌词:", LrcManager().get_lrc(song), Button("更改"))
        self.connect("expose-event", self.expose_mask_cb)
        main_align.add(self.main_table)
        self.pack_start(main_align, False, True)
        
    def create_combo_entry(self, top_attach, bottom_attach, label_text, content_text, button=False):   
        title_label_box = gtk.HBox()
        title_label = Label(label_text)
        title_label_box.pack_start(create_right_align(), False, True)
        title_label_box.pack_start(title_label, False, True)
        
        content_entry = TextEntry(content_text)
        content_entry.set_size(260, 25)
        self.main_table.attach(title_label_box, 0, 1, top_attach, bottom_attach, xoptions=gtk.FILL)
        self.main_table.attach(content_entry, 1, 2, top_attach, bottom_attach, xoptions=gtk.FILL)
        if button:
            self.main_table.attach(button, 2, 3, top_attach, bottom_attach, xoptions=gtk.FILL)
        return content_entry
        
        
    def expose_mask_cb(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
    
class CoverSetting(gtk.VBox):
    
    def __init__(self, song):
        super(CoverSetting, self).__init__()
        
        cover_box = gtk.VBox()
        cover_image_align =  gtk.Alignment()
        cover_image_align.set(0.5, 0.5, 0.5, 0.5)
        self.cover_image = gtk.Image()
        self.cover_image.set_size_request(300, 180)
        
        # Load cover pixbuf.
        song_cover_pixbuf = CoverManager.get_pixbuf_from_song(song, 300, 180)
        self.cover_image.set_from_pixbuf(song_cover_pixbuf)
        
        cover_image_align.add(self.cover_image)
        cover_box.add(cover_image_align)
        cover_box.set_size_request(400, 220)
        cover_box.connect("expose-event", self.expose_mask_cb)
        
        cover_box_align = gtk.Alignment()
        cover_box_align.set_padding(20, 20, 10, 10)
        cover_box_align.set(0.5, 0.5, 0.5, 0.5)
        cover_box_align.add(cover_box)
        
        button_box = gtk.HBox(spacing=10)
        button_box_align = gtk.Alignment()
        button_box_align.set_padding(0, 0, 0, 30)
        button_box_align.set(0, 0, 1.0, 1.0)
        button_box_align.add(button_box)
        self.change_button = Button("更改")
        self.delete_button = Button("删除")
        button_box.pack_start(create_right_align(), True, True)
        button_box.pack_start(self.change_button, False, False)
        button_box.pack_start(self.delete_button, False, False)
        
        self.pack_start(cover_box_align, False, True)
        self.pack_start(button_box_align, False, True)
        
    def expose_mask_cb(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
        
    
class SongEditor(Window):    
    
    def __init__(self, song):
        super(SongEditor, self).__init__()
        self.set_modal(True)
        self.set_keep_above(True)
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_size_request(500, 430)
        titlebar = gtk.EventBox()
        titlebar.set_visible_window(False)
        titlebar.set_size_request(-1, 32)
        titlebar.connect("button-press-event", lambda w, e: move_window(w, e, self))
        statusbar = gtk.EventBox()
        statusbar.set_visible_window(False)
        statusbar.set_size_request(-1, 50)
        
        button_align = gtk.Alignment()
        button_align.set(1.0, 0.5, 0, 0)
        button_align.set_padding(10, 10, 5, 10)
        button_box = gtk.HBox()
        
        button_align.add(button_box)
        save_button = Button("保存")
        save_button.connect("clicked", self.click_save_button)
        cancel_button = Button("取消")
        cancel_button.connect("clicked", self.click_cancel_button)
        
        button_box.pack_start(save_button, False, True, 5)
        button_box.pack_start(cancel_button, False, True, 5)
        
        statusbar.add(button_align)
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(0, 0, 2, 2)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)
        self.window_frame.pack_start(titlebar, False, True)
        self.window_frame.pack_start(main_align, True, True)
        self.window_frame.pack_start(statusbar, False, True)
        self.main_box.connect("expose-event", self.expose_mask_cb)
        
        self.tab_box = TabBox()
        self.tab_box.add_items([("歌曲信息", SongInfo(song)), ("信息设置", InfoSetting(song)), ("封面设置", CoverSetting(song))])
        self.main_box.pack_start(self.tab_box)
        
    def expose_mask_cb(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
    
    def click_save_button(self, widget):        
        self.destroy()
        
    def click_cancel_button(self, widget):    
        self.destroy()
    
