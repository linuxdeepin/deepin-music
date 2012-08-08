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


from library import MediaDB
from udisks import udisks
from nls import _
from logger import Logger

import utils

try:
    import musicbrainz2.disc as disc
    import musicbrainz2.webservice as ws
except:    
    print "No Audio cd support (musicbrainz2 missing)"
    
import CDDB

class CDDBThread(threading.Thread, Logger):
    def __init__(self, disc_id, cb):
        threading.Thread.__init__(self)
        self._disc_id = disc_id
        print "disc_id: ", disc_id
        self._cb = cb

    def run(self):
        (status, info) = CDDB.query(self._disc_id,
                                    client_name="dmusic",
                                    client_version="1.0")

        if status in [200, 210, 211]:
            if status in [210, 211]:
                info = sorted(info, key=lambda w: w["category"])
                info = info[0]
        else:
            self.logdebug('Unable to fetch CDDB info, status=%d', status)

        (status, info) = CDDB.read(info['category'], info['disc_id'])
        if status != 210:
            self.logdebug('Unable to fetch CDDB info, status=%d', status)
            return
        gobject.idle_add(self._cb, self._disc_id, info)
    
class MusicBrainzAudioCD(object):    
    def __init__(self, device):
        self.device  = device
        try:
            self.cd = disc.readDisc(device)
        except:    
            self.disc_id = None
        else:    
            self.disc_id = self.cd.getId()
            
    def get_mb_release_id(self):
        q = ws.Query()
        filter = ws.ReleaseFilter(discId=self.disc_id)    
        
        try: results = q.getReleases(filter=filter)
        except: return None
        else:
            if len(results) > 0:
                return results[0].release.getId()               
        return None
                
    def get_mb_release(self, id):
        q = ws.Query()    
        include = ws.ReleaseIncludes(artist=True, tracks=True)
        try : res = q.getReleaseById(id, include)
        except: return None
        else: return res
        
        
    def get_tracks(self):
        if not self.disc_id : 
            print "W:MusicBrainzAudioCD:No disk id"
            return []
        
        release_id = self.get_mb_release_id()
        release = self.get_mb_release(release_id)
        if not release_id or not release: 
            print "W:MusicBrainzAudioCD:No release found"
            cd_tracks = self.cd.getTracks()
            tracks_tags = []
            for i in range(0, len(cd_tracks)):
                t = cd_tracks[i]
                # an audio CD sector size = 2352 
                # and audio CD encoded at 44.1KHz, Stereo, with 16-bit/sample 
                tags = {
                    "title" : _("Track") + " %d" % (i + 1),
                    "album" : _("Unknown album"),
                    "artist" : _("Unknown artist"),
                    "#track" : (i + 1),
                    "uri" : "cdda://%d#%s" % (i + 1, self.device),
                    "#duration" : (t[1] * 2352) / (2 * 2 * 44100) * 1000
                    }
                tracks_tags.append(tags)
            return tracks_tags
        
        album_name = release.getTitle()
        artist = release.getArtist()
        artist_name = artist.getName()
        date_album = release.getEarliestReleaseDate()
        
        tracks = release.getTracks()
        cd_tracks = self.cd.getTracks()
        
        tracks_tags = []
        
        for i in range(0, len(tracks)):
            t = tracks[i]
            cd_t = cd_tracks[i]
            tags = {
                "title" : t.getTitle(),
                "album" : album_name,
                "#date" : date_album,
                "artist" : artist_name,
                "#track" : (i + 1),
                "uri" : "cdda://%d#%s" % (i + 1, self.device),
                "#duration" : (cd_t[1] * 2352) / (2 * 2 * 44100) * 1000
                }
            tracks_tags.append(tags)
        return tracks_tags
    

class AudioCDSource(object):    
    
    def __init__(self):
        self.play_device = None
        
        try:
            import musicbrainz2
        except:    
            pass
        else:
            udisks.connect("volume-mounted", self.mount)
            # udisks.connect("volume-umounted", self.umount)
            
        for udi in udisks.get_mounted_devices():
            gobject.idle_add(self.mount, udisks, udi)
            
            
    def mount(self, udisks, udi):        
        if not self.check(udi):
            return
        device_path = udisks.get_device_path(udi)
        if device_path:
            t = threading.Thread(target=self.post_mount_thread, args=(device_path, udi))
            t.setDaemon(True)
            t.start()
            
    def post_mount_thread(self, device, udi):        
        gobject.idle_add(self.post_mount_cb, MusicBrainzAudioCD(device).get_tracks(), device, udi)
        
    def post_mount_cb(self, tracks_tags, device, udi):    
        # print tracks_tags
        print self.read_cdinfo(device)
        
    def check(self, udi):    
        if udisks.get_property(udi, "OpticalDiscNumAudioTracks") > 0:
            return True
        return False
    
    def __update_metadata_cddb(self, disc_id, cddb_info):
        num_tracks = disc_id[1]
        # encoding = 'iso8859-1'
        encoding = 'big5'
        if CDDB.proto >= 6:
            encoding = 'utf-8'

        artist, album = cddb_info['DTITLE'].split(' / ')
        year = cddb_info['DYEAR']
        print {"artist" : artist, 
               "album"  : album,
               "year"  : year
               }
        
        for i in range(num_tracks):
            title = cddb_info["TTITLE%d" % i]
            print title.decode(encoding)
            
    def read_cdinfo(self, device_path):
        import DiscID
        disc = DiscID.open(device_path)
        try:
            disc_info = DiscID.disc_id(disc)
        except:
            disc.close()
        disc.close()
        disc_id = disc_info[0]
        num_tracks = disc_info[1]

        # Create playlist from CD tracks
        minus = 0
        total = 0
        for i in range(num_tracks):
            length = (disc_info[i + 3] / 75) - minus
            if i + 1 == disc_info[1]:
                length = disc_info[i + 3] - total

            minus = disc_info[i + 3] / 75
            total += length

        # Fetch metadata from CDDB
        self.cddb_fetcher = CDDBThread(disc_info, self.__update_metadata_cddb)
        self.cddb_fetcher.start()
