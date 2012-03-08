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

from ui_toolkit import *
import gtk
from player import Player
from widget.information import PlayInfo
from widget.timer import SongTimer, VolumeSlider

class HeaderBar(gtk.HBox):
    def __init__(self):
        super(HeaderBar, self).__init__()
        self.set_border_width(5)
        
        # cover box
        cover_box = ImageBox(app_theme.get_pixbuf("skin/default_cover.png"))
        

        play_status_pixbuf = app_theme.get_pixbuf("action/play.png")
        pause_status_pixbuf = app_theme.get_pixbuf("action/pause.png")
        
        # swap played status handler
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        
        
        # play button
        self.__play = ToggleButton(play_status_pixbuf, None, pause_status_pixbuf)
        self.__id_signal_play = self.__play.connect("toggled", lambda w: Player.playpause())
        
        prev = self.__create_button("previous")
        next = self.__create_button("next")
        
        self.vol = VolumeSlider()
        song_timer = SongTimer()
        
        mainbtn = gtk.HBox(spacing=3)
        prev_align = gtk.Alignment()
        prev_align.set(0.5, 0.5, 0, 0)
        prev_align.add(prev)
        
        next_align = gtk.Alignment()
        next_align.set(0.5, 0.5, 0, 0)
        next_align.add(next)
        
        mainbtn.pack_start(prev_align, False, False)
        mainbtn.pack_start(self.__play, False, False)
        mainbtn.pack_start(next_align, False, False)
        
        topbox = gtk.HBox()
        topbox.pack_start(PlayInfo(),True, True)
        topbox.pack_start(mainbtn, False, False)
        
        control_box = gtk.VBox(spacing=3)
        control_box.pack_start(topbox, False, False)
        control_box.pack_start(song_timer, False, False)
        
        plugs_box = gtk.HBox()
        more_box = gtk.HBox(False, 10)
        more_align = gtk.Alignment()
        more_align.set(1.0, 0, 0, 0)
        more_box.pack_start(playlist_button)
        more_box.pack_start(lyrics_button)
        more_box.pack_start(musicbox_button)
        more_box.pack_start(media_button)
        more_align.add(more_box)        
        
        volume_align = gtk.Alignment()
        volume_align.set(0, 1.0, 0, 0)
        volume_align.add(self.vol)
        

        plugs_box.pack_start(volume_align, False, False)
        plugs_box.pack_start(more_align, True, True)        

        control_box.pack_start(plugs_box, False, False)
        information = gtk.HBox(spacing=6)
        information.pack_start(cover_box, False, False)
        information.pack_start(control_box, True, True)
        
        
        
        self.pack_start(information, True, True)
                
        # Player.connect("stopped", self.reload_action_button)
        # Player.connect("instant-new-song", self.on_new_song)
        

        
    def __swap_play_status(self, obj, active):    
        self.__play.handler_block(self.__id_signal_play)
        self.__play.set_active(active)
        self.__play.handler_unblock(self.__id_signal_play)
        
        
    def __create_button(self, name, tip_msg=None):    
        button = ImageButton(
            app_theme.get_pixbuf("action/%s_normal.png" % name),
            app_theme.get_pixbuf("action/%s_normal.png" % name),
            app_theme.get_pixbuf("action/%s_hover.png" % name),
            )
        button.connect("clicked", self.player_control, name)
        # todo tip
        return button
    
    def player_control(self, button, name):
        getattr(Player, name)()
        
        

        

        

