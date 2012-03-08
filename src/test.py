#!/usr/bin/env python
#-*- coding: utf-8 -*-

        

import utils
from player import Player
import gtk
from library import MediaDB
from widget.dialog import WinDir
from source.local import ImportFolderJob
from widget.jobs_manager import jobs_manager

from ui_toolkit import *
from widget.headerbar import HeaderBar


class DeepinPlayer(object):
    '''Music player for linux deepin.'''
    def __init__(self):
        self.window = Application("DMuisc")
        self.window.set_default_size(320, 157)
        self.window.add_titlebar(["max", "min", "close"], 
                                 None,
                                 "  Deepin Music")
        

        mainbox = gtk.VBox(spacing=5)
        
        self.file_chooser = gtk.FileChooserButton("select file")        
        self.file_chooser.connect("file-set", self.play_cb)
        
        mainbox.pack_start(HeaderBar(), False, False)
        mainbox.pack_start(self.file_chooser, False, False)
        
        test_button = gtk.Button("test")
        test_button.connect("clicked", self.test_cb)
        
        mainbox.pack_start(test_button)
        mainbox.pack_start(jobs_manager)
        # save_button = gtk.Button("save")
        # save_button.connect("clicked", self.save_to_file)
        # mainbox.pack_start(save_button)
        self.window.window.change_background(app_theme.get_pixbuf("skin/main.png"))
        self.window.main_box.pack_start(mainbox)
        
        self.player = Player        
        self.window.run()
        
        
    def test_cb(self, widget):    
        print "dd"
        ImportFolderJob()
        
        
    def play_cb(self, widget):
        '''play song'''
        song_uri = utils.get_uri_from_path(widget.get_filename())
        song = MediaDB.create_song({"uri": song_uri}, "local", True)

        self.player.play_new(song)
        
    # def save_to_file(self, widget):    
    #     songs = MediaDB.get_songs("local")
    #     for song in songs:
    #         fp.write(song.get_str("title"))
        
    #     fp.close()    
        
    # def rewind_cb(self, widget):    
    #     # file_song = self.file_chooser.get_uri()

    #     # song =  MediaDB.get_song(file_song)
    #     # print song.get_dict()
    #     # print song.get_str("#duration")
    #     # print song.get_str("title")
    #     # print song.get_str("album")
    #     # print song.get_str("artist")
    #     # print song.get_searchable()

    #     artist_dict = {}
        
    #     all_songs = MediaDB.get_songs("local")
        
    #     for song in all_songs:
            
    #         artist = song.get("artist")
            
    #         if not artist_dict.has_key(artist):
    #             artist_dict[artist] = [song]                
    #         else:    
    #             artist_dict[artist].append(song)


        
if __name__ == "__main__":       
    DeepinPlayer()

    

        
