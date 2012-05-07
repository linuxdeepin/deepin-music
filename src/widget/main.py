#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 Deepin, Inc.
#               2011 Hou Shaohui
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
from dtk.ui.application import Application
from dtk.ui.notebook import Notebook
from dtk.ui.browser_client import BrowserClient
from dtk.ui.frame import HorizontalFrame

import utils
from widget.ui import app_theme
from widget.headerbar import HeaderBar
from widget.playlist import PlaylistUI
from widget.ui_utils import switch_tab
from widget.browser import SimpleBrowser

from config import config
from player import Player
from findfile import get_cache_file
from library import MediaDB
from dbus_manager import DeepinMusicDBus

class DeepinMusic(gobject.GObject):
    __gsignals__ = {"ready" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        gtk.window_set_default_icon_from_file(app_theme.get_image_path("skin/logo.png"))
        
        application = Application("DMuisc")
        application.close_callback = self.force_quit
        application.set_default_size(900, 540)
        application.set_icon(app_theme.get_pixbuf("skin/logo.png"))
        application.add_titlebar(
            ["max", "min", "close"],
            app_theme.get_pixbuf("skin/logo1.png"),
            "深度音乐"
            )

        self.window = application.window
        utils.set_main_window(self.window)
        application.window.change_background(app_theme.get_pixbuf("skin/main.png"))
        
        if config.get("window", "x") == "-1":
            self.window.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.window.move(int(config.get("window", "x"), int(config.get("window", "y"))))
        
        self.playlist_ui = PlaylistUI()    
        self.dbus_service = DeepinMusicDBus()
        
        notebook_box = gtk.VBox()
        web_music_box = gtk.VBox()
        local_music_box = gtk.VBox()
        
        
        notebook = Notebook([
                (app_theme.get_pixbuf("web.png"), "百度ting", lambda : switch_tab(notebook_box, web_music_box)),
                (app_theme.get_pixbuf("web.png"), "音乐管理", lambda : switch_tab(notebook_box, local_music_box)),
                ])
        
        notebook_frame = HorizontalFrame(10)
        notebook_frame.add(notebook)
        horizontal_frame = HorizontalFrame()
        browser_client = BrowserClient("http://ting.baidu.com", get_cache_file("cookie"),
                                      application.app_bus_name, application.app_dbus_name,)
        horizontal_frame.add(browser_client)
        
        # init box 
        web_music_box.pack_start(horizontal_frame, True, True)
        local_music_box.pack_start(SimpleBrowser(), True, True)
        notebook_box.add(web_music_box)
        
        right_box = gtk.VBox()
        right_box.pack_start(notebook_frame, False, False)
        browser_align = gtk.Alignment()
        browser_align.set_padding(0, 25, 10, 10)
        browser_align.set(0.5, 0.5, 1, 1)
        browser_align.add(notebook_box)
        right_box.pack_start(browser_align, True, True)
        
        
        left_box = gtk.VBox()
        left_box.pack_start(HeaderBar(), False)
        left_box.pack_start(self.playlist_ui, True, True)
        left_align = gtk.Alignment()
        left_align.set(0.5, 0.5, 1, 1)
        left_align.add(left_box)
        
        main_box = gtk.HBox()
        main_box.pack_start(left_align, False)
        main_box.pack_start(right_box, True)
        application.main_box.pack_start(main_box)
        gobject.idle_add(self.ready)
        
    def ready(self):    
        self.window.show_all()
        self.emit("ready")
        
    def force_quit(self, *args):    
        print "Start quit..."
        self.window.hide_all()
        Player.save_state()
        if not Player.is_paused(): Player.pause()
        gobject.timeout_add(500, self.__idle_quit)
        
    def __idle_quit(self, *args):    
        print "Exiting..."
        Player.stop()
        self.playlist_ui.save_to_library()
        MediaDB.save()
        config.write()
        self.window.destroy()        
        gtk.main_quit()
        print "Exit successful."
