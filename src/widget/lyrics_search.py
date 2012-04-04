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
import os
import gobject
from dtk.ui.button import Button
from dtk.ui.entry import Entry
from dtk.ui.utils import get_content_size
from dtk.ui.threads import post_gui
from dtk.ui.constant import ALIGN_END
from constant import DEFAULT_FONT_SIZE
from dtk.ui.listview import ListView, render_text
from dtk.ui.scrolled_window import ScrolledWindow
from ui_toolkit import NormalWindow, app_theme
from lrc_manager import ttplayer_engine
import utils


class SearchUI(NormalWindow):
    
    def __init__(self):
        NormalWindow.__init__(self)
        self.window.background_dpixbuf = app_theme.get_pixbuf("skin/main.png")
        self.add_titlebar(["close"], title="歌词搜索")
        padding_x = 10
        padding_y = 5
        
        info_box = gtk.HBox(spacing=10)
        self.artist_entry = Entry()
        # self.artist_entry = gtk.Entry()
        self.artist_entry.set_text("Beyond")
        self.artist_entry.set_size_request(150, 25)
        # self.title_entry = gtk.Entry()
        self.title_entry = Entry()
        self.title_entry.set_text("海阔天空")
        self.title_entry.set_size_request(150, 25)
        artist_label = gtk.Label()
        artist_label.set_markup("<span color=\"black\">%s</span>" % "艺术家:")
        title_label = gtk.Label()
        title_label.set_markup("<span color=\"black\">%s</span>" % "歌曲:")
        
        search_button = Button("搜索")
        search_button.connect("clicked", self.search_lyric_cb)
        
        info_box.pack_start(title_label, False, False)
        info_box.pack_start(self.title_entry, False, False)
        info_box.pack_start(artist_label, False, False)
        info_box.pack_start(self.artist_entry, False, False)
        info_box.pack_start(search_button, False, False)
        info_align = gtk.Alignment()
        info_align.set_padding(0, 0, padding_x, padding_x)
        info_align.add(info_box)
        
        self.prompt_label = gtk.Label()
        self.prompt_label.set_alignment(0.0, 0.5)
        self.prompt_label.set_size_request(-1, -1)
        
        
        fill_left = gtk.EventBox()
        fill_left.set_size_request(padding_x, -1)
        fill_left.set_visible_window(False)
        
        fill_right = gtk.EventBox()
        fill_right.set_size_request(padding_x, -1)
        fill_right.set_visible_window(False)
        scrolled_window = ScrolledWindow()
        scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        sort_items = [(lambda item: item.title, cmp), (lambda item: item.artist, cmp)]
        self.result_view = ListView(sort_items)
        self.result_view.add_titles(["歌曲名", "艺术家"])
        scrolled_window.add_child(self.result_view)
        scrolled_box = gtk.HBox()
        scrolled_box.pack_start(fill_left, False, False)
        scrolled_box.pack_start(scrolled_window, True, True)
        scrolled_box.pack_start(fill_right, False, False)
        
        left_align = gtk.Alignment()
        left_align.set(0, 0, 0, 1)
        bottom_box = gtk.HBox(spacing=10)
        download_button = Button("下载")
        download_button.connect("clicked", self.download_lyric_cb)
        cancel_button = Button("取消")
        cancel_button.connect("clicked", self.cancel_cb)
        right_align = gtk.EventBox()
        right_align.set_visible_window(False)

        bottom_box.pack_start(self.prompt_label, False, False)
        bottom_box.pack_start(left_align, True, True)
        bottom_box.pack_start(download_button, False, False)
        bottom_box.pack_start(cancel_button, False, False)
        bottom_box.pack_start(right_align, False, False)
        
        botton_align = gtk.EventBox()
        botton_align.set_visible_window(False)
        botton_align.set_size_request(-1, padding_y)
        
        self.main_box.set_spacing(5)
        self.main_box.pack_start(info_align, False, False)
        self.main_box.pack_start(scrolled_box, True, True)
        self.main_box.pack_start(bottom_box, False, False)
        self.main_box.pack_start(botton_align, False, False)

    def cancel_cb(self, widget):            
        self.window.hide_all()
        
    def search_lyric_cb(self, widget):
        self.result_view.clear()
        artist = self.artist_entry.get_text()
        title = self.title_entry.get_text()
        if artist == "" and title == "":
            self.prompt_label.set_markup("<span color=\"white\">   %s</span>" % "囧!没有找到!")
            return
        utils.threaded(self.request_lyrics(artist, title))
            
    @post_gui        
    def request_lyrics(self, artist, title):
        result = ttplayer_engine.request(artist, title)
        if result != None:
            items = [SearchItem(each_info) for each_info in result]
            self.result_view.add_items(items)
            self.prompt_label.set_markup("<span color=\"white\">   %s</span>" % "找到%d个歌词 :)" % len(result))
        else:    
            self.prompt_label.set_markup("<span color=\"white\">   %s</span>" % "囧!没有找到!")
        

    def download_lyric_cb(self, widget):
        artist = self.artist_entry.get_text()
        title = self.title_entry.get_text()
        select_items = self.result_view.select_rows
        save_filepath = os.path.join(os.path.expanduser("~/.lyrics"), "%s-%s.lrc" % (artist, title))
        if len(select_items) > 0:
            url = self.result_view.items[select_items[0]].get_url()
            utils.threaded(self.start_download(url, save_filepath))
            
    @post_gui        
    def start_download(self, url, local_uri):
        result = utils.download(url, local_uri)
        if result:
            self.prompt_label.set_markup("<span color=\"white\">   %s</span>" % "文件已保存到 ~\.lyrics")
        else:    
            self.prompt_label.set_markup("<span color=\"white\">   %s</span>" % "囧! 下载失败!")
        
        
class SearchItem(gobject.GObject):        
    
    __gsignals__ = {"redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()), }

    def __init__(self, lrc_list):
        super(SearchItem, self).__init__()
        self.update(lrc_list)
        
    def set_index(self, index):    
        self.index = index
        
    def get_index(self):    
        return self.index
    
    def emit_redraw_request(self):
        self.emit("redraw-request")
    
    def update(self, lrc_list):    
        self.__url = lrc_list[2]
        self.title  = lrc_list[1]
        self.artist = lrc_list[0]
        
        # Calculate item size.
        self.title_padding_x = 10
        self.title_padding_y = 5
        (self.title_width, self.title_height) = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
        self.artist_padding_x = 10
        self.artist_padding_y = 5
        (self.artist_width, self.artist_height) = get_content_size(self.artist, DEFAULT_FONT_SIZE)
        
    def render_title(self, cr, rect):
        '''Render title.'''
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_text(cr, rect, self.title, font_size=DEFAULT_FONT_SIZE)
    
    def render_artist(self, cr, rect):
        '''Render artist.'''
        rect.x += self.artist_padding_x
        rect.width -= self.title_padding_x * 2
        render_text(cr, rect, self.artist, font_size = DEFAULT_FONT_SIZE)
        
    def get_column_sizes(self):
        '''Get sizes.'''
        return [(min(self.title_width + self.title_padding_x * 2, 120),
                 self.title_height + self.title_padding_y * 2),
                (min(self.artist_width + self.artist_padding_x * 2, 100),
                 self.artist_height + self.artist_padding_y * 2)
                ]    
    
    def get_renders(self):
        '''Get render callbacks.'''
        return [self.render_title,
                self.render_artist]
    
    def get_url(self):
        return self.__url
    
    
        

    
        
        
        
