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

import urllib
import random
import os
import re
import gobject
import gtk
import threading
import codecs

from song import Song
from player import Player


class ttpClient(object):
    '''
    privide ttplayer specific function, such as encoding artist and title,
    generate a Id code for server authorizition.
    (see http://ttplyrics.googlecode.com/svn/trunk/crack) 
    '''
    @staticmethod
    def CodeFunc(Id, data):
	'''
	Generate a Id Code
	These code may be ugly coz it is translated
	from C code which is translated from asm code
	grabed by ollydbg from ttp_lrcs.dll.
	(see http://ttplyrics.googlecode.com/svn/trunk/crack) 
	'''
	length = len(data)

	tmp2=0
	tmp3=0

	tmp1 = (Id & 0x0000FF00) >> 8							#右移8位后为x0000015F

	    #tmp1 0x0000005F
	if ( (Id & 0x00FF0000) == 0 ):
	    tmp3 = 0x000000FF & ~tmp1							#CL 0x000000E7
	else:
	    tmp3 = 0x000000FF & ((Id & 0x00FF0000) >> 16)                               #右移16后为x00000001

	tmp3 = tmp3 | ((0x000000FF & Id) << 8)                                          #tmp3 0x00001801
	tmp3 = tmp3 << 8                                                                #tmp3 0x00180100
	tmp3 = tmp3 | (0x000000FF & tmp1)                                               #tmp3 0x0018015F
	tmp3 = tmp3 << 8                                                                #tmp3 0x18015F00
	if ( (Id & 0xFF000000) == 0 ) :
	    tmp3 = tmp3 | (0x000000FF & (~Id))                                          #tmp3 0x18015FE7
	else :
	    tmp3 = tmp3 | (0x000000FF & (Id >> 24))                                     #右移24位后为0x00000000

	#tmp3	18015FE7
        
	i=length-1
	while(i >= 0):
	    char = ord(data[i])
	    if char >= 0x80:
		char = char - 0x100
	    tmp1 = (char + tmp2) & 0x00000000FFFFFFFF
	    tmp2 = (tmp2 << (i%2 + 4)) & 0x00000000FFFFFFFF
	    tmp2 = (tmp1 + tmp2) & 0x00000000FFFFFFFF
	    #tmp2 = (ord(data[i])) + tmp2 + ((tmp2 << (i%2 + 4)) & 0x00000000FFFFFFFF)
	    i -= 1

	#tmp2 88203cc2
	i=0
	tmp1=0
	while(i<=length-1):
	    char = ord(data[i])
	    if char >= 128:
		char = char - 256
	    tmp7 = (char + tmp1) & 0x00000000FFFFFFFF
	    tmp1 = (tmp1 << (i%2 + 3)) & 0x00000000FFFFFFFF
	    tmp1 = (tmp1 + tmp7) & 0x00000000FFFFFFFF
	    #tmp1 = (ord(data[i])) + tmp1 + ((tmp1 << (i%2 + 3)) & 0x00000000FFFFFFFF)
	    i += 1

	#EBX 5CC0B3BA

	#EDX = EBX | Id
	#EBX = EBX | tmp3
	tmp1 = (((((tmp2 ^ tmp3) & 0x00000000FFFFFFFF) + (tmp1 | Id)) & 0x00000000FFFFFFFF) * (tmp1 | tmp3)) & 0x00000000FFFFFFFF
	tmp1 = (tmp1 * (tmp2 ^ Id)) & 0x00000000FFFFFFFF

	if tmp1 > 0x80000000:
	    tmp1 = tmp1 - 0x100000000
	return tmp1
    
    @staticmethod
    def EncodeArtTit(str):
	rtn = ''
	str = str.encode('UTF-16')[2:]
	for i in range(len(str)):
	    rtn += '%02x' % ord(str[i])

	return rtn


