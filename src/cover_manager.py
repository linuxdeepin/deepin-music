#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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

import os
import re
import gobject
import gtk
import fnmatch
import time

from urllib import quote, urlopen
from mutagen.id3 import ID3

import utils
from config import config
from logger import Logger
from library import MediaDB
from findfile import get_cache_file
from widget.ui import app_theme


REINIT_COVER_TO_SKIP_TIME = 100 * 60 * 30

COVER_SIZE = {"x": 68, "y": 68}
COVER_SAVE_SIZE = {"x": 300, "y": 300}
BROWSER_COVER_SIZE = {"x": 40, "y": 40}

KEEP_COVER_IN_MEMORY = True

COVER_PATTERNS = [
    "cover.jpg", "cover.png",
    "*front*.jpg", "*front*.png",
    "*cover*.jpg", "*cover*.png",
    "*album*.jpg", "*album*.png",
    "folder.jpg", "folder.png",
    ".folder.jpg", ".folder.png",
    "*artist*.jpg", "*artist*.png",
    "*.jpg","*.png",
    ]

class DeepinCoverManager(Logger):
    COVER_PIXBUF = {}
    COVER_TO_SKIP = []
    
    def __init__(self):
        gobject.timeout_add(REINIT_COVER_TO_SKIP_TIME, self.reinit_skip_cover)
        
        self.DEFAULT_COVER = app_theme.get_image_path("cover/default_cover.png")
        self.DEFAULT_COVER_PIXBUF = {
            (40, 40) : gtk.gdk.pixbuf_new_from_file_at_size(self.DEFAULT_COVER, 40, 40),
            (COVER_SIZE["x"], COVER_SIZE["y"]) : gtk.gdk.pixbuf_new_from_file_at_size(self.DEFAULT_COVER, COVER_SIZE["x"], COVER_SIZE["y"]),
            (BROWSER_COVER_SIZE["x"], BROWSER_COVER_SIZE["y"]) : gtk.gdk.pixbuf_new_from_file_at_size(self.DEFAULT_COVER, BROWSER_COVER_SIZE["x"], BROWSER_COVER_SIZE["y"])
            }
        
    def reinit_skip_cover(self):    
        self.COVER_TO_SKIP = []
        
    def get_cover_search_str(self, song):
        if not song.get_str("album"):
            return song.get_str("artist") + " " + song.get_str("title")
        else:
            return song.get_str("album")
    
    def get_pixbuf_from_album(self, album, x=None, y=None):
        x = (x or BROWSER_COVER_SIZE["x"])
        y = (y or BROWSER_COVER_SIZE["y"])
        
        filename = get_cache_file("cover/%s.jpg" % album)
        
        if not os.path.exists(filename):
            filename = self.DEFAULT_COVER
            
        if not self.COVER_PIXBUF.has_key((filename, x, y)):
            try:
                pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(filename, x, y)
            except gobject.GError:    
                self.logwarn("failed to load %s, try deleted it...", filename)
                try:
                    os.unlink(filename)
                except:    
                    pass
                pixbuf = None
        elif KEEP_COVER_IN_MEMORY:        
            pixbuf = self.COVER_PIXBUF[(filename, x, y)]
        else:    
            pixbuf = None
            
        if not pixbuf:    
            if not self.DEFAULT_COVER_PIXBUF.has_key((x, y)):
                self.DEFAULT_COVER_PIXBUF[(x, y)] = gtk.gdk.pixbuf_new_from_file_at_size(self.DEFAULT_COVER, x, y)
            pixbuf = self.DEFAULT_COVER_PIXBUF[(x, y)]    
            
        if KEEP_COVER_IN_MEMORY:    
            self.COVER_PIXBUF[(filename, x, y)] = pixbuf
            
        return pixbuf    
    
    
    def remove_pixbuf_from_cache(self, album, x=None, y=None):
        filename = get_cache_file("cover/%s.jpg" % album)
        
        for info in self.COVER_PIXBUF.keys():
            if filename == info[0]:
                del self.COVER_PIXBUF[info]
                
    def has_cover(self, song):            
        cover = self.get_cover(song, False) # todo
        return cover != self.DEFAULT_COVER and os.path.exists(cover)
    
    def get_cover_path(self, song):
        return get_cache_file("cover/" + self.get_cover_search_str(song) + ".jpg")
    
    def get_pixbuf_from_song(self, song, x, y, try_web=True):
        
        filename = self.get_cover(song, try_web)
        if not self.COVER_PIXBUF.has_key((filename, x, y)):
            try:
                pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(filename, x, y)
            except gobject.GError:    
                self.logwarn("failed to load %s in a pixbuf", filename)
                try:
                    os.unlink(filename)
                except:
                    pass
                pixbuf = None
                
        elif KEEP_COVER_IN_MEMORY:        
            pixbuf = self.COVER_PIXBUF[(filename, x, y)]
                
        else:    
            pixbuf = None
            
        if not pixbuf:    
            if not self.DEFAULT_COVER_PIXBUF.has_key((x, y)):
                self.DEFAULT_COVER_PIXBUF[(x, y)] = gtk.gdk.pixbuf_new_from_file_at_size(self.DEFAULT_COVER, x, y)
            pixbuf = self.DEFAULT_COVER_PIXBUF[(x, y)]    
            
        if KEEP_COVER_IN_MEMORY:    
            self.COVER_PIXBUF[(filename, x, y)] = pixbuf
            
        return pixbuf    
    
    def get_cover(self, song, try_web=True):
        default_image_path = self.DEFAULT_COVER
        album = self.get_cover_search_str(song)
        image_path = get_cache_file("cover/%s.jpg" % album)
        image_path_disable = get_cache_file("cover/%s.jpg.#disable#" % album)

        if  (not song.get_str("title") and not song.get_str("album")) or os.path.exists(image_path_disable) or image_path in self.COVER_TO_SKIP:
            return default_image_path
                        
        # Cover already exist.
        if os.path.exists(image_path):
            try:
                _pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(image_path, COVER_SIZE["x"], COVER_SIZE["y"])
            except gobject.GError:    
                os.unlink(image_path)
            else:    
                return image_path
        
        # Retrieve cover from mp3 tag
        if song.get_scheme() == "file" and song.get_ext() in [".mp3", ".tta"]:
            found = False
            fp = None
            try:
                fp = file(image_path, "wb+")
                tag = ID3(song.get_path())
                for frame in tag.getall("APIC"):
                    found = True
                    fp.write(frame.data)
                    fp.flush()
                    fp.seek(0, 0)
            except:    
                if fp:
                    fp.close()
            else:        
                if fp:
                    fp.close()
                if found and self.cleanup_cover(song, image_path):
                    return image_path
                    
        # Search in local directory of the file.        
        if song.get("uri") != None and song.get_scheme() == "file":       
            song_dir = song.get_dir()
            if os.path.exists(song_dir):
                list_file = os.listdir(song_dir)
                for pattern in COVER_PATTERNS:
                    matches = fnmatch.filter(list_file, pattern)
                    if matches:
                        matches = sorted(matches, lambda a,b : (len(a) - len(b)) * 10 + cmp(a, b))
                        if self.cleanup_cover(song, song_dir + "/" + matches[0], image_path):
                            return image_path

        if not config.getboolean("setting", "offline") and try_web:                
            try:
                ret = False
                
                # try url cover tag
                if song.get("album_cover_url"):
                    ret = utils.download(song.get("album_cover_url"), utils.get_uri_from_path(image_path))
                    if ret and self.cleanup_cover(song, image_path):
                        return image_path
                    
                cover_img_url = download_album_cover(album)    
                if cover_img_url:
                    ret = utils.download(cover_img_url, image_path)
                    if ret and self.cleanup_cover(song, image_path):
                        return image_path
                
            except:        
                pass
            self.COVER_TO_SKIP.append(image_path)
            
        # No cover found    
        self.remove_cover(song)    
        if try_web:
            self.logdebug("cover not found %s (web: %s)", image_path, try_web)
        return default_image_path    
    
    def remove_cover(self, song):
        image_path = self.get_cover_path(song)
        if image_path in self.COVER_TO_SKIP:
            del self.COVER_TO_SKIP[self.COVER_TO_SKIP.index(image_path)]
            
        if os.path.exists(image_path):    
            os.unlink(image_path)
        self.remove_pixbuf_from_cache(song.get_str("album"))    
        
    def cleanup_cover(self, song, old_path, path=None):    
        if not path:
            path = old_path
        if not os.path.exists(old_path):    
            return False
        
        try:
            pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(old_path, COVER_SAVE_SIZE["x"], COVER_SAVE_SIZE["y"])
        except gobject.GError:    
            return False
        else:
            # Check cover is not a big black image
            str_pixbuf = pixbuf.get_pixels()
            if str_pixbuf.count("\x00") > len(str_pixbuf)/2 or str_pixbuf.count("\xff") > len(str_pixbuf)/2 : 
                return False
            else:
                if os.path.exists(path): os.unlink(path)
                pixbuf.save(path, "jpeg", {"quality":"85"})
                del pixbuf  
                
                # Change property album to update UI
                MediaDB.set_property(song, {"album" : song.get("album")})
                return True
            
            
            
def download_album_cover(keywords):            
    douban_search_api = 'http://api.douban.com/music/subjects?q={0}&start-index=1&max-results=2'
    douban_cover_pattern = '<link href="http://img(\d).douban.com/spic/s(\d+).jpg" rel="image'
    douban_cover_addr = 'http://img{0}.douban.com/spic/s{1}.jpg'
    
    request = douban_search_api.format(quote(keywords))
    result = urlopen(request).read()
    if not len(result):
        return False
    
    match = re.compile(douban_cover_pattern, re.IGNORECASE).search(result)
    if match:
        return douban_cover_addr.format(match.groups()[0], match.groups()[1])
    else:
        return False

CoverManager =  DeepinCoverManager()