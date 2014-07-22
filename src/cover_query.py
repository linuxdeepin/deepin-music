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

import urllib
import urlparse
from pyquery import PyQuery


import time
import utils
from logger import Logger
from mycurl import MyCurl, CurlException, public_curl
from deepin_utils.net import is_network_connected

class PosterLib(Logger):
    
    def __init__(self):
        headers = ['User-agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.4 ' \
                       '(KHTML, like Gecko) Chrome/22.0.1229.94 Safari/537.4',]
        
        self.mycurl = MyCurl(header=headers)
        self.ting_public_data = {"format" : "json"}
        
    def api_request(self, url, method="GET", extra_data=dict(), retry_limit=2,  **params):
        ret = None
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
                
        start = time.time()        
        try:        
            if method == "GET":        
                if data:
                    query = urllib.urlencode(data)
                    url = "%s?%s" % (url, query)
                ret = self.mycurl.get(url)
            elif method == "POST":
                body = urllib.urlencode(data)
                ret = self.mycurl.post(url, body)
                
        except CurlException, e:        
            if retry_limit == 0:
                self.logdebug("API request error: url=%s error=%s",  url, e)
                return dict(result="network_error")
            else:
                retry_limit -= 1
                return self.api_request(url, method, extra_data, retry_limit, **params)
            
        data = utils.parser_json(ret)       
        self.logdebug("API response %s: TT=%.3fs", url,  time.time() - start )
        return data
        
    def ting_request(self, method="GET", extra_data=dict(), retry_limit=2, **params):    
        
        url = "http://tingapi.ting.baidu.com/v1/restserver/ting"
        params_data = self.ting_public_data.copy()
        params_data.update(extra_data)
        return self.api_request(url, method, extra_data, retry_limit, **params)
    
    def search_common(self, query="", page_size=1, page_no=1):
        params = {"method" : "baidu.ting.search.common",
                  "query" : query,
                  "page_size" : page_size, 
                  "page_no" : page_no}
        
        return self.ting_request(extra_data=params)
    
    def get_album_info(self, album_id):              
        params = {"method" : "baidu.ting.album.getAlbumInfo",
                  "album_id" : album_id}
        return self.ting_request(extra_data=params)
    
    def get_album_cover(self, keywords):
        search_result = self.search_common(keywords)
        try:
            album_id = search_result.get("song_list", [{}])[0].get("album_id", None)
        except:    
            return False
        
        if album_id is None:
            return False
        
        album_info = self.get_album_info(album_id)
        return album_info.get("albumInfo", {}).get("pic_big", False)
    
    def query_lrc_info(self, artist, title):
        query = "%s %s" % (artist, title)
        search_result = self.search_common(query)
        lrc_infos = []
        if search_result:
            song_lists = search_result.get("song_list", None)
            if song_lists:
                for song in song_lists:
                    title = song.get("title", "").replace("<em>", "").replace("</em>", "") or title
                    artist = song.get("author", "").replace("<em>", "").replace("</em>", "") or artist
                    url = song.get("lrclink", None)
                    if url:
                        url = urlparse.urljoin("http://music.baidu.com/", url)
                        lrc_infos.append((artist, title, url))
        return lrc_infos                
                        
    
poster = PosterLib()    

def multi_query_artist_engine(artist_name):
    if not is_network_connected():
        return None
    if artist_name.find(",") != -1:
        artist_name = artist_name.split(",")[0]
    quote_artist_name = urllib.quote(artist_name)
    ting_result = query_artist_cover_from_ting(artist_name)
    if ting_result:
        return ting_result
    return query_artist_cover_from_xiami(quote_artist_name)

def multi_query_album_engine(artist_name, album_name):
    quote_artist_name = urllib.quote(artist_name)
    quote_album_name = urllib.quote(album_name)
    ting_result = poster.get_album_cover("%s %s" % (artist_name, album_name))
    if ting_result:
        return ting_result
    return query_album_cover_from_xiami(quote_artist_name, quote_album_name)

def query_artist_cover_from_ting(keywords):    
    results = poster.search_common(keywords)
    artist_cover = results.get("artist", {}).get("avatar", {}).get("big", False)
    return artist_cover

def query_artist_cover_from_xiami(artist_name):
    xiami_search_url = "http://www.xiami.com/search?key={0}&pos=1"
    search_url = xiami_search_url.format(artist_name)
    html = public_curl.get(search_url)
    
    try:
        search_query = PyQuery(html)
    except Exception, e:    
        return False
    else:
        try:
            artist_div_block = search_query("div.artistBlock_list div.artist_item100_block p.buddy a.artist100 img")
            artist_href = artist_div_block.attr("src")
            if not artist_href: return False
            # artist_url = "%s%s" % (xiami_domain, artist_href)
            # artist_query = PyQuery(url=artist_url)
            # cover_url = artist_query("a#cover_lightbox").attr("href").encode("utf-8", "ingnore")
            return artist_href.replace("_1", "_2")
        except Exception, e:
            return False
        
def query_album_cover_from_xiami(artist_name, album_name):    
    if not is_network_connected():
        return None
    
    if not artist_name and not album_name:
        return False
    xiami_album_search_url = 'http://www.xiami.com/search/album?key=' + artist_name + '+' + album_name
    html = public_curl.get(xiami_album_search_url)
    try:
        search_result_object = PyQuery(html)
        album_info_element = search_result_object('div.albumBlock_list div.album_item100_block p.cover a.CDcover100 img')
        info_href_attr = album_info_element.attr('src')
        if not info_href_attr: return False
        return info_href_attr.replace("_1", "_2")
    except:
        return False
