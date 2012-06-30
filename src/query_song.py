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

import sys
import urllib

from pyquery import PyQuery as pq

search_song_url = 'http://mp3.baidu.com/m?word={0}&lm=-1&f=ms&tn=baidump3&ct=134217728&lf=&rn='

def search_song_urls(song_name):
    quote_song_name = urllib.quote(song_name.decode("utf-8").encode("gbk"))
    results = []
    d = pq(url=search_song_url.format(quote_song_name))
    try:
        query_song_results = d("div#songResults td.down a.btn_replace")
        for query_song_element in query_song_results:
            query_song_href = query_song_element.get("href").encode("utf-8",'ingnore')
            if query_song_href.find(",,,") == -1:
                results.append(query_song_href)
    except Exception, e:        
        print e
    
    return results

def query_down_info(query_url):
    d = pq(query_url)
    query_info = {}
    try:
        title_field = d("div#main div.left span.title")
        query_info["title"] = title_field.text().encode("utf-8", "ingnore")
        
        artist_field = d("div#main div.songinfo-more span.singer a")
        artist_text = artist_field.text()
        if artist_text:
            query_info["artist"] = artist_text.encode("utf-8", "ingnore")
        else:    
            query_info["artist"] = ""
        
        album_field = d("div#main div.songinfo-more span.album a")
        album_text = album_field.text()
        if album_text:
            query_info["album"] = album_text.encode("utf-8", "ingnore")
        else:    
            query_info["album"] = ""
            
        attr_field = d("div#main form#form-download div.format b")    
        query_info["attr"] = attr_field.text().encode("utf-8", "ingnore")
        
        url_field = d("a#downlink")
        query_info["downlink"] = 'http://mp3.baidu.com' + url_field.attr('href').encode("utf-8","ingnore")
        
    except Exception, e:    
        print e
        
    return query_info    


if __name__ == "__main__":
    query_url_list = search_song_urls(sys.argv[1])
    if query_url_list:
        for query_url in query_url_list:
            print query_down_info(query_url)

