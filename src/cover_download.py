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



XIMMI_SEARCH_URL = "http://www.xiami.com/search?key={0}&pos=1"
XIMMI_DOMAIN = "http://www.xiami.com"

def download_artist_cover(artist_name):
    quote_artist_name = urllib.quote(artist_name)
    search_url = XIMMI_SEARCH_URL.format(quote_artist_name)
    search_query = PyQuery(url=search_url)
    try:
        artist_div_block = search_query("div.artistBlock_list div.artist_item100_block p.buddy a.artist100")
        artist_href = artist_div_block.attr("href")
        artist_url = "%s%s" % (XIMMI_DOMAIN, artist_href)
        artist_query = PyQuery(url=artist_url)
        cover_url = artist_query("a#cover_lightbox").attr("href").encode("utf-8", "ingnore")
        return cover_url
    except Exception, e:
        return False
    
def download_album_cover(artist_name):
    quote_artist_name = urllib.quote(artist_name)
    search_url = XIMMI_SEARCH_URL.format(quote_artist_name)
    search_query = PyQuery(url=search_url)
    try:
        artist_div_block = search_query("div.artistBlock_list div.artist_item100_block p.buddy a.artist100")
        artist_href = artist_div_block.attr("href")
        artist_url = "%s%s" % (XIMMI_DOMAIN, artist_href)
        artist_query = PyQuery(url=artist_url)
        cover_url = artist_query("a#cover_lightbox").attr("href").encode("utf-8", "ingnore")
        return cover_url
    except Exception, e:
        return False

def download_album_cover_from_douban(keywords):            
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
