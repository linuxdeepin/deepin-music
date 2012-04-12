#!/usr/bin/env python
#-*- coding: utf-8 -*-



import gtk        
gtk.gdk.threads_init()
from config import config
config.load()
import utils
from player import Player
from dbus_manager import DeepinMusicDBus


from library import MediaDB
from widget.dialog import WinDir
from source.local import ImportFolderJob
from widget.jobs_manager import jobs_manager
import gobject
import gtk
import os
import subprocess
from widget.ui import app_theme
from dtk.ui.application import Application
from widget.song_item import SongItem
from widget.playlist import playlist_ui
from widget.lyrics_module import lyrics_display
from widget.headerbar import header_bar
from findfile import get_cache_file
from dtk.ui.scrolled_window import ScrolledWindow
import dtk_browser

class DeepinPlayer(object):
    '''Music player for linux deepin.'''
    def __init__(self):
        self.window = Application("DMuisc")
        self.window.set_default_size(320, 550)
        self.window.add_titlebar(["min", "max", "close"], None,
                                 "  Deepin Music")
        
        # self.browser = dtk_browser.browser_new("http://www.xiami.com", get_cache_file("cookie.txt"))
        # scrolled_window  = gtk.ScrolledWindow()
        # scrolled_window.add(self.browser)
        right_box = gtk.VBox(spacing=5)
        utils.set_main_window(self.window.window)
        MediaDB.load()
        Player.load()
        
        # right_box.pack_start(header_bar, False, False)
        # right_box.pack_start(playlist_ui, True, True)
        # right_box.pack_start(jobs_manager, False, False)
        
        main_box = gtk.HBox(spacing=10)
        main_box.pack_start(right_box, False, False)
        # main_box.pack_start(scrolled_window, True, True)
        self.window.window.change_background(app_theme.get_pixbuf("skin/main.png"))
        self.window.main_box.pack_start(header_bar, False, False)
        self.window.main_box.pack_start(playlist_ui, True, True)
        self.window.main_box.pack_start(jobs_manager, False, False)
        self.player = Player        
        self.time_source = 0
        self.lyrics_display = lyrics_display
        self.lyrics_display.run()
        self.dbus_service = DeepinMusicDBus()
        self.window.run()        
        
if __name__ == "__main__":       
    DeepinPlayer()
