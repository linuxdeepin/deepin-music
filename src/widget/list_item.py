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
from dtk.ui.utils import propagate_expose

from widget.song_view import SongView
from widget.ui_utils import switch_tab as switch_box
from widget.ui import ComplexButton
from widget.skin import app_theme
from widget.ui_utils import (
    draw_alpha_mask, create_upper_align, create_bottom_align,
    create_left_align, create_right_align)
from nls import _


DEFAULT_FONT_SIZE = 8


class PlaylistItem(gobject.GObject):
    
    def __init__(self, playlist, udi=None):
        '''Init song item.'''
        self.item_id = None
        self.main_box = gtk.VBox()
        self.update(playlist)        
        self.create_jobs_box()
        self.udi = udi

    def on_jobs_expose_event(self, widget, event):            
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "layoutMiddle")
        
    def create_jobs_box(self):    
        
        self.file_job_button = self.create_job_button("plus", _("Add Music"), self.song_view.recursion_add_dir)

        self.job_box = gtk.EventBox()
        self.job_box.set_size_request(195, -1)
        targets = [("text/deepin-songs", gtk.TARGET_SAME_APP, 1), ("text/uri-list", 0, 2), ("text/plain", 0, 3)]
        self.job_box.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP,
                           targets, gtk.gdk.ACTION_COPY)
        self.job_box.set_visible_window(False)
        self.job_box.connect("drag-data-received", self.song_view.on_drag_data_received)
        
        # Content box. 
        content_box = gtk.VBox()
        content_box.pack_start(create_bottom_align(), True, True)
        content_box.pack_start(self.file_job_button, False, False)
        content_box.pack_start(create_upper_align(), True, True)
        
        # Rind box.
        rind_box = gtk.HBox()
        rind_box.pack_start(create_right_align(), True, True)
        rind_box.pack_start(content_box, False, False)
        rind_box.pack_start(create_left_align(), True, True)
        
        self.job_box.add(rind_box)
        jobs_align = gtk.Alignment()
        jobs_align.set(0.5, 0.5, 1, 1)
        jobs_align.add(self.job_box)

        
        self.jobs_main_box = gtk.VBox()
        self.jobs_main_box.add(jobs_align)
        self.jobs_main_box.connect("expose-event", self.on_jobs_expose_event)        
        
    def create_job_button(self, icon_name, content, callback=None):    
        button = ComplexButton(
            [app_theme.get_pixbuf("jobs/complex_normal.png"),
             app_theme.get_pixbuf("jobs/complex_hover.png"),
             app_theme.get_pixbuf("jobs/complex_press.png")],
            app_theme.get_pixbuf("jobs/%s.png" % icon_name),
            content
            )
        if callback:
            button.connect("clicked", lambda w : callback())
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
        self.song_view.connect("begin-add-items", lambda w: self.switch_it())
        self.song_view.connect("empty-items", lambda w: self.switch_it(False))
        self.scrolled_window = ScrolledWindow(0, 0)
        self.scrolled_window.add_child(self.song_view)
        # self.scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        self.scrolled_window.set_size_request(195, -1)
        self.title = playlist.get_name()
        
    def get_list_widget(self):
        if self.get_songs():
            switch_box(self.main_box, self.scrolled_window)
        else:    
            switch_box(self.main_box, self.jobs_main_box)
        return self.main_box    
    
    def switch_it(self, scrolled_window=True):
        if scrolled_window:
            switch_box(self.main_box, self.scrolled_window)
        else:    
            switch_box(self.main_box, self.jobs_main_box)
    
    def get_songs(self):
        if self.song_view:
            return self.song_view.get_songs()
