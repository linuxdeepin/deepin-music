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
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.label import Label

from widget.song_view import MultiDragSongView
from widget.radio_view import RadioIconView, TAG_SEARCH
from widget.ui_utils import draw_alpha_mask
from nls import _
from library import MediaDB


class GlobalSearch(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        self.set_spacing(5)
        local_box = gtk.VBox()
        local_label = Label("<b>%s</b>" % _("本地资源"))
        local_label.set_size_request(200, 25)
        local_label_align = gtk.Alignment()
        local_label_align.set_padding(5, 0, 10, 0)
        local_label_align.add(local_label)
        
        self.local_view, self.local_sw = self.get_local_view()
        local_box.pack_start(local_label_align, False, True)
        local_box.pack_start(self.local_sw, False, True)
        
        radio_box = gtk.VBox()
        radio_label = Label("<b>%s</b>" % _("电台资源"))
        radio_label.set_size_request(200, 25)
        radio_label_align = gtk.Alignment()
        radio_label_align.set_padding(5, 0, 10, 0)
        radio_label_align.add(radio_label)
        self.radio_view, self.radio_sw = self.get_radio_view()
        radio_box.pack_start(radio_label_align, False, True)
        radio_box.pack_start(self.radio_sw, True, True)
        
        self.pack_start(local_box, True, True)
        self.pack_start(radio_box, True, True)
        
    def get_local_view(self):    
        song_view = MultiDragSongView()
        song_view.add_titles([_("Title"), _("Artist"), _("Album"), _("Added time")])
        scrolled_window = song_view.get_scrolled_window()
        return song_view, scrolled_window
    
    def get_radio_view(self):
        radio_view = RadioIconView(tag=TAG_SEARCH, padding_y=10)
        scrolled_window = radio_view.get_scrolled_window()
        return radio_view, scrolled_window
    
    def begin_search(self, keyword):
        self.radio_view.clear_items()
        self.radio_view.set_keyword(keyword)
        self.radio_view.start_fetch_channels()
        self.local_view.start_search_songs(keyword)
