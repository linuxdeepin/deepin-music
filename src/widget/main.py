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

from dtk.ui.window import Window
from dtk.ui.titlebar import Titlebar
from dtk.ui.utils import move_window

import utils
from widget.ui import app_theme
from widget.headerbar import header_bar
from widget.playlist import playlist_ui

from config import config
from player import Player
from library import MediaDB
from dbus_manager import DeepinMusicDBus

class DeepinMusic(gobject.GObject):
    __gsignals__ = {"ready" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        gtk.window_set_default_icon_from_file(app_theme.get_image_path("skin/logo1.png"))
        
        self.window = Window()
        utils.set_main_window(self.window)
        self.window.change_background(app_theme.get_pixbuf("skin/main.png"))
        self.window.set_default_size(280, 500)
        self.titlebar = Titlebar(["min", "max", "close"], app_theme.get_pixbuf("skin/logo1.png"), "深度音乐")
        self.titlebar.min_button.connect("clicked", lambda w: self.window.min_window())
        self.titlebar.max_button.connect("clicked", lambda w: self.window.toggle_max_window())         
        self.titlebar.close_button.connect("clicked", self.force_quit)       
        self.titlebar.drag_box.connect('button-press-event', lambda w, e: move_window(w, e, self.window))

        
        self.main_box = self.window.window_frame
        self.main_box.pack_start(self.titlebar.box, False)
        
        if config.get("window", "x") == "-1":
            self.window.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.window.move(int(config.get("window", "x"), int(config.get("window", "y"))))
        
        # self.dbus_service = DeepinMusicDBus()
        self.main_box.pack_start(header_bar, False)
        self.main_box.pack_start(playlist_ui, True, True)
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
        playlist_ui.save_to_library()
        MediaDB.save()
        config.write()
        gtk.main_quit()
        print "Exit successful."
        
