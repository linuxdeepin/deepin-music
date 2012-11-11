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

import os
import urllib
import urllib2
import urlparse
import cookielib
import time
import random
    
import socket    
socket.setdefaulttimeout(40) # 40s

import utils
from logger import Logger
from song import Song
from config import config

TAGS_DOUBAN_KEYS = {
    "album"       : "album_url",
    "url"         : "uri",
    "albumtitle"  : "album",
    "public_time" : "date",
    "length"      : "#duration",
    "picture"     : "album_cover_url"
    }

class DoubanFM(Logger):
    def __init__(self):
        cookie_file = utils.get_cookie_file("anonymous")
        cj = cookielib.LWPCookieJar(cookie_file)
        cookie_handler = urllib2.HTTPCookieProcessor(cj)
        opener = urllib2.build_opener(cookie_handler)
        opener.addheaders = [
            ('User-agent', 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.4 ' \
             '(KHTML, like Gecko) Chrome/22.0.1229.94 Safari/537.4'),]
        self.cookiejar = cj
        if os.path.isfile(cookie_file):
            self.cookiejar.load(ignore_discard=True, ignore_expires=True)
        self.opener = opener
        self.explore_data = {}
        self.mine_data = {}                
        self.username = ""
        self.password = ""
        self.__uid = ""
        self.__channel = "2"
        
    def get_public_params(self, type_name=None):    
        params = {}
        for i in ['aid', 'channel', 'du', 'h', 'r', 'rest', 'sid', 'type', 'uid']:
            params[i] = ""
            
        params['r'] = random.random
        params['uid'] = self.__uid
        params['channel'] = self.__channel

        if type_name is not None:
            params['type'] = type_name
        return params
    
    def set_login_info(self, username, password):    
        self.username = username
        self.password = password
        
    def load_user_cookie(self):    
        cookie_file = utils.get_cookie_file(self.username)
        if os.path.isfile(cookie_file):
            self.cookiejar.clear()
            self.cookiejar.filename = cookie_file
            self.cookiejar.load(ignore_discard=True, ignore_expires=True)
        
    def check_login(self, load_cookie=True, stage=0):
        # load cookie.
        if load_cookie:
            self.load_user_cookie()
        req = urllib2.Request("http://douban.fm/")
        ret = self.opener.open(req)
        ret.read() # Fix
        
        ret = self.get_recent_chls()
        if ret.get("status", False):
            self.__uid = config.get("douban", "uid", "")
            self.cookiejar.save()
            self.loginfo("Login check success!")
            return True
            
        if stage >= 2:
            self.loginfo("Login check failed!")
            return False

        captcha_id = self.new_captcha()
        params = {}        
        params["captcha_id"] = captcha_id
        params["captcha_solution"] = self.get_login_captcha(captcha_id)
        params["source"] = "radio"
        params["alias"] = self.username
        params["form_password"] = self.password
        params["remember"] = "on"
        # params["task"] = "sync_channel_list"
        req = urllib2.Request("http://douban.fm/j/login",
                              data=urllib.urlencode(params))
        ret = self.opener.open(req).read()
        ret_data =  utils.parser_json(ret)
        user_info = ret_data.get("user_info", None)
        if user_info:
            self.__uid = user_info["id"]
            self.cookiejar.save()
            config.set("douban", "uid", self.__uid)
            config.write()
            self.loginfo("Login check success!")
            return True
        self.loginfo("login info: %s", ret)
        
        # Begin second login check..
        if stage == 0:
            self.loginfo("Begin second login check..")
        elif stage == 1:    
            self.loginfo("Begin three login check..")
        return self.check_login(load_cookie=False, stage=stage+1)
    
    def get_login_captcha(self, captcha_id):
        url = "http://douban.fm/misc/captcha?size=m&id=%s" % captcha_id
        pic_image = utils.get_cache_file("pic")
        if utils.download(url, pic_image):
            self.loginfo("Verify code pic download ok!")
            return raw_input("piz input code > ").strip()    
    
    def new_captcha(self):
        req = urllib2.Request("http://douban.fm/j/new_captcha")
        ret = self.opener.open(req).read()
        return ret.strip("\"")
    
    def api_request(self, url, method="GET", extra_data=dict(), retry_limit=2,  **params):
        data = {}
        data.update(extra_data)
        data.update(params)
        for key in data:
            if callable(data[key]):
                data[key] = data[key]()
            if isinstance(data[key], (list, tuple, set)):
                data[key] = ",".join(map(str, list(data[key])))
            if isinstance(data[key], unicode):    
                data[key] = data[key].encode("utf-8")
                
        if method == "GET":        
            if data:
                query = urllib.urlencode(data)
                url = "%s?%s" % (url, query)
            req = urllib2.Request(url)
        elif method == "POST":
            body = urllib.urlencode(data)
            req = urllib2.Request(url, data=body)
            
        self.logdebug("API request url: %s", url)    
        start = time.time()    
        try:
            ret = self.opener.open(req)
        except Exception, e:    
            if retry_limit == 0:
                self.logdebug("API request error: url=%s error=%s",  url, e)
                return dict(result="network_error")
            else:
                retry_limit -= 1
                return self.api_request(url, method, extra_data, retry_limit, **params)
            
        raw = ret.read()    
        data = utils.parser_json(raw)       
        self.logdebug("API response: %s TT=%.3fs", data, time.time() - start )
        return data
    
    def explore_request(self, api, method="GET", extra_data=dict(), retry_limit=2,  **params):    
        url = urlparse.urljoin("http://douban.fm/j/explore/", api)
        params_data = self.explore_data.copy()
        params_data.update(extra_data)
        return self.api_request(url, method, params_data, retry_limit, **params)
    
    def mine_request(self, method="GET", extra_data=dict(), retry_limit=2,  **params):
        url = "http://douban.fm/j/mine/playlist"
        return self.api_request(url, method, extra_data, retry_limit, **params)
    
    def get_promotion_chls(self):
        return self.explore_request("promotion_chls")
    
    def get_recent_chls(self):
        return self.explore_request("recent_chls")
    
    def get_genre_chls(self, genre_id, start=0, limit=20):
        params = {"query" : "genre_id:%d" % genre_id, 
                  "start" : start, "limit" : limit}
        return self.explore_request("search", extra_data=params)
    
    def get_uptrending_chls(self, start=0, limit=20):
        return self.explore_request("up_trending_channels", start=start, limit=limit)
    
    def get_hot_chls(self, start=0, limit=20):
        return self.explore_request("hot_channels", start=start, limit=limit)
    
    def get_search_chls(self, name, start=0, limit=20):
        params = {"query" : urllib.quote(name), 
                  "start" : start, "limit" : limit}
        return self.explore_request("search", extra_data=params)
    
    def new_playlist_no_user(self, channel_id):
        params = {"type" : "n", "sid" : "", "channel" : channel_id}
        ret = self.mine_request(extra_data=params)
        return self.json_to_deepin_songs(ret)
    
    def json_to_deepin_songs(self, ret):
        douban_songs = ret.get("song", None)
        if douban_songs is not None:
            if len(douban_songs) > 0:
                return map(self.parser_song, douban_songs)
        return None    
    
    def load_channels(self):
        req = urllib.Request('http://www.douban.com/j/app/radio/channels')
        ret = self.opener.open(req)
        return utils.parser_json(ret)
        
    def parser_song(self, douban_song):    
        for douban_tag , tag in TAGS_DOUBAN_KEYS.iteritems():
            if douban_song.has_key(douban_tag):
                douban_song[tag] = douban_song[douban_tag]
                del douban_song[douban_tag]
        for key, value in douban_song.iteritems():        
            if key == "album_url":
                douban_song[key] = urlparse.urljoin("http://music.douban.com", value)
            if key in "album title company".split():    
                douban_song[key] = value.encode("utf-8", "ignore")
        song = Song()        
        song.init_from_dict(douban_song)
        song.set_type("douban")
        return song     
    
    def __format_list(self, sidlist, verb=None):
        """
        for sidlist with ite verb status
        """
        if sidlist is None or len(sidlist) == 0:
            return ''
        else:
            if verb is not None:
                return ''.join(map(lambda s: '|'+str(s)+':'+str(verb), sidlist))
            else:
                return ''.join(map(lambda s: '|'+str(s), sidlist))
            
    def new_playlist(self, history=[]):        
        params = self.get_public_params(type_name='n')
        params['h'] = self.__format_list(history, True)
        ret = self.mine_request(extra_data=params)
        return self.json_to_deepin_songs(ret)
    
    def del_song(self, sid, aid, rest=[]):
        """
        delete a song from your playlist
        * sid - song id
        * aid - album id
        * rest - rest song ids in current playlist
        """
        params = self.get_public_params(type_name="b")
        params['sid'] = sid
        params['aid'] = aid
        params['rest'] = self.__format_list(rest)
        
        ret = self.mine_request(extra_data=params)
        return self.json_to_deepin_songs(ret)
    
    def fav_song(self, sid, aid):
        """
        mark a song as favorite
        * sid - song id
        * aid - album id
        """
        params = self.get_public_params(type_name='r')
        params['sid'] = sid
        params['aid'] = aid
        self.mine_request(extra_data=params)
        
    def unfav_song(self, sid, aid):
        """
        unmark a favorite song
        * sid - song id
        * aid - album id
        """
        params = self.get_public_params('u')
        params['sid'] = sid
        params['aid'] = aid
        self.mine_request(extra_data=params)
        
    def skip_song(self, sid, aid, history=[]):
        """
        skip a song, tell douban that you have skipped the song.
        * sid - song id
        * aid - album id
        * history - your playlist history(played songs and skipped songs)
        """
        params = self.get_public_params('s')
        params['h'] = self.__format_list(history[:50])
        params['sid'] = sid
        params['aid'] = aid
        ret = self.mine_request(extra_data=params)
        return self.json_to_deepin_songs(ret)
        
    def played_song(self, sid, aid, du=0):
        """
        tell douban that you have finished a song
        * sid - song id
        * aid - album id
        * du - time your have been idle
        """
        params  = self.get_public_params('e')
        params['sid'] = sid
        params['aid'] = aid
        params['du'] = du
        self.mine_request(extra_data=params)

    def played_list(self, sid, history=[]):
        """
        request more playlist items
        * history - your playlist history(played songs and skipped songs)
        """
        params = self.get_public_params('p')
        params['h'] = self.__format_list(history[:50])
        params['sid'] = sid
        
        ret = self.mine_request(extra_data=params)
        return self.json_to_deepin_songs(ret)
        
if __name__ == "__main__":    
    douban = DoubanFM()
    douban.set_login_info("username", "password")
    douban.check_login()
    print douban.new_playlist()
