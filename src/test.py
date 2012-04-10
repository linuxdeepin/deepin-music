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


class DeepinPlayer(object):
    '''Music player for linux deepin.'''
    def __init__(self):
        self.window = Application("DMuisc")
        self.window.set_default_size(320, 550)
        self.window.add_titlebar(["min", "max", "close"], None,
                                 "  Deepin Music")
        mainbox = gtk.VBox(spacing=5)
        utils.set_main_window(self.window.window)
        MediaDB.load()
        mainbox.pack_start(header_bar, False, False)
        Player.load()

    
        # self.list_view = playlist_ui.song_view
        
        # if MediaDB.get_songs("local"):
        #     self.list_view.add_songs(MediaDB.get_songs("local"))
        #     self.list_view.set_highlight_song(Player.song)

        # MediaDB.connect("added", self.reload_db)    
        self.window.window.change_background(app_theme.get_pixbuf("skin/main.png"))
        self.window.main_box.pack_start(mainbox, False, False)
        self.window.main_box.pack_start(playlist_ui, True, True)
        self.window.main_box.pack_start(jobs_manager, False, False)
        self.player = Player        
        self.time_source = 0
        self.lyrics_display = lyrics_display
        self.lyrics_display.run()
        # self.player.set_source(self.list_view)
        self.dbus_service = DeepinMusicDBus()
        self.window.run()        
    
    # def double_click_item(self, widget, item, column, x, y):
    #     self.list_view.set_highlight(item)        
    #     Player.play_new(item.get_song())

    # def popup_listview_menu(self, widget, x, y, current_item,  select_items):    
    #     self.list_view.popup_menu().show((x,y))
    
    # def reload_db(self, db, song_type, songs):

    #     items = [ SongItem(song) for song in songs]
    #     self.list_view.add_items(items)
        
    def test_cb(self, widget):    
        MediaDB.full_erase("local")
        ImportFolderJob()
        
    def play_cb(self, widget):
        '''play song'''
        song_uri = utils.get_uri_from_path(widget.get_filename())
        song = MediaDB.create_song({"uri": song_uri}, "local", True)
        self.player.play_new(song)
        
if __name__ == "__main__":       
    DeepinPlayer()
