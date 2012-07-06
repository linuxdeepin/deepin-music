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
import gobject
import gtk
import fnmatch
import random
from dtk.ui.utils import get_optimum_pixbuf_from_file

from mutagen.id3 import ID3

import utils
from config import config
from logger import Logger
from library import MediaDB
from xdg_support import get_cache_file
from widget.skin import app_theme
from cover_query import multi_query_artist_engine


REINIT_COVER_TO_SKIP_TIME = 100 * 60 * 30

COVER_SIZE = {"x": 58, "y": 58}
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

GENRE_DICT = {
	"blues.png" : ["蓝调", "布鲁斯", "blue"],
	"classical.png" : ["古典", "classical"],
	"country.png" : ["乡村", "兰草", "牛仔", "小酒馆", 
			 "country", "americana", "bluegrass", "cowboy"],
	"easy_listen.png" : ["轻音乐", "异域", "沙发", "管弦乐", 
			     "orchestral", "lounge", "exotica", "easy"],
	"electonic.png" : ["电子", "大节拍", "迪斯科", 
			   "缓拍", "迷幻", "科技",
			   "techno", "trance", "electronic"
			   "disco", "beat"],
	"folk.png" : ["民谣", "政治", "传统", "城市",
		      "political", "traditional", "folk", "chanson"],
	"international.png" : ["国际", "凯尔特", "夏威夷", "印度古典", "戏曲", "世界",
			       "曲艺", "民乐", "民歌", "岛呗", "同人", "演歌",
			       "shima", "enka", 
			       "hawaiian", "celtic", "international"],
	"jazz.png" : ["爵士", "布基伍基", "波普", "摇摆乐", 
		      "swing", "bop", "boogie-woogie", "jazz"],
	"latin.png" : ["拉丁", "巴萨诺瓦", "恰朗加", "伦巴", "萨尔萨", "热带",
		       "bossa", "charanga", "latin", "rumba", "salsa", "tropical"],
	"new_age.png" : ["新世纪", "氛围", "冥想", "放松", "新原音", "器乐独奏", "太空乐",
			 "精神", "部落",
			 "acoustic", "instrumental", "space",
			 "spiritual", "tribal",
			 "meditation", "relaxation", "age", "ambient"],
	"pop_rock.png" : ["摇滚", "流行", "金属", "哥特", "朋克", "rock", "pop", "metal",
			  "britpop", "dance-pop",
			  "folk-rock", "grunge", "nu-metal", "punk", 
			  "post-punk", "post-rock", "psychedelic", 
			  "shoegaze"],
	"rap.png" : ["说唱", "rap"],
	"r_b.png" : ["节奏布鲁斯", "放克", "灵魂", "嘻哈", "r&b", "drum", "doo", "funk", "dotown", 
		     "soul", "urban", "hip-hop"],
	"reggae.png" : ["雷鬼", "雷嘎", "洛克丝黛蒂", "Ska", "大伯", "蒙托", 
			"ska", "rocksteady", "mento", "dub", "reggae"],
	"stage_screen.png" : ["舞台", "电影原声", "原声带", "音乐剧", "舞曲", "脱口秀", "游戏",                              
                              "game", "film", "stage", "musicals", "soundtrack"],
        "vocal.png" : ["声乐", "vocal", "合唱", "人声", "演出", 
                       "harmony", "show"]
	}

GENRE_PATH = ['pop_rock.png',
     'jazz.png',
     'international.png',
     'easy_listen.png',
     'classical.png',
     'latin.png',
     'reggae.png',
     'electonic.png',
     'country.png',
     'blues.png',
     'stage_screen.png',
     'rap.png',
     'r_b.png',
     'new_age.png',
     'folk.png']

