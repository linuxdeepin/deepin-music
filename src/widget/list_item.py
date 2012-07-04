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
import gobject
from dtk.ui.scrolled_window import ScrolledWindow

from widget.song_view import SongView
from widget.ui_utils import switch_tab as switch_box
from widget.ui import ComplexButton
from widget.skin import app_theme

DEFAULT_FONT_SIZE = 8


class PlaylistItem(gobject.GObject):
    
    def __init__(self, playlist):
        '''Init song item.'''
        self.item_id = None
        self.main_box = gtk.HBox()        
        self.create_jobs_box()
        self.update(playlist)

    def create_jobs_box(self):    
        jobs_box = gtk.VBox(spacing=5)
        self.file_job_button = self.create_job_button("add_file", "添加文件")
        self.dir_job_button = self.create_job_button("add_dir", "添加文件夹")
        jobs_box.pack_start(self.file_job_button, False, False)
        jobs_box.pack_start(self.dir_job_button, False, False)
        self.jobs_align = gtk.Alignment()
        self.jobs_align.set(1.0, 1.0, 0.5, 0.5)
        self.jobs_align.add(jobs_box)
        
    def create_job_button(self, icon_name, content, callback=None):    
        button = ComplexButton(
            [app_theme.get_pixbuf("jobs/small_normal.png"),
             app_theme.get_pixbuf("jobs/small_hover.png"),
             app_theme.get_pixbuf("jobs/small_press.png")],
            app_theme.get_pixbuf("jobs/%s.png" % icon_name),
            content
            )
        if callback:
            button.connect("clicked", callback)
        return button    
        
    def set_title(self, value):    
        self.title = value
        
    def get_title(self):    
        return self.title
    
    def get_left_image(self):
        return None
    
    def get_has_arrow(self):
        return None
    
    def set_item_id(self, index):
        self.item_id = index
        
    def get_item_id(self):    
        return self.item_id
        
    def update(self, playlist):
        '''update'''
        self.playlist = playlist
        songs = self.playlist.get_songs()
        self.song_view = SongView()
        self.song_view.add_songs(songs)
        self.scrolled_window = ScrolledWindow(0, 0)
        self.scrolled_window.add_child(self.song_view)
        self.scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        self.scrolled_window.set_size_request(220, -1)
        self.title = playlist.get_name()
        
    def get_list_widget(self):
        if self.get_songs():
            switch_box(self.main_box, self.scrolled_window)
        else:    
            switch_box(self.main_box, self.jobs_align)
        return self.main_box    
    
    def get_songs(self):
        if self.song_view:
            return self.song_view.get_songs()
