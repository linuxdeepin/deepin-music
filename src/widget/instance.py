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
from dtk.ui.menu import Menu

import utils
from widget.skin import app_theme
from widget.headerbar import FullHeaderBar
from widget.playlist import PlaylistUI
from widget.browser import SimpleBrowser
from widget.jobs_manager import jobs_manager
from widget.tray import TrayIcon
from widget.equalizer import EqualizerWindow
from widget.preference import PreferenceDialog

from config import config
from player import Player
from library import MediaDB
from helper import Dispatcher
from logger import Logger

def mainloop():    
    gtk.main()
    

class DeepinMusic(gobject.GObject, Logger):
    __gsignals__ = {"ready" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        application = Application("DMuisc")
        application.close_callback = self.quit
        application.set_default_size(858, 625)
        application.set_icon(app_theme.get_pixbuf("skin/logo.ico"))
        application.set_skin_preview(app_theme.get_pixbuf("frame.png"))
        application.add_titlebar(
            ["theme", "max", "min", "close"],
            app_theme.get_pixbuf("skin/logo1.png"),
            "深度音乐"
            )
        
        self.window = application.window
        utils.set_main_window(self)
        
        if config.get("window", "x") == "-1":
            self.window.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.window.move(int(config.get("window","x")),int(config.get("window","y")))
            
        try:    
            self.window.resize(int(config.get("window","width")),int(config.get("window","height")))
        except:    
            pass
        
        window_state = config.get("window", "state")
        if window_state == "maximized":
            self.window.maximize()
        elif window_state == "normal":    
            self.window.unmaximize()
        
        self.playlist_ui = PlaylistUI()    
        self.header_bar = FullHeaderBar()
        self.preference_dialog = PreferenceDialog()
        self.equalizer_win = EqualizerWindow()
        self.window.add_move_event(self.header_bar)

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
        
        self.window.connect("delete-event", self.quit)
        self.window.connect("configure-event", self.on_configure_event)
        self.window.connect("destroy", self.quit)
        
        Dispatcher.connect("quit",self.force_quit)
        Dispatcher.connect("show-main-menu", self.show_instance_menu)
        Dispatcher.connect("show-setting", lambda w : self.preference_dialog.show_all())
        Dispatcher.connect("show-desktop-page", lambda w: self.preference_dialog.show_desktop_lyrics_page())
        Dispatcher.connect("show-scroll-page", lambda w: self.preference_dialog.show_scroll_lyrics_page())
        
        self.tray_icon = None
        gobject.idle_add(self.ready)
        
        
    def quit(self, *param):    
        self.__save_configure()
        if config.get("setting", "close_to_tray") == "false" or self.tray_icon == None:
            self.force_quit()
        return True
            
    def ready(self, show=True):    
        if show:
            self.window.show_all()
        if config.getboolean("setting", "use_tray"):    
            self.tray_icon = TrayIcon(self)
        self.emit("ready")
        
    def force_quit(self, *args):    
        self.loginfo("Start quit...")
        self.header_bar.hide_lyrics()
        self.window.hide_all()
        Player.save_state()
        if not Player.is_paused(): Player.pause()
        gobject.timeout_add(500, self.__idle_quit)
        
    def __idle_quit(self, *args):    
        self.loginfo("Exiting...")
        Player.stop()
        self.playlist_ui.save_to_library()
        MediaDB.save()
        config.write()
        self.window.destroy()        
        gtk.main_quit()
        self.loginfo("Exit successful.")
        
    def on_configure_event(self,widget=None,event=None):
        if widget.get_property("visible"):
            if widget.get_resizable():
                config.set("window","width","%d"%event.width)
                config.set("window","height","%d"%event.height)
            config.set("window","x","%d"%event.x)
            config.set("window","y","%d"%event.y)
            
    def __on_config_set(self, ob, section, option, value):        
        if section == "setting" and option == "use_tray":
            use_tray = config.getboolean(section, option)
            if self.tray_icon and not use_tray:
                self.tray_icon.destroy()
                self.tray_icon = None
            elif not self.tray_icon and use_tray:    
                self.tray_icon = TrayIcon(self)

    def __save_configure(self):            
        event = self.window.get_state()
        if event == gtk.gdk.WINDOW_STATE_MAXIMIZED:
            config.set("window", "state", "maximized")
        else:
            config.set("window", "state", "normal")
        self.window.hide_all()
        
    def toggle_visible(self, bring_to_front=False):    
        if self.window.get_property("visible"):
            if self.window.is_active():
                if not bring_to_front:
                    self.hide_to_tray()
            else:    
                self.window.present()
        else:        
            self.show_from_tray()
            
    def hide_to_tray(self):
        event = self.window.get_state()
        if event == gtk.gdk.WINDOW_STATE_MAXIMIZED:
            config.set("window", "state", "maximized")
        else:
            config.set("window", "state", "normal")
        self.window.hide_all()

    def show_from_tray(self):
        self.window.move(int(config.get("window", "x")), int(config.get("window", "y")))
        window_state = config.get("window", "state")
        if window_state == "maximized" :
            self.window.maximize()
        if window_state == "normal":
            self.window.unmaximize()
        self.window.show_all()
        
    def show_instance_menu(self, obj, x, y):
        curren_view = self.playlist_ui.get_selected_song_view()
        menu_items = [
            (None, "文件添加", curren_view.get_add_menu()),
            (None, "播放控制", None),
            (None, "播放模式", curren_view.get_playmode_menu()),
            None,
            (None, "均衡器", lambda : self.equalizer_win.run()),
            (None, "总在最前", None),
            None,
            (None, "桌面歌词", None),
            (None, "窗口歌词", None),
            None,
            (None, "选项设置", lambda : self.preference_dialog.show_all()),
            (None, "关于软件", None),
            None,
            (None, "退出", None),
            ]
        Menu(menu_items, True).show((x, y))
    