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
from dtk.ui.categorybar import Categorybar
from dtk.ui.entry import TextEntry
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.listview import ListView

from widget.ui import app_theme

class SongsManager(gtk.VBox):
    def __init__(self):
        super(SongsManager, self).__init__()
        
        filter_items = [
            (app_theme.get_pixbuf("manager/artist_normal.png"), "按歌手", None),
            (app_theme.get_pixbuf("manager/album_normal.png"), "按专辑", None),
            (app_theme.get_pixbuf("manager/genre_normal.png"), "按流派", None),
            (app_theme.get_pixbuf("manager/customer_normal.png"), "自定义", None),
            (app_theme.get_pixbuf("manager/all_normal.png"), "所有歌曲", None),
            ]
        
        filter_categorybar = Categorybar(filter_items)
        # self.pack_start(filter_categorybar.category_event_box)
        
        search_box = gtk.HBox()
        search_align = gtk.Alignment()
        search_align.set(0, 0, 0, 1)
        
        entry_button = ImageButton(
            app_theme.get_pixbuf("entry/search_normal.png"),
            app_theme.get_pixbuf("entry/search_hover.png"),
            app_theme.get_pixbuf("entry/search_press.png")
            )
        
        self.search_box = TextEntry("", entry_button)
        self.search_box.set_size(300, 25)
        search_box.pack_start(search_align, True, True)
        search_box.pack_start(self.search_box, False, False)
        
        
        
