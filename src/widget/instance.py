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
import os
from dtk.ui.application import Application
from dtk.ui.menu import Menu
from dtk.ui.button import ToggleButton
from dtk.ui.slider import Wizard
from deepin_utils.file import get_parent_dir
from dtk.ui.button import LinkButton

from widget.skin import app_theme
from widget.headerbar import SimpleHeadbar
from widget.list_manager import ListManager
from widget.lyrics_module import LyricsModule
from widget.browser_manager import BrowserMananger
from widget.jobs_manager import jobs_manager
from widget.tray import TrayIcon
from widget.equalizer import EqualizerWindow
from widget.preference import PreferenceDialog
from widget.ui_utils import  create_right_align
from widget.global_keys import global_hotkeys
from widget.dialog import WinFile
from widget.converter import AttributesUI, convert_task_manager
from widget.mini import MiniWindow
from widget.playlist import playlist_ui
from widget.ui import QuitDialog

import plugins
from constant import (FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT,
                      SIMPLE_DEFAULT_WIDTH, SIMPLE_DEFAULT_HEIGHT,
                      PROGRAM_NAME_LONG)
from nls import _
from config import config
from player import Player
from library import MediaDB
from webcast_library import WebcastDB
from mmkeys_wrap import MMKeys
from song import Song
import common
import utils
from source.audiocd import AudioCDSource

from helper import Dispatcher
from logger import Logger
import locale

def mainloop():    
    gtk.main()

(lang, encode) = locale.getdefaultlocale()
wizard_dir = os.path.join(get_parent_dir(__file__, 3), "wizard/en")    
wizard_root_dir = os.path.dirname(wizard_dir)
if lang == "zh_CN":
    wizard_dir = os.path.join(get_parent_dir(__file__, 3), "wizard/zh_CN")    
elif lang in ["zh_HK", "zh_TW"]:
    wizard_dir = os.path.join(get_parent_dir(__file__, 3), "wizard/zh_HK")    
    
