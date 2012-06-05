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

import utils
from widget.skin import app_theme
from widget.headerbar import HeaderBar
from widget.playlist import PlaylistUI
from widget.browser import SimpleBrowser
from widget.jobs_manager import jobs_manager

from config import config
from player import Player
from library import MediaDB
from dbus_manager import DeepinMusicDBus

class DeepinMusic(gobject.GObject):
    __gsignals__ = {"ready" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        gtk.window_set_default_icon_from_file(app_theme.get_theme_file_path("image/skin/logo.png"))
        
        application = Application("DMuisc")
        application.close_callback = self.force_quit
        application.set_default_size(941, 612)
        application.set_icon(app_theme.get_pixbuf("skin/logo.png"))
        application.set_skin_preview(app_theme.get_pixbuf("frame.png"))
        application.add_titlebar(
            ["theme", "max", "min", "close"],
            app_theme.get_pixbuf("skin/logo1.png"),
            "深度音乐"
            )

        self.window = application.window
        utils.set_main_window(self.window)
        
        if config.get("window", "x") == "-1":
            self.window.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.window.move(int(config.get("window", "x"), int(config.get("window", "y"))))
        
        self.playlist_ui = PlaylistUI()    
        self.header_bar = HeaderBar()
        self.dbus_service = DeepinMusicDBus()
        

        bottom_box = gtk.HBox()
        browser_align = gtk.Alignment()
        browser_align.set_padding(0, 0, 0, 0)
        browser_align.set(0.5, 0.5, 1, 1)
        browser_align.add(SimpleBrowser())
        bottom_box.pack_start(self.playlist_ui, False, False)        
        bottom_box.pack_start(browser_align, True, True)
        
        main_box = gtk.VBox()
        main_box.pack_start(self.header_bar, False)
        main_box.pack_start(bottom_box, True)


        block_box = gtk.EventBox()
        block_box.set_visible_window(False)
        block_box.set_size_request(-1, 22)
        block_box.add(jobs_manager)
        
        application.main_box.pack_start(main_box)        
        application.main_box.pack_start(block_box, False, True)
        
        gobject.idle_add(self.ready)
        
        
    def ready(self):    
        self.window.show_all()
        self.emit("ready")
        
    def force_quit(self, *args):    
        print "Start quit..."
        self.header_bar.hide_lyrics()
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