class Engine(object):
    def __init__(self, proxy=None, locale="utf-8", check=True):
        
        self.locale = locale
        self.proxy = proxy
        self.net_encoder = None
        self.need_check = check
        
    def request(self, artist, title):    
        raise NotImplementedError("request must be implemented in subclass.")
    
    def valid_lrc(self, lrc):
        partial = ""
        
        for i in lrc:
            if ord(i) < 128 and ord(i) != 0:
                partial += i
        if re.search('\[\d{1,}:\d{1,}.*?\]', partial):        
            return True
        else:
            return False
        
    def tokenize(self, content):    
        content = content.replace('(', ' ').replace(')', ' ')
        content = content.replace('[', ' ').replace(']', ' ')
        content = content.replace('[', ' ').replace(']', ' ')
        ignore = ["a", "an", "the"]
        content = content.split()
        for token in content:
            if token.lower() in ignore:
                content.remove(token)
        return content        
    
    def similarity(self, string1, string2):
        if string1.lower() == string2.lower():
            return 1
        string1 = self.tokenize(string1.lower())
        string2 = self.tokenize(string2.lower())
        count = 0
        for i in string1:
            if i in string2:
                count += 1
        return count / len(string1)        
    
    def cmp_result(self, result, comp):
        value = self.similarity(result[0], comp[0])
        value += self.similarity(result[1], comp[1])
        value = value / 2
        return value
    
    def order_results(self, results, artist, title):
        comp = [artist, title, ""]
        temp = []
        for i in results:
            temp.append([self.cmp_result(comp, i), i])
        temp.sort()    
        temp.reverse()
        results = []
        for i in temp:
            results.append(i[1])
        return results    
    
    def download(self, url):
        try:
            ff = urllib.urlopen(url, None, self.proxy)
            original_lrc = ff.read()
            ff.close()
        except IOError:    
            return (None, True)
        else:
            if self.need_check:
                if self.net_encoder == None or self.net_encoder.startswith("utf-16") or self.net_encoder.startswith("utf-32"):
                    if not self.valid_lrc(original_lrc):
                        original_lrc = None
                    elif not re.search('\[\d{1,}:\d{1,}.*?\]', original_lrc):    
                        original_lrc = None
            return (original_lrc, False)            
        
        
#DOWNLOAD_URL = "http://ttlrcct.qianqian.com/dll/lyricsvr.dll?dl?Id=%d&Code=%d&uid=01&mac=%012x"        
DOWNLOAD_URL = 'http://lrcct2.ttplayer.com/dll/lyricsvr.dll?dl?Id=%d&Code=%d&uid=01&mac=%012x'
#PREFIX_SEARCH_URL = "http://ttlrcct.qianqian.com/dll/lyricsvr.dll?sh?"
PREFIX_SEARCH_URL = 'http://lrcct2.ttplayer.com/dll/lyricsvr.dll?sh?Artist=%s&Title=%s&Flags=0'
ttplayer_client = ttpClient()
        
class TTPlayer(Engine):
    
    def __init__(self, proxy=None, locale="utf-8"):
        super(TTPlayer, self).__init__(proxy, locale)
        self.net_encoder = "utf-8"
        
    def parser(self, content):
        parser_list = []
        for each_lrc in content:
            result  = re.search('id=\"(.*?)\" artist=\"(.*?)\" title=\"(.*?)\"', each_lrc)
            try:
                _id     = result.group(1)
                _artist = result.group(2)
                _title  = result.group(3)
            except:    
                pass
            else:
                codes = ttplayer_client.CodeFunc(int(_id), (_artist+_title))
                download_url = DOWNLOAD_URL % (int(_id), codes, random.randint(0, 0xFFFFFFFFFFFF))
                parser_list.append((_artist, _title, download_url))
        return parser_list        
               
    def request(self, artist, title):    
        convert_artist = ttplayer_client.EncodeArtTit(unicode(artist, self.locale).replace(u' ', '').lower())
        convert_title = ttplayer_client.EncodeArtTit(unicode(title, self.locale).replace(u' ', '').lower())
        url = PREFIX_SEARCH_URL % (convert_artist, convert_title)
        try:
            fp = urllib.urlopen(url, None, self.proxy)
            web_info = fp.read()
            fp.close()
        except IOError:    
            return None
        else:
            tmp_list = re.findall(r'<lrc.*?</lrc>', web_info)
            if len(tmp_list) == 0:
                return None
            else:
                lrc_list = self.parser(tmp_list)
                return lrc_list
            
ttplayer_engine = TTPlayer()

class LrcManager(object):
    
    def __init__(self):
        pass
        
    
if __name__ == "__main__":
    from widget.lyrics_search import SearchUI
    import gtk
    a = SearchUI()
    a.run()
    gtk.main()
        