class DeepinCoverManager(Logger):
    COVER_PIXBUF = {}
    COVER_TO_SKIP = []
    
    def __init__(self):
        self.default_cover = app_theme.get_theme_file_path("image/cover/default_cover1.png")
        self.all_song_cover = app_theme.get_theme_file_path("image/cover/all_song.png")
        
    def get_cover_search_str(self, song):
        artist = song.get_str("artist")
        title  = song.get_str("title")
        if artist:
            result =  artist
        else:    
            result = title
        return result.replace("/", "")    
    
    def get_default_cover(self, x, y):    
        return gtk.gdk.pixbuf_new_from_file_at_size(self.default_cover, x, y)
    
    def get_all_song_cover(self, x, y):
        return gtk.gdk.pixbuf_new_from_file_at_size(self.all_song_cover, x, y)
    
    def get_pixbuf_from_name(self, query_name, x=None, y=None):
        x = (x or BROWSER_COVER_SIZE["x"])
        y = (y or BROWSER_COVER_SIZE["y"])
        
        filename = get_cache_file("cover/%s.jpg" % query_name.replace("/", ""))
        if os.path.exists(filename):
            try:
                gtk.gdk.pixbuf_new_from_file_at_size(filename, COVER_SIZE["x"], COVER_SIZE["y"])
            except gobject.GError:    
                os.unlink(filename)
                filename = self.default_cover
        else:        
            filename = self.default_cover
        return get_optimum_pixbuf_from_file(filename, x, y)
    
    def has_cover(self, song):            
        cover = self.get_cover(song, False) # todo
        return cover != self.default_cover and os.path.exists(cover)
    
    def get_cover_path(self, song):
        return get_cache_file("cover/" + self.get_cover_search_str(song) + ".jpg")
    
    def get_pixbuf_from_song(self, song, x, y, try_web=True, optimum=True):
        filename = self.get_cover(song, try_web)
        if optimum:
            return get_optimum_pixbuf_from_file(filename, x, y)
        else:
            return gtk.gdk.pixbuf_new_from_file_at_size(filename, x, y)
    
    def get_cover_from_genre(self, genre_info):
        '''Get cover from genre.'''
        for (genre_cover, genre_keywords) in GENRE_DICT.items():
            for genre_keyword in genre_keywords:
                if genre_keyword in genre_info.lower():
                    return genre_cover
    	    
        return None

    def get_pixbuf_from_genre(self, genre):
        cover_file = self.get_cover_from_genre(genre)
        if cover_file:
            return app_theme.get_pixbuf("genre/%s" % cover_file).get_pixbuf()
        else:
            return app_theme.get_pixbuf("genre/other.png").get_pixbuf()
    
    def get_random_pixbuf_from_genre(self):
        random.shuffle(GENRE_PATH)
        return app_theme.get_pixbuf("genre/%s" % GENRE_PATH[1]).get_pixbuf()
    
    def get_cover(self, song, try_web=True):
        default_image_path = self.default_cover
        album = self.get_cover_search_str(song)
        image_path = get_cache_file("cover/%s.jpg" % album)
        image_path_disable = get_cache_file("cover/%s.jpg.#disable#" % album)

        if  (not song.get_str("title") and not song.get_str("album")) or os.path.exists(image_path_disable) or image_path in self.COVER_TO_SKIP:
            return default_image_path
                        
        # Cover already exist.
        if os.path.exists(image_path):
            try:
                gtk.gdk.pixbuf_new_from_file_at_size(image_path, COVER_SIZE["x"], COVER_SIZE["y"])
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
                    
                cover_img_url = multi_query_artist_engine(album)    
                if cover_img_url:
                    ret = utils.download(cover_img_url, image_path)
                    if ret and self.cleanup_cover(song, image_path):
                        return image_path
            except:        
                pass

        # No cover found    
        self.remove_cover(song)    
        if try_web:
            self.logdebug("cover not found %s (web: %s)", image_path, try_web)
        return default_image_path    
    
    def remove_cover(self, song, emit=False):
        image_path = self.get_cover_path(song)
        if os.path.exists(image_path):    
            try:
                os.unlink(image_path)
            except:    
                pass
        if emit:    
            MediaDB.set_property(song, {"album" : song.get("album")})
            
            
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
            
    def change_cover(self, song, new_cover):        
        save_path = self.get_cover_path(song)
        if not os.path.exists(new_cover):
            return False
        try:
            pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(new_cover, COVER_SAVE_SIZE["x"], COVER_SAVE_SIZE["y"])
        except gobject.GError:    
            return False
        else:
            str_pixbuf = pixbuf.get_pixels()
            if str_pixbuf.count("\x00") > len(str_pixbuf)/2 or str_pixbuf.count("\xff") > len(str_pixbuf)/2 : 
                return False
            else:
                if os.path.exists(save_path): os.unlink(save_path)
                pixbuf.save(save_path, "jpeg", {"quality":"85"})
                del pixbuf  
                
                # Change property album to update UI
                MediaDB.set_property(song, {"album" : song.get("album")})
                return True

CoverManager =  DeepinCoverManager()