class DeepinMusic(gobject.GObject, Logger):
    __gsignals__ = {"ready" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ())}
    
    def __init__(self):
        gobject.GObject.__init__(self)
        image_dir = os.path.join(get_parent_dir(__file__, 3), "image")
        application = Application("DMuisc", resizable=False)
        application.close_callback = self.prompt_quit
        application.set_icon(os.path.join(image_dir, "logo.ico"))
        application.set_skin_preview(os.path.join(image_dir, "frame.png"))
        application.add_titlebar(
            ["theme", "menu", "min", "close"],
            os.path.join(image_dir, "logo1.png"),
            PROGRAM_NAME_LONG
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
        
        self.plugins = plugins.PluginsManager(self, False)        
        self.browser_manager = BrowserMananger()        
        self.tray_icon = TrayIcon(self)        
        self.lyrics_display = LyricsModule()
        self.list_manager = ListManager()
        self.simple_header_bar = SimpleHeadbar()
        self.preference_dialog = PreferenceDialog()
        self.equalizer_win = EqualizerWindow()
        self.mmkeys = MMKeys()
        self.audiocd = AudioCDSource()
        self.playlist_ui = playlist_ui

        self.mini_window = MiniWindow()
        
        self.window.add_move_event(self.simple_header_bar)

        bottom_box = gtk.HBox()
        self.browser_align = gtk.Alignment()
        self.browser_align.set_padding(0, 0, 0, 0)
        self.browser_align.set(0.5, 0.5, 1, 1)
        
        list_manager_align = gtk.Alignment()
        list_manager_align.set_padding(0, 0, 0, 0)
        list_manager_align.set(1, 1, 1, 1)
        list_manager_align.add(self.list_manager)
        
        self.browser_align.add(self.browser_manager)
        bottom_box.pack_start(list_manager_align, False, False)        
        bottom_box.pack_start(self.browser_align, True, True)
        self.browser_align.set_no_show_all(True)
        
        main_box = gtk.VBox()
        self.header_box = gtk.VBox()
        self.header_box.add(self.simple_header_bar)
        main_box.pack_start(self.header_box, False)
        main_box.pack_start(bottom_box, True)
        
        self.link_box = gtk.HBox()
        self.link_box.pack_start(create_right_align(), True, True)
        self.link_box.pack_start(LinkButton(_("Join us"), "http://www.linuxdeepin.com/joinus/job"), False, False)
        
        status_box = gtk.HBox(spacing=5)
        status_box.pack_start(jobs_manager)
        status_box.pack_start(self.link_box, padding=5)
        
        status_bar = gtk.EventBox()
        status_bar.set_visible_window(False)
        status_bar.set_size_request(-1, 22)
        status_bar.add(status_box)
        
        application.main_box.pack_start(main_box)        
        application.main_box.pack_start(status_bar, False, True)
        
        # if config.get("globalkey", "enable", "false") == "true":
            # global_hotkeys.start_bind()
        
        if config.get("setting", "window_mode") == "simple":
            self.revert_toggle_button.set_active(False)
        else:    
            self.revert_toggle_button.set_active(True)
                
        self.change_view(self.revert_toggle_button)    
            
        if config.get("window", "x") == "-1":
            self.window.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.window.move(int(config.get("window","x")),int(config.get("window","y")))
            
        # try:    
        #     self.window.resize(int(config.get("window","width")),int(config.get("window","height")))
        # except:    
        #     pass
        
        # window_state = config.get("window", "state")
        # if window_state == "maximized":
        #     self.window.maximize()
        # elif window_state == "normal":    
        #     self.window.unmaximize()
        
        self.window.connect("delete-event", self.quit)
        self.window.connect("configure-event", self.on_configure_event)
        self.window.connect("destroy", self.quit)
        
        Dispatcher.connect("quit",self.force_quit)
        Dispatcher.connect("show-main-menu", self.show_instance_menu)
        Dispatcher.connect("show-setting", lambda w : self.preference_dialog.show_all())
        Dispatcher.connect("show-desktop-page", lambda w: self.preference_dialog.show_desktop_lyrics_page())
        Dispatcher.connect("show-scroll-page", lambda w: self.preference_dialog.show_scroll_lyrics_page())
        Dispatcher.connect("show-job", self.hide_link_box)
        Dispatcher.connect("hide-job", self.show_link_box)
        
        gobject.idle_add(self.ready)
        
    def right_click_cb(self, widget, event):    
        if event.button == 3:
            Dispatcher.show_main_menu(int(event.x_root), int(event.y_root))
        
    def quit(self, *param):    
        self.hide_to_tray()
        if config.get("setting", "close_to_tray") == "false" or self.tray_icon == None:
            self.force_quit()
        return True
    
    def prompt_quit(self, *param):
        if config.get("setting", "close_remember") != "true":
            QuitDialog(lambda : self.quit()).show_all()
        else:    
            self.quit()    
            
    def ready(self, show=True):    
        first_started =  config.get("setting", "first_started", "")        
        if show and first_started:
            self.ready_show()
            

        self.emit("ready")
        Dispatcher.emit("ready")
        # wizard
        if not first_started:
            self.show_wizard_win(True, self.ready_show)
            config.set("setting", "first_started", "false")
            
    def ready_show(self):    
        self.plugins.load_enabled()
        self.preference_dialog.load_plugins()
        self.app_show_all()
        if config.getboolean("lyrics", "status"):
            self.lyrics_display.run()

        
    def force_quit(self, *args):    
        self.loginfo("Start quit...")
        self.app_hide_all()
        Player.save_state()
        if not Player.is_paused(): Player.pause()
        gobject.timeout_add(500, self.__idle_quit)
        
    def __idle_quit(self, *args):    
        self.loginfo("Exiting...")
        Player.stop()
        self.mmkeys.release()
        Dispatcher.emit("being-quit")
        playlist_ui.save_to_library()
        MediaDB.save()
        WebcastDB.save()
        config.write()
        global_hotkeys.stop_bind()

        self.app_destroy()
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
        if self.get_app_mode() == "normal":
            if self.window.get_property("visible"):
                self.hide_to_tray()
            else:    
                self.show_from_tray()
        else:        
            self.mini_window.toggle_window()
        
    def toggle_visible(self, bring_to_front=False):    
        if self.get_app_mode() == "normal":
            if self.window.get_property("visible"):
                if self.window.is_active():
                    if not bring_to_front:
                        self.hide_to_tray()
                else:    
                    self.window.present()
            else:        
                self.show_from_tray()
        else:        
            self.mini_window.toggle_visible(bring_to_front)
            
    def hide_to_tray(self):
        event = self.window.get_state()
        if config.get("setting", "window_mode") == "full":
            if event & gtk.gdk.WINDOW_STATE_MAXIMIZED == gtk.gdk.WINDOW_STATE_MAXIMIZED:
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
            state_label = _("Play")
            state_pixbuf = self.get_pixbuf_group("play")
        else:    
            state_label = _("Pause")
            state_pixbuf = self.get_pixbuf_group("pause")
        menu_items.append((state_pixbuf, state_label, Player.playpause))    
        control_items = [
            (self.get_pixbuf_group("forward"), _("Forward"), Player.forward),
            (self.get_pixbuf_group("rewind"), _("Rewind"), Player.rewind),
            (self.get_pixbuf_group("previous"), _("Previous"), Player.previous),
            (self.get_pixbuf_group("next"), _("Next"), Player.next),
            ]
        menu_items.extend(control_items)
        return Menu(menu_items)
    
    def menu_button_press(self, widget, event):
        self.show_instance_menu(None, int(event.x_root), int(event.y_root))
        
    def get_convert_sub_menu(self):    
        menu_items = [
            (None, _("Converter"), self.choose_file_and_convert),
            (None, _("Task Manager"), lambda : convert_task_manager.visible_it()),
            ]
        return Menu(menu_items)
    
    def choose_file_and_convert(self):
        filename = WinFile(False).run()
        if filename and common.file_is_supported(filename):
            tags = {"uri" : utils.get_uri_from_path(filename)}
            s = Song()
            s.init_from_dict(tags)
            s.set_type("local")                        
            s.read_from_file()
            AttributesUI([s]).show_window()
        
    def show_instance_menu(self, obj, x, y):
        curren_view = self.playlist_ui.get_selected_song_view()
        menu_items = [
            (None, _("Add"), curren_view.get_add_menu()),
            (None, _("Play Controls"), self.get_play_control_menu()),
            (self.get_pixbuf_group("playmode"), _("Playback mode"), curren_view.get_playmode_menu()),
            None,
            (None, _("Equalizer"), lambda : self.equalizer_win.run()),
            (None, _("Converter"), self.get_convert_sub_menu()),
            None,
            self.get_app_mode_menu(),
            None,
            self.get_lyrics_menu_items(),
            self.get_locked_menu_items(),
            None,
            (None, _("View New Features"), self.show_wizard_win),            
            (self.get_pixbuf_group("setting"), _("Preferences"), lambda : self.preference_dialog.show_all()),
            None,
            (self.get_pixbuf_group("close"), _("Quit"), self.force_quit),
            ]
        
        if config.getboolean("lyrics", "status"):
            menu_items.insert(10, (None, _("Search Lyrics"), lambda : Dispatcher.emit("search-lyrics")))

        Menu(menu_items, True).show((x, y))
        
    def show_wizard_win(self, show_button=False, callback=None):    
        Wizard(
            [os.path.join(wizard_dir, "%d.png" % i) for i in range(3)],
            (os.path.join(wizard_root_dir, "dot_normal.png"),
             os.path.join(wizard_root_dir, "dot_active.png"),             
             ),
            (os.path.join(wizard_dir, "start_normal.png"),
             os.path.join(wizard_dir, "start_press.png"),             
             ),
            show_button,
            callback
            ).show_all()
        
    def get_lyrics_menu_items(self):    
        if config.getboolean("lyrics", "status"):
            return (None, _("Hide Lyrics"), lambda : Dispatcher.close_lyrics())
        else:    
            return (None, _("Show Lyrics"), lambda : Dispatcher.show_lyrics())
        
    def get_locked_menu_items(self):    
        if config.getboolean("lyrics", "locked"):    
            return (self.get_pixbuf_group("unlock"), _("Unlock Lyrics"), lambda : Dispatcher.unlock_lyrics())
        else:
            return (self.get_pixbuf_group("lock"), _("Lock Lyrics"), lambda : Dispatcher.lock_lyrics())
            
    def get_pixbuf_group(self, name):    
        return (app_theme.get_pixbuf("tray/%s_normal.png" % name),
                app_theme.get_pixbuf("tray/%s_hover.png" % name),
                app_theme.get_pixbuf("tray/%s_disable.png" % name),
                )
    
    def change_view(self, widget):    

        if not widget.get_active():
            config.set("setting", "window_mode", "simple")
            self.window.unmaximize()
            self.browser_align.hide_all()
            self.browser_align.set_no_show_all(True)
            self.window.set_default_size(SIMPLE_DEFAULT_WIDTH, SIMPLE_DEFAULT_HEIGHT)
            self.window.set_geometry_hints(None, SIMPLE_DEFAULT_WIDTH, SIMPLE_DEFAULT_HEIGHT, 
                                           SIMPLE_DEFAULT_WIDTH, SIMPLE_DEFAULT_HEIGHT, # (310, 700)
                                           -1, -1, -1, -1, -1, -1)
            self.window.resize(SIMPLE_DEFAULT_WIDTH, SIMPLE_DEFAULT_HEIGHT)
            self.window.queue_draw()
        else:
            config.set("setting", "window_mode", "full")
            self.browser_align.set_no_show_all(False)
            self.browser_align.show_all()
            self.window.set_default_size(FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT)            
            self.window.set_geometry_hints(None, FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT, 
                                           FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT,  -1, -1, -1, -1, -1, -1)
            self.window.resize(FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT)
        # Dispatcher.volume(float(config.get("player", "volume", "1.0")))        
        
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

    def hide_link_box(self, obj):
        self.link_box.hide_all()
        self.link_box.set_no_show_all(True)
        
    def show_link_box(self, obj):    
        self.link_box.set_no_show_all(False)
        self.link_box.show_all()
        
    def app_show_all(self):    
        if self.get_app_mode() == "normal":
            self.window.show_all()
        else:    
            self.mini_window.show_all()
            
    def app_hide_all(self):        
        self.mini_window.hide_all()        
        self.window.hide_all()
            
    def app_destroy(self):
        self.mini_window.destroy()        
        self.window.destroy()
        
    def get_app_mode(self):    
        return config.get("setting", "app_mode", "normal")
    
    def get_app_mode_menu(self):
        if self.get_app_mode() == "normal":
            return (None, _("Mini Mode"), lambda : self.change_app_mode("mini"))
        else:    
            return (None, _("Normal Mode"), lambda : self.change_app_mode("normal"))
            
    def change_app_mode(self, mode):        
        config.set("setting", "app_mode", mode)        
        if mode == "normal":
            self.mini_window.hide_to_tray()
            self.show_from_tray()
            self.simple_header_bar.sync_volume()
        else:    
            self.hide_to_tray()
            self.mini_window.show_from_tray()
            self.mini_window.sync_volume()
