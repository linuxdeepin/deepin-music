#!/usr/bin/env python
#-*- coding: utf-8 -*-

        
from config import config
config.load()
import utils
from player import Player
from lrcparser import LrcParser
from osd_lrc_module import ToolWindow
from osd_window import OsdWindow

import gtk
from library import MediaDB
from widget.dialog import WinDir
from source.local import ImportFolderJob
from widget.jobs_manager import jobs_manager
import gobject
import gtk
import subprocess
from ui_toolkit import *
from widget.song_item import SongItem
from widget.playlist import SongView


from widget.headerbar import HeaderBar
gobject.threads_init()
gtk.gdk.threads_init()


class PlayList(object):
    def __init__(self):
        self.songs = MediaDB.get_songs("local")
        self.index = 0
        
    def get_next_song(self):    
        self.songs = MediaDB.get_songs("local")
        self.index += 1
        if self.index > len(self.songs) - 1:
            self.index = 0
        return self.songs[self.index], False   
            
    def get_previous_song(self):
        self.songs = MediaDB.get_songs("local")
        self.index -= 1
        if self.index < 0:
            self.index = len(self.songs) - 1
        return self.songs[self.index]
        
playlists = PlayList()        


class DeepinPlayer(object):
    '''Music player for linux deepin.'''
    def __init__(self):
        self.window = Application("DMuisc")
        self.window.set_default_size(320, 400)
        self.window.add_titlebar(["min", "max", "close"], None,
                                 "  Deepin Music")

        # self.window.window.connect("expose-event", self.expose_cb)
        mainbox = gtk.VBox(spacing=5)
        
        MediaDB.load()
        mainbox.pack_start(HeaderBar(), False, False)

        Player.load()
        MediaDB.connect("added", self.reload_db)
        scrolled_window = ScrolledWindow()
        
        # 
        self.list_view = SongView(
            [(lambda item: item.title, cmp),
             (lambda item: item.artist, cmp),
             (lambda item: item.length, cmp)])

        self.list_view.connect("double-click-item", self.double_click_item)
        self.list_view.add_titles(["歌名", "艺术家", "时间"])

        scrolled_window.add_child(self.list_view)
        
        if MediaDB.get_songs("local"):
            # items = [ SongItem(song) for song in MediaDB.get_songs("local")]
            # self.list_view.add_items(items)
            self.list_view.add_songs(MediaDB.get_songs("local"))

        self.window.window.change_background(app_theme.get_pixbuf("skin/main.png"))
        self.window.main_box.pack_start(mainbox, False, False)
        self.window.main_box.pack_start(scrolled_window, True, True)
        self.window.main_box.pack_start(jobs_manager, False, False)


        # window = OsdWindow()
        # window.set_lyric(0, "把你捧在手上 虔诚地焚香")
        # window.set_lyric(1, "剪下一段烛光 将经纶点亮, 不求荡气回肠 只求爱一场, 爱到最后受了伤 哭得好绝望, 我用尽一生一世 来将你供养")
        # window.set_percentage(1, 0.5)
        # window.window.show_all()
        # lrc_win = ToolWindow('/home/vicious/.lyrics/王麟-伤不起.lrc')    
        # lrc_win.run()
        
        self.player = Player        
        self.player.set_source(self.list_view)
        self.window.run()        
        
    def expose_cb(self, widget, event):    
        pass
    
    def double_click_item(self, widget, item, column, x, y):
        Player.play_new(item.get_song())
        self.list_view.set_highlight(item)


    
    def reload_db(self, db, song_type, songs):

        items = [ SongItem(song) for song in songs]
        self.list_view.add_items(items)
        
    def test_cb(self, widget):    
        # print "dd"
        MediaDB.full_erase("local")
        ImportFolderJob()
        
    def play_cb(self, widget):
        '''play song'''
        song_uri = utils.get_uri_from_path(widget.get_filename())
        song = MediaDB.create_song({"uri": song_uri}, "local", True)
        self.player.play_new(song)
        
if __name__ == "__main__":       
    DeepinPlayer()

    

        
