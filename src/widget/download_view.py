#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2013 Deepin, Inc.
#               2011 ~ 2013 Hou ShaoHui
# 
# Author:     Hou ShaoHui <houshao55@gmail.com>
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

from dtk.ui.new_treeview import TreeView

from widget.download_item import TaskItem
from player import Player

class TaskView(TreeView):
    
    def __init__(self):
        TreeView.__init__(self, enable_drag_drop=False, enable_multiple_select=False)
        
        Player.connect("instant-new-song", self.on_new_song)
        
        self.set_expand_column(1)
        
    def add_songs(self, songs):    
        pass
            
    def download_current_song(self):        
        if Player.song:
            self.add_items([TaskItem(Player.song)])
        
    def draw_mask(self, cr, x, y, w, h):
        cr.set_source_rgb(1, 1, 1)
        cr.rectangle(x, y, w, h)
        cr.fill()
        
    def on_new_song(self, player, song):    
        if song.get_type() == "douban":
            self.download_current_song()
        
