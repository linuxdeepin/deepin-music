#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Hou Shaohui
# 
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou Shaohui <houshao55@gmail.com>
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

import gobject
import threading

from udisks import udisks
from mscddb import mscddb
from nls import _
from helper import Dispatcher

from player import Player
from song import Song

try:
    import DiscID
except:
    import discid as DiscID


class CDDBInfo(object):
    def __init__(self, device_path):
        self.device_path = device_path
        try:
            disc_fp = DiscID.open(device_path)            
        except:    
            self.disc_info = None
        else:    
            self.disc_info = DiscID.disc_id(disc_fp)
        finally:    
            disc_fp.close()
            
    def get_track_lengths(self):        
        num_tracks = self.disc_info[1]
        minus = 0
        total = 0
        lengths = []
        for i in range(num_tracks):
            length = (self.disc_info[i + 3] / 75) - minus
            if i + 1 == self.disc_info[1]:
                length = self.disc_info[i + 3] - total
                
            lengths.append(length * 1000)    
            minus = self.disc_info[i + 3] / 75
            total += length
        return lengths    
            
    def get_tracks(self):        
        if not self.disc_info:
            print "W:DiscID:No disk id"
            return []
        query_infos = mscddb.query(self.disc_info[1:])
        lengths = self.get_track_lengths()

        if query_infos is None:
            track_tags = []
            for i in range(0, self.disc_info[1]):
                tags = {
                    "title" : _("Track") + " %d" % (i + 1),
                    "album" : _("Unknown album"),
                    "artist" : _("Unknown artist"),
                    "#track" : (i + 1),
                    "uri" : "cdda://%d#%s" % (i + 1, self.device_path),
                    "#duration" : lengths[i]
                    }
                track_tags.append(tags)
            return track_tags, _("Audio CD")
        
        query_tags, query_label = query_infos
        [info.update({"uri" : "cdda://%d#%s" % (info["#track"], self.device_path),
                      "#duration" : lengths[info["#track"] - 1]}) for info in query_tags]
        
        return query_tags, query_label or _("Audio CD")
    
class AudioCDSource(object):        
    
    def __init__(self):
        
        self.audiocd_items = {}
        udisks.connect("volume-mounted", self.mount)
        udisks.connect("volume-umounted", self.umount)
        
    def mount(self, udisks, udi):    
        if not self.check(udi):
            return
        device_path = udisks.get_device_path(udi)
        if device_path:
            t = threading.Thread(target=self.post_mount_thread, args=(device_path, udi))
            t.setDaemon(True)
            t.start()
            
    def umount(self, udisks, udi):        
        if udi and self.audiocd_items.has_key(udi):
            if Player.song and Player.song.get_type() == "audiocd":
                Player.stop()
            del self.audiocd_items[udi]
            Dispatcher.delete_audiocd_playlist(udi)
            
    def post_mount_thread(self, device_path, udi):        
        gobject.idle_add(self.post_mount_cb, CDDBInfo(device_path).get_tracks(), device_path, udi)
        
    def post_mount_cb(self, track_infos, device_path, udi):    
        track_tags, cd_label = track_infos
        if len(track_tags) > 0:
            songs = [] 
            for tag in track_tags:
                cd_song = Song()
                cd_song.init_from_dict(tag)
                cd_song.set_type("cdda")
                songs.append(cd_song)
            Dispatcher.new_audiocd_playlist(cd_label, songs, udi)
            self.audiocd_items[udi] = songs
        
    def check(self, udi):    
        if udisks.get_property(udi, "OpticalDiscNumAudioTracks") > 0:
            return True
        return False
