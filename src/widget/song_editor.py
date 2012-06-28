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
from dtk.ui.line import draw_vlinear
from dtk.ui.entry import TextEntry
from dtk.ui.tab_window import TabBox
from dtk.ui.titlebar import Titlebar


from widget.ui_utils import create_separator_box, create_right_align
from widget.skin import app_theme
from cover_manager import CoverManager
from library import MediaDB
from widget.dialog import WinFile

class SongInfo(gtk.VBox):
    def __init__(self, song=None):
        super(SongInfo, self).__init__()
        self.set_spacing(10)
        block_box = gtk.EventBox()
        block_box.set_visible_window(False)
        block_box.set_size_request(-1, 10)
        self.detail_table = gtk.Table(5, 4)
        self.detail_table.set_row_spacings(5)
        self.create_detail_box()
        self.pack_start(block_box, False, True)
        self.pack_start(self.create_simpler_box(), False, True)
        self.pack_start(create_separator_box(), False, True)
        self.pack_start(self.detail_table, False, True)
        self.pack_start(create_separator_box(), False, True)
        self.pack_start(self.create_location_box(), False, True)
        
        # update_song.
        if song:
            self.update_song(song)
            
        self.connect("expose-event", self.expose_mask_cb)
        
    def create_simpler_box(self):    
        simpler_box = gtk.HBox(spacing=30)
        cover_image_align = gtk.Alignment()
        cover_image_align.set_padding(0, 0, 50, 0)
        self.cover_image = gtk.Image()
        self.cover_image.set_size_request(92, 92)
        cover_image_align.add(self.cover_image)
        
        content_box_align = gtk.Alignment()
        content_box_align.set_padding(20, 0, 0, 0)
        content_box = gtk.VBox(spacing=5)
        title_box, self.title_label = self.create_combo_label("歌名:")
        artist_box, self.artist_label = self.create_combo_label("歌手:")
        album_box, self.album_label  = self.create_combo_label("专辑:")        
        content_box_align.add(content_box)
        
        content_box.pack_start(title_box, False, False)        
        content_box.pack_start(artist_box, False, False)
        content_box.pack_start(album_box, False, False)        

        simpler_box.pack_start(cover_image_align, False, False)
        simpler_box.pack_start(content_box_align, False, False)

        return simpler_box

    def create_combo_label(self, title, content=""):   
        hbox = gtk.HBox(spacing=5)
        title_label  = Label(title)
        content_label = Label(content)
        content_label.set_size_request(200, -1)
        hbox.pack_start(title_label, False, False)
        hbox.pack_start(content_label, False, False)
        return hbox, content_label
    
    def create_detail_box(self):
        self.file_type_label, self.file_format_label = self.create_double_combo_label(0, 1, "文件类型:", "文件格式:")
        self.file_size_label, self.song_duration_label = self.create_double_combo_label(1, 2, "文件大小:", "歌曲时长:")
        self.song_bitrate_label, self.song_rate_label = self.create_double_combo_label(2, 3, "比特率:", "采样速率:")
        self.song_added_label, self.song_lasted_label = self.create_double_combo_label(3, 4, "添加时间:", "最近播放时间:")
        self.song_playcount_label, self.song_date_label = self.create_double_combo_label(4, 5, "播放次数:", "年代:")
        
    def get_song_attr(self, song, song_type=True):    
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
        self.detail_table.attach(second_label_box, 2, 3, top_attach, bottom_attach, xpadding=5)
        self.detail_table.attach(second_content_label, 3, 4, top_attach, bottom_attach)
        
        return first_content_label, second_content_label
    
    def create_location_box(self):
        location_align = gtk.Alignment()
        location_align.set(1.0, 1.0, 0.5, 0.5)
        location_box = gtk.HBox(spacing=5)
        location_label = Label("文件位置:")
        self.location_entry = TextEntry("")
        self.location_entry.set_size(250, 25)
        open_button = Button("打开目录")
        location_box.pack_start(location_label, False, True)
        location_box.pack_start(self.location_entry, False, True)
        location_box.pack_start(open_button, False, True)
        location_align.add(location_box)
        return location_align
    
    def update_song(self, song):
        self.song = song
        
        # Update simpler.
        self.cover_image.set_from_pixbuf(CoverManager.get_pixbuf_from_song(song, 92, 92))
        self.title_label.set_text(song.get_str("title"))
        self.artist_label.set_text(song.get_str("artist"))
        self.album_label.set_text(song.get_str("album"))
        
        # Update detail.
        self.file_type_label.set_text(self.get_song_attr(song, True))
        self.file_format_label.set_text(self.get_song_attr(song, False))        
        self.file_size_label.set_text(song.get_str("#size"))
        self.song_duration_label.set_text(song.get_str("#duration"))        
        self.song_bitrate_label.set_text(song.get_str("#bitrate"))
        self.song_rate_label.set_text(song.get_str("#rate"))        
        self.song_added_label.set_text(song.get_str("#added"))
        self.song_lasted_label.set_text(song.get_str("#lastplayed"))
        self.song_playcount_label.set_text(song.get_str("#playcount"))
        self.song_date_label.set_text(song.get_str("#date"))        
        
        # Update location.
        self.location_entry.set_text(song.get_path())
    
    def expose_mask_cb(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
    
    
class InfoSetting(gtk.VBox):    
    
    def __init__(self, song=None):
        super(InfoSetting, self).__init__()
        self.song = song
        main_align = gtk.Alignment()
        main_align.set_padding(40, 0, 100, 0)
        main_align.set(0, 0, 0.5, 0.5)
        self.main_table = gtk.Table(6, 3)
        self.main_table.set_col_spacings(10)
        self.main_table.set_row_spacings(10)
        
        self.title_entry  = self.create_combo_entry(0, 1, "歌名:")
        self.artist_entry = self.create_combo_entry(1, 2, "歌手:")
        self.album_entry  = self.create_combo_entry(2, 3, "专辑:")
        self.genre_entry  = self.create_combo_entry(3, 4, "流派:")
        self.date_entry   = self.create_combo_entry(4, 5, "年代:")
        
        self.connect("expose-event", self.expose_mask_cb)
        main_align.add(self.main_table)
        
        # Update song
        if song:
            self.update_song(song)
            
        save_button = Button("保存")    
        save_button.connect("clicked", self.save_taginfo)
        block_box = gtk.EventBox()
        block_box.set_visible_window(False)
        block_box.set_size_request(90, -1)
        button_box = gtk.HBox()
        button_box.pack_start(create_right_align(), True, True)
        button_box.pack_start(save_button, False, False)
        button_box.pack_start(block_box, False, False)
        self.pack_start(main_align, False, True)
        self.pack_start(button_box, False, False)
        
    def create_combo_entry(self, top_attach, bottom_attach, label_text, content_text=""):   
        title_label_box = gtk.HBox()
        title_label = Label(label_text)
        title_label_box.pack_start(create_right_align(), False, True)
        title_label_box.pack_start(title_label, False, True)
        
        content_entry = TextEntry(content_text)
        content_entry.set_size(260, 25)
        self.main_table.attach(title_label_box, 0, 1, top_attach, bottom_attach, xoptions=gtk.FILL)
        self.main_table.attach(content_entry, 1, 2, top_attach, bottom_attach, xoptions=gtk.FILL)
        return content_entry
    
    def update_song(self, song):
        self.song = song
        self.title_entry.set_text(song.get_str("title"))
        self.artist_entry.set_text(song.get_str("artist"))
        self.album_entry.set_text(song.get_str("album"))
        self.genre_entry.set_text(song.get_str("genre"))
        self.date_entry.set_text(song.get_str("date"))
        
    def save_taginfo(self, widget):    
        tags_modifiable = {}
        new_title = self.title_entry.get_text()
        new_artist = self.artist_entry.get_text()
        new_album = self.album_entry.get_text()
        new_genre = self.genre_entry.get_text()
        new_date = self.date_entry.get_text()
        
        db_song = MediaDB.get_song(self.song.get("uri"))
        
        if new_title != db_song.get_str("title"):
            tags_modifiable.update({"title" : new_title})
        if new_artist != db_song.get_str("artist"):
            tags_modifiable.update({"artist" : new_artist})
        if new_album != db_song.get_str("album"):
            tags_modifiable.update({"album" : new_album})
        if new_genre != db_song.get_str("genre"):
            tags_modifiable.update({"genre" : new_genre})
        if new_date != db_song.get_str("date"):    
            tags_modifiable.update({"date" : new_date})
            
        if tags_modifiable:    
            MediaDB.set_property(db_song, tags_modifiable, write_to_file=True)
        
    def expose_mask_cb(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
    
class CoverSetting(gtk.VBox):
    
    def __init__(self, song=None):
        super(CoverSetting, self).__init__()
        
        cover_box = gtk.VBox()
        cover_image_align =  gtk.Alignment()
        cover_image_align.set(0.5, 0.5, 0.5, 0.5)
        self.cover_image = gtk.Image()
        self.cover_image.set_size_request(300, 180)
        
        
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
        change_button = Button("更改")
        change_button.connect("clicked", self.change_cover_image)
        delete_button = Button("默认")
        delete_button.connect("clicked", self.delete_cover_image)
        button_box.pack_start(create_right_align(), True, True)
        button_box.pack_start(change_button, False, False)
        button_box.pack_start(delete_button, False, False)
        
        self.pack_start(cover_box_align, False, True)
        self.pack_start(button_box_align, False, True)
        
        if song:
            self.update_song(song)
        
    def update_song(self, song):    
        self.song = song
        song_cover_pixbuf = CoverManager.get_pixbuf_from_song(song, 300, 180, optimum=False)
        self.cover_image.set_from_pixbuf(song_cover_pixbuf)
        
    def expose_mask_cb(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
    
    def change_cover_image(self, widget):
        new_cover_path = WinFile(False, "选择图片").run()
        if new_cover_path:
            CoverManager.change_cover(self.song, new_cover_path)
            
    def delete_cover_image(self, widget):        
        CoverManager.remove_cover(self.song, True)
    
class SongEditor(Window):    
    
    def __init__(self, songs, init_index=0):
        super(SongEditor, self).__init__()
        self.set_modal(True)
        self.set_keep_above(True)
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_size_request(500, 430)
        
        titlebar = Titlebar(["close"], app_name="  歌曲属性")
        titlebar.close_button.connect_after("clicked", lambda w: self.destroy())
        self.add_move_event(titlebar)
        statusbar = gtk.EventBox()
        statusbar.set_visible_window(False)
        statusbar.set_size_request(-1, 50)
        
        close_button = Button("关闭")
        close_button.connect("clicked", self.click_close_button)
        
        previous_button = Button("上一首")
        previous_button.connect("clicked", lambda w : self.update_previous_song())
        next_button = Button("下一首")
        next_button.connect("clicked", lambda w : self.update_next_song())
        
        self.record_label = Label("0/0")
        
        action_box = gtk.HBox(spacing=5)
        action_box.pack_start(previous_button, False, False)
        action_box.pack_start(self.record_label, False, False)
        action_box.pack_start(next_button, False, False)
        
        button_box = gtk.HBox()        
        button_box.pack_start(action_box, False, True)
        button_box.pack_start(create_right_align(), True, True)
        button_box.pack_start(close_button, False, True, 5)
        
        button_box_align = gtk.Alignment()
        button_box_align.set(1.0, 0.5, 1.0, 1.0)
        button_box_align.set_padding(10, 10, 10, 10)
        button_box_align.add(button_box)
        statusbar.add(button_box_align)
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(0, 0, 2, 2)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)
        
        self.window_frame.pack_start(titlebar, False, True)
        self.window_frame.pack_start(main_align, True, True)
        self.window_frame.pack_start(statusbar, False, True)
        self.main_box.connect("expose-event", self.expose_mask_cb)
        
        MediaDB.connect("simple-changed", self.db_simple_changed)
        
        # action_box.
        if len(songs) <= 1:
            action_box.set_no_show_all(True)
        else:    
            self.record_label.set_text("%d/%d" % (init_index + 1, len(songs)))
        
        # tabs
        self.song_info = SongInfo(songs[init_index])
        self.info_setting = InfoSetting(songs[init_index])
        self.cover_setting = CoverSetting(songs[init_index])
        
        self.tab_box = TabBox()
        self.tab_box.add_items([("歌曲信息", self.song_info), ("信息设置", self.info_setting), ("封面设置", self.cover_setting)])
        self.main_box.pack_start(self.tab_box)
        
        # Constants.
        self.current_index = init_index
        self.songs = songs
        
    def update_previous_song(self):    
        new_index = self.current_index - 1
        if self.is_vaild_index(new_index):
            self.current_index = new_index
            self.update_song(self.songs[new_index])
            self.update_record_label()
            
    def update_next_song(self):        
        new_index = self.current_index + 1
        if self.is_vaild_index(new_index):
            self.current_index = new_index
            self.update_song(self.songs[new_index])
            self.update_record_label()
        
    def is_vaild_index(self, index):    
        if 0 <= index < len(self.songs):
            return True
        return False
    
    def update_record_label(self):
        self.record_label.set_text("%d/%d" % (self.current_index + 1, len(self.songs)))
            
    def update_song(self, song):        
        self.song_info.update_song(song)
        self.info_setting.update_song(song)
        self.cover_setting.update_song(song)
        
    def db_simple_changed(self, widget, songs):    
        current_song = self.songs[self.current_index]
        if isinstance(songs, list):
            if self.songs[self.current_index] in songs:
                self.song_info.update_song(songs[songs.index(current_song)])
                self.cover_setting.update_song(songs[songs.index(current_song)])
        
    def expose_mask_cb(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info())
        return False
    
    def click_close_button(self, widget):    
        self.destroy()
