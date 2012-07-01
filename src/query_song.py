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
import re

from pyquery import PyQuery as pq

search_song_url = 'http://mp3.baidu.com/m?word={0}&lm=-1&f=ms&tn=baidump3&ct=134217728&lf=&rn='

def convert_encoding(content):
    if not content:
        return ""
    else:
        return content.encode("utf-8", "ingnore")

def repair_url(invaild_url):    
    first_repair = invaild_url.replace("+", "").replace("'", "").replace('"', "").replace(" ", "")
    return first_repair.replace("zhangmenshiting.","zhangmenshiting2.")
    
def filter_url(vaild_url):
    if "zhangmenshiting2" in vaild_url and "xcode" in vaild_url:
        return True
    return False

def search_song_from_ting(song_name):
    quote_song_name = urllib.quote(song_name.decode("utf-8").encode("gbk"))
    d = pq(url=search_song_url.format(quote_song_name))
    results = []
    try:
        title_elements = d("div#tingResults td.second em")
        artist_elements = d("div#tingResults td.third em")
        album_elements = d("div#tingResults td.fourth a")
        type_elements = d("div#tingResults td.eighth span")
        size_elements = d("div#tingResults td.seventh span")
        down_elements = d("div#tingResults td.down a.btn_replace")
        
    except Exception, e:        
        print e
    else:    
        title_contents = map(convert_encoding, [ element.text_content() for element in title_elements ])
        artist_contents = map(convert_encoding, [ element.text_content() for element in artist_elements ])
        album_contents = map(convert_encoding, [ element.get("title") for element in album_elements ])
        type_contents = map(convert_encoding, [ element.text_content() for element in type_elements ])
        size_contents = map(convert_encoding, [ element.text_content() for element in size_elements ])
        down_contents = map(convert_encoding, [ element.get("href") for element in down_elements])
        for i, title in enumerate(title_contents): 
            results.append(
                {
                    "title" : title,
                    "artist": artist_contents[i],
                    "album" : album_contents[i],
                    "type"  : type_contents[i],
                    "size"  : size_contents[i],
                    "down"  : down_contents[i],
                    "from"  : "ting"
                    }
                )
        return results    
    
        
def search_song_from_baidu(song_name):
    quote_song_name = urllib.quote(song_name.decode("utf-8").encode("gbk"))
    d = pq(url=search_song_url.format(quote_song_name))
    results = []
    try:
        title_elements = d("div#songResults td.second a")
        artist_elements = d("div#songResults td.third span")
        album_elements = d("div#songResults td.fourth a")
        type_elements = d("div#songResults td.eighth span")
        size_elements = d("div#songResults td.seventh span")
        down_elements = d("div#songResults td.down a.btn_replace")
        
    except Exception, e:        
        print e
        
    else:    
        title_contents = map(convert_encoding, [ element.text_content() for element in title_elements ])
        artist_contents = map(convert_encoding, [ element.text_content() for element in artist_elements ])
        album_contents = map(convert_encoding, [ element.get("title") for element in album_elements ])
        type_contents = map(convert_encoding, [ element.text_content() for element in type_elements ])
        size_contents = map(convert_encoding, [ element.text_content() for element in size_elements ])
        down_contents = map(convert_encoding, [ element.get("href") for element in down_elements])
        for i, title in enumerate(title_contents): 
            if ",,," in down_contents[i]:
                from_type = "any"
            else:    
                from_type = "baidu"
                
            results.append(
                {
                    "title" : title,
                    "artist": artist_contents[i],
                    "album" : album_contents[i],
                    "type"  : type_contents[i],
                    "size"  : size_contents[i],
                    "down"  : down_contents[i],
                    "from"  : from_type
                    }
                )
        return results    
    
    
def fetch_downlink_from_ting(query_url):   
    d = pq(url=query_url)
    try:
        d = pq(url=query_url)    
        down_field = d("div.operation a.btn-download")
        song_url = 'http://ting.baidu.com' + down_field.attr('href').encode("utf-8","ingnore")
    except Exception, e:    
        print e
        return ""
    else:    
        return song_url
    
def fetch_downlink_from_baidu(query_url):    
    d = pq(url=query_url)
    try:
        d = pq(url=query_url)    
        down_field = d("a#downlink")
        song_url = 'http://ting.baidu.com' + down_field.attr('href').encode("utf-8","ingnore")
    except Exception, e:    
        print e
        return ""
    else:    
        return song_url

def fetch_downlink_from_any(query_url):    
    web_contents = urllib.urlopen(query_url).read()
    filter_pattern = re.compile(r"urlM\((.*?)\)")
    results = filter_pattern.findall(web_contents)
    if results:
        filter_results = filter(filter_url, map(repair_url, results))
        if len(filter_results) > 0:
            return filter_results[0]
    return ""    

def multi_ways_query_song(query_name):    
    query_url_list = search_song_from_ting(query_name)
    if query_url_list:
        return query_url_list
    else:
        return search_song_from_baidu(query_name)

if __name__ == "__main__":
    query_url_list = search_song_from_ting(sys.argv[1])
    if query_url_list:
        for query_url in query_url_list:
            print "*************************************"
            for key, value in query_url.items():
                print key,":" ,value
            print "*************************************"    
    else:        
        query_url_baidu = search_song_from_baidu(sys.argv[1])
        if query_url_baidu:
            for query_url in query_url_baidu:
                print "*************************************"
                for key, value in query_url.items():
                    print key,":" ,value
                print "*************************************"    
