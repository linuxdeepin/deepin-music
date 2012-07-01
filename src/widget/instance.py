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
from dtk.ui.button import ToggleButton

import utils
from widget.skin import app_theme
from widget.headerbar import FullHeaderBar, SimpleHeadber
from widget.playlist import PlaylistUI
from widget.lyrics_module import LyricsModule
from widget.browser import SimpleBrowser
from widget.jobs_manager import jobs_manager
from widget.tray import TrayIcon
from widget.equalizer import EqualizerWindow
from widget.preference import PreferenceDialog
from widget.ui_utils import switch_tab
from widget.global_keys import GlobalKeys

from widget.song_search import SongSearchUI

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
        application.set_icon(app_theme.get_pixbuf("skin/logo.ico"))
        application.set_skin_preview(app_theme.get_pixbuf("frame.png"))
        application.add_titlebar(
            ["theme", "menu", "min", "close"],
            app_theme.get_pixbuf("skin/logo1.png"),
            "深度音乐"
            )
        application.titlebar.menu_button.connect("button-press-event", self.menu_button_press)        
        application.titlebar.connect("button-press-event", self.right_click_cb)
        
        # Window mode change.
        self.revert_toggle_button = self.create_revert_button()
        self.revert_toggle_button.connect("toggled", self.change_view) 

        application.titlebar.button_box.pack_start(self.revert_toggle_button)
        application.titlebar.button_box.reorder_child(self.revert_toggle_button, 1)
        self.window = application.window
        self.window.is_disable_window_maximized = self.is_disable_window_maximized
        utils.set_main_window(self)
        
        self.lyrics_display = LyricsModule()
        self.playlist_ui = PlaylistUI()    
        self.full_header_bar = FullHeaderBar()
        self.simple_header_bar = SimpleHeadber()
        self.preference_dialog = PreferenceDialog()
        self.simple_browser = SimpleBrowser()
        self.equalizer_win = EqualizerWindow()
        self.global_keys = GlobalKeys()
            
        self.window.add_move_event(self.full_header_bar)
        self.window.add_move_event(self.simple_header_bar)

        bottom_box = gtk.HBox()
        self.browser_align = gtk.Alignment()
        self.browser_align.set_padding(0, 0, 0, 0)
        self.browser_align.set(0.5, 0.5, 1, 1)
        self.browser_align.add(self.simple_browser)
        bottom_box.pack_start(self.playlist_ui, False, False)        
        bottom_box.pack_start(self.browser_align, True, True)
        self.browser_align.set_no_show_all(True)
        
        main_box = gtk.VBox()
        self.header_box = gtk.VBox()
        self.header_box.add(self.simple_header_bar)
        main_box.pack_start(self.header_box, False)
        main_box.pack_start(bottom_box, True)

        block_box = gtk.EventBox()
        block_box.set_visible_window(False)
        block_box.set_size_request(-1, 22)
        block_box.add(jobs_manager)
        
        application.main_box.pack_start(main_box)        
        application.main_box.pack_start(block_box, False, True)
        
        if config.get("setting", "window_mode") == "simple":
            self.revert_toggle_button.set_active(False)
        else:    
            self.revert_toggle_button.set_active(True)
                
        self.change_view(self.revert_toggle_button)    
            
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
        
        self.window.connect("delete-event", self.quit)
        self.window.connect("configure-event", self.on_configure_event)
        self.window.connect("destroy", self.quit)
        
        Dispatcher.connect("quit",self.force_quit)
        Dispatcher.connect("show-main-menu", self.show_instance_menu)
        Dispatcher.connect("show-setting", lambda w : self.preference_dialog.show_all())
        Dispatcher.connect("show-desktop-page", lambda w: self.preference_dialog.show_desktop_lyrics_page())
        Dispatcher.connect("show-scroll-page", lambda w: self.preference_dialog.show_scroll_lyrics_page())
        
        self.tray_icon = None
        if config.get("setting", "use_tray", "true") == "true":    
            self.tray_icon = TrayIcon(self)
        
        gobject.idle_add(self.ready)
        
    def right_click_cb(self, widget, event):    
        if event.button == 3:
            Dispatcher.show_main_menu(int(event.x_root), int(event.y_root))
        
    def quit(self, *param):    
        self.hide_to_tray()
        if config.get("setting", "close_to_tray") == "false" or self.tray_icon == None:
            self.force_quit()
        return True
            
    def ready(self, show=True):    
        if show:
            self.window.show_all()
            if config.getboolean("lyrics", "status"):
                self.lyrics_display.run()
        self.emit("ready")
        
    def force_quit(self, *args):    
        self.loginfo("Start quit...")
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
                
    def toggle_window(self):            
        if self.window.get_property("visible"):
            self.hide_to_tray()
        else:    
            self.show_from_tray()
        
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
        if config.get("setting", "window_mode") == "full":
            if event == gtk.gdk.WINDOW_STATE_MAXIMIZED:
                config.set("window", "state", "maximized")
            else:
                config.set("window", "state", "normal")
        self.window.hide_all()

    def show_from_tray(self):
        self.window.move(int(config.get("window", "x")), int(config.get("window", "y")))
        if config.get("setting", "window_mode") == "full":
            window_state = config.get("window", "state")
            if window_state == "maximized" :
                self.window.maximize()
            if window_state == "normal":
                self.window.unmaximize()
        self.window.show_all()
        
    def get_play_control_menu(self):    
        menu_items = []
        if Player.is_paused():
            state_label = "播放"
            state_pixbuf = self.get_pixbuf_group("play")
        else:    
            state_label = "暂停"
            state_pixbuf = self.get_pixbuf_group("pause")
        menu_items.append((state_pixbuf, state_label, Player.playpause))    
        control_items = [
            (self.get_pixbuf_group("forward"), "快进", Player.forward),
            (self.get_pixbuf_group("rewind"), "后退", Player.rewind),
            (self.get_pixbuf_group("previous"), "上一首", Player.previous),
            (self.get_pixbuf_group("next"), "下一首", Player.next),
            ]
        menu_items.extend(control_items)
        return Menu(menu_items)
    
    def menu_button_press(self, widget, event):
        self.show_instance_menu(None, int(event.x_root), int(event.y_root))
        
    def show_instance_menu(self, obj, x, y):
        curren_view = self.playlist_ui.get_selected_song_view()
        menu_items = [
            (None, "文件添加", curren_view.get_add_menu()),
            (None, "播放控制", self.get_play_control_menu()),
            (self.get_pixbuf_group("playmode"), "播放模式", curren_view.get_playmode_menu()),
            None,
            (None, "均衡器", lambda : self.equalizer_win.run()),
            (None, "歌曲搜索", lambda : SongSearchUI().show_window()),
            None,
            self.get_lyrics_menu_items(),
            self.get_locked_menu_items(),
            None,
            (self.get_pixbuf_group("setting"), "选项设置", lambda : self.preference_dialog.show_all()),
            (None, "关于软件", None),
            None,
            (self.get_pixbuf_group("close"), "退出", self.force_quit),
            ]
        Menu(menu_items, True).show((x, y))
        
    def get_lyrics_menu_items(self):    
        if config.getboolean("lyrics", "status"):
            return (None, "关闭歌词", lambda : Dispatcher.close_lyrics())
        else:    
            return (None, "打开歌词", lambda : Dispatcher.show_lyrics())
        
    def get_locked_menu_items(self):    
        if config.getboolean("lyrics", "locked"):    
            return (self.get_pixbuf_group("unlock"), "解锁歌词", lambda : Dispatcher.unlock_lyrics())
        else:
            return (self.get_pixbuf_group("lock"), "锁定歌词", lambda : Dispatcher.lock_lyrics())
            
    def get_pixbuf_group(self, name):    
        return (app_theme.get_pixbuf("tray/%s_normal.png" % name), app_theme.get_pixbuf("tray/%s_hover.png" % name))
    
    def change_view(self, widget):    

        if not widget.get_active():
            Dispatcher.change_window_mode("simple")
            config.set("setting", "window_mode", "simple")
            switch_tab(self.header_box, self.simple_header_bar)
            self.browser_align.hide_all()
            self.browser_align.set_no_show_all(True)
            self.window.set_default_size(330, 625)
            self.window.set_geometry_hints(None, 330, 300, 330, 700, -1, -1, -1, -1, -1, -1)
            self.window.resize(330, 625)
            self.window.queue_draw()
        else:
            Dispatcher.change_window_mode("full")
            config.set("setting", "window_mode", "full")
            switch_tab(self.header_box, self.full_header_bar)
            self.browser_align.set_no_show_all(False)
            self.browser_align.show_all()
            self.window.set_default_size(816, 625)            
            self.window.set_geometry_hints(None, 816, 625, -1, -1,  -1, -1, -1, -1, -1, -1)
            self.window.resize(816, 625)
        Dispatcher.volume(float(config.get("player", "volume", "1.0")))        
        
    def is_disable_window_maximized(self):    
        if config.get("setting", "window_mode") == "simple":
            return True
        else:
            return False
        
    def create_revert_button(self):    
        button = ToggleButton(
            app_theme.get_pixbuf("mode/simple_normal.png"),
            app_theme.get_pixbuf("mode/full_normal.png"),
            app_theme.get_pixbuf("mode/simple_hover.png"),
            app_theme.get_pixbuf("mode/full_hover.png"),
            app_theme.get_pixbuf("mode/simple_press.png"),
            app_theme.get_pixbuf("mode/full_press.png"),
            )
        return button
