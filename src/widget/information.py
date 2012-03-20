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
import pango

from library import MediaDB
from player import Player


class PlayInfo(gtk.Label):
    def __init__(self):
        super(PlayInfo,self).__init__("")

        self.set_ellipsize(pango.ELLIPSIZE_END)
        # self.set_use_markup(True)
        # self.set_selectable(True)
        self.set_alignment(0,0.5)
        
        # self.set_markup("<span size=\"large\"><b>"+"深度音乐\n"+"</b></span>")
        # self.set_tooltip_text("No playing")
        self.set_label("<span color=\"black\"> 深度音乐 Music</span>")
        self.set_use_markup(True)

        Player.connect("instant-new-song",self.__new_song)
        MediaDB.connect("simple-changed",self.__on_change)
        Player.bin.connect("buffering", self.__on_buffering)
        
        self.song = None
        
    def __on_buffering(self, playbin, progress):
        if self.song:
            self.update(self.song, progress)

    def __on_change(self,db,songs):
        if self.song in songs:
            self.update(self.song)
    
    def __new_song(self, player, song):
        self.update(song)

    def update(self, song , buffering = None):
        if not song: return
        self.song = song
        content = song.get_str("title") + " - " + song.get_str("artist")
        if "&" in content: content = content.replace("&", " ")
        self.set_label("<span color=\"black\"> %s</span>" % content)
        
        
        # markup = ""

        # tip = song.get_str("title")
        # markup += "<span size=\"large\"><b>"+song.get_str("title",True)+"</b></span>"
        # if song.get("station"):
        #     markup += " <b><i>("+song.get_str("station",True)+")</i></b>"
        
        # markup += "\n<span>"

        # # if song.get("artist"):
        # #     tip += " - "+song.get_str("artist")
        # #     markup += "<i>"+song.get_str("artist",True)+"</i> "
        # markup += "\n"
        # if song.get("album"):
        #     tip += " - "+song.get_str("album")
        #     markup += song.get_str("album",True)
        #     date =  song.get_str("date", True)
        #     if date:
        #         markup += " <i>(%s)</i>"%date
        # elif song.get("info_supp"):
        #     markup +=  song.get_str("info_supp",True)
        # elif buffering is not None and buffering <= 100:
        #     markup +=  "\n<i>(%s... %d%%)</i>"%("buffering", buffering)
        # markup += "</span>"
        
        
        

        # self.set_markup(markup)
        # self.set_tooltip_text(tip)
