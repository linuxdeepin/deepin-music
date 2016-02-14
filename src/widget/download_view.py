#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

from dtk.ui.treeview import TreeView
from helper import Dispatcher

from widget.download_item import TaskItem
from player import Player

class TaskView(TreeView):
    
    def __init__(self):
        TreeView.__init__(self, enable_drag_drop=False, enable_multiple_select=False)
        
        # Player.connect("instant-new-song", self.on_new_song)
        Dispatcher.connect("download-songs", self.on_dispatcher_download_songs)
        
        self.set_expand_column(1)
        
        
    def on_dispatcher_download_songs(self, obj, songs):    
        self.add_songs(songs)
        
    def add_songs(self, songs):    
        self.add_items([TaskItem(song) for song in songs])
        
    def download_current_song(self):        
        if Player.song:
            self.add_items([TaskItem(Player.song)])
        
    def draw_mask(self, cr, x, y, w, h):
        cr.set_source_rgb(1, 1, 1)
        cr.rectangle(x, y, w, h)
        cr.fill()
        
    def on_new_song(self, player, song):    
        if song.get_type() == "baidumusic":
            self.download_current_song()
        
