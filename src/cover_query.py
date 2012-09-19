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

import re
import urllib
from pyquery import PyQuery

def multi_query_artist_engine(artist_name):
    quote_artist_name = urllib.quote(artist_name)
    xiami_result = query_artist_cover_from_xiami(quote_artist_name)
    if xiami_result:
        return xiami_result
    return query_artist_cover_from_ting(quote_artist_name)

def multi_query_album_engine(artist_name, album_name):
    quote_artist_name = urllib.quote(artist_name)
    quote_album_name = urllib.quote(album_name)
    xiami_result = query_album_cover_from_xiami(quote_artist_name, quote_album_name)
    if xiami_result:
        return xiami_result
    return query_album_cover_from_ting(quote_artist_name, quote_album_name)

def query_artist_cover_from_xiami(artist_name):
    xiami_search_url = "http://www.xiami.com/search?key={0}&pos=1"
    xiami_domain = "http://www.xiami.com"
    search_url = xiami_search_url.format(artist_name)
    try:
        search_query = PyQuery(url=search_url)
    except:    
        return False
    else:
        try:
            artist_div_block = search_query("div.artistBlock_list div.artist_item100_block p.buddy a.artist100")
            artist_href = artist_div_block.attr("href")
            if not artist_href: return False
            artist_url = "%s%s" % (xiami_domain, artist_href)
            artist_query = PyQuery(url=artist_url)
            cover_url = artist_query("a#cover_lightbox").attr("href").encode("utf-8", "ingnore")
            return cover_url
        except:
            return False
        
def query_artist_cover_from_ting(artist_name):
    ting_artist_search_url = 'http://ting.baidu.com/search?key=' + artist_name
    try:
        search_result_object = PyQuery(url=ting_artist_search_url)
        artist_info_element = search_result_object('div#target_artist a.avatar')
        info_href_attr = artist_info_element.attr('href')
        if not info_href_attr: return False
        artist_info_url = 'http://ting.baidu.com' + info_href_attr
        artist_info_object = PyQuery(url=artist_info_url)
        artist_picture_element = artist_info_object('div.mod-info-up img')
        artist_picture_url = artist_picture_element.attr('src').encode('utf-8', 'ignore')
        return artist_picture_url
    except:
        return False
    
def query_album_cover_from_xiami(artist_name, album_name):    
    if not artist_name and not album_name:
        return False
    xiami_album_search_url = 'http://www.xiami.com/search/album?key=' + artist_name + '+' + album_name
    try:
        search_result_object = PyQuery(url=xiami_album_search_url)
        album_info_element = search_result_object('div.albumBlock_list div.album_item100_block p.cover a.CDcover100')
        info_href_attr = album_info_element.attr('href')
        if not info_href_attr: return False
        album_info_url = 'http://www.xiami.com' + info_href_attr
        album_info_object = PyQuery(url=album_info_url)
        album_picture_element = album_info_object('a#cover_lightbox')
        album_picture_url = album_picture_element.attr('href').encode('utf-8', 'ignore')
        return album_picture_url
    except:
        return False
    
def query_album_cover_from_ting(artist_name, album_name):    
    if not artist_name and not album_name:
        return False
    ting_album_search_url = 'http://ting.baidu.com/search?key=' + artist_name + '+' + album_name
    try:
        search_result_object = PyQuery(url=ting_album_search_url)
        album_info_element = search_result_object.find('div.song-item').eq(0).find('span.album-title a') 
        info_href_attr = album_info_element.attr("href")
        if not info_href_attr: return False
        album_info_url = 'http://ting.baidu.com' + info_href_attr
        album_info_object = PyQuery(url=album_info_url)
        
        album_picture_element = album_info_object('div.album-cover img')
        temp_picture_url = album_picture_element.attr('src').encode('utf-8', 'ignore')
        temp_picture_url = temp_picture_url.strip()
        if temp_picture_url.startswitch("http:"):    
            album_picture_url = temp_picture_url
        else:    
            album_picture_url = "http:" + temp_picture_url
        return album_picture_url
    except:
        return False
    
def query_album_cover_from_douban(keywords):            
    douban_search_api = 'http://api.douban.com/music/subjects?q={0}&start-index=1&max-results=2'
    douban_cover_pattern = '<link href="http://img(\d).douban.com/spic/s(\d+).jpg" rel="image'
    douban_cover_addr = 'http://img{0}.douban.com/spic/s{1}.jpg'
    
    request = douban_search_api.format(urllib.quote(keywords))
    result = urllib.urlopen(request).read()
    if not len(result):
        return False
    
    match = re.compile(douban_cover_pattern, re.IGNORECASE).search(result)
    if match:
        return douban_cover_addr.format(match.groups()[0], match.groups()[1])
    else:
        return False
