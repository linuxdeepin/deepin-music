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

import utils

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
        partial="".join( (i for i in lrc if (ord(i) < 128 and ord(i) != 0) ) )
	return bool(re.search('\[\d{1,}:\d{1,}.*?\]',partial))
        
    def tokenize(self, content):    
	content=tuple( (i.lower() for i in re.findall("\w+",content) if i.lower() not in ('a','an','the')) )
	return content
        
    def similarity(self, string1, string2):
	if string1.lower() == string2.lower():
		return 1
	string1=self.tokenize(string1.lower())
	string2=self.tokenize(string2.lower())
	count = len(tuple( (i for i in string1 if i in string2) ))
	return float(count)/max((len(string2), 1))
    
    def cmp_result(self, result, comp):
        value = self.similarity(result[0], comp[0])
        value += self.similarity(result[1], comp[1])
        value = value / 2
        return value
    
    def order_results(self, results, artist, title):
	comp = [artist,title]
	sorted_list = map(lambda x: x[1],reversed(sorted(((self.cmp_result(comp,i),i) for i in results))))
	return sorted_list
        # cmp_list = map(lambda (i, c): (i, self.cmp_result(comp, c)), enumerate(results))
        # sorted_list = sorted(cmp_list, key=lambda i: i[1], reverse=True)
        # results = map(lambda i: results[i[0]], sorted_list)
    
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
        
        
ttplayer_client = ttpClient()        
DOWNLOAD_URL = "http://ttlrcct.qianqian.com/dll/lyricsvr.dll?dl?Id=%d&Code=%d&uid=01&mac=%012x"        
PREFIX_SEARCH_URL = "http://ttlrcct.qianqian.com/dll/lyricsvr.dll?sh?Artist=%s&Title=%s&Flags=0"

        
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
                entities={'&apos;':'\'','&quot;':'"','&gt;':'>','&lt;':'<','&amp;':'&'}
                
                for ii in entities:
                    _id     = _id.replace(ii, entities[ii])
                    _artist = _artist.replace(ii, entities[ii])
                    _title  = _title.replace(ii, entities[ii])
            except:    
                pass
            else:
                codes = ttplayer_client.CodeFunc(int(_id), (_artist+_title))
                download_url = DOWNLOAD_URL % (int(_id), codes, random.randint(0, 0xFFFFFFFFFFFF))
                parser_list.append((_artist, _title, download_url, self.net_encoder))
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
                return self.order_results(lrc_list, artist, title)
            
ttplayer_engine = TTPlayer()

SOSO_DOWNLOAD_URL = 'http://cgi.music.soso.com/fcgi-bin/fcg_download_lrc.q?song=%s&singer=%s&down=1' 
SOSO_PREFIX_SEARCH_URL = 'http://cgi.music.soso.com/fcgi-bin/m.q?w='

class SOSO(Engine):
    def __init__(self, proxy=None, locale="utf-8"):
        super(SOSO, self).__init__(proxy, locale)
        self.net_encoder = "gb18030"
        
    def change_url_to_gb(self, info):    
        address = unicode(info, self.locale).encode(self.net_encoder)
        return address
    
    def parser(self, content):
        parser_list = []
        for each_lrc in content:
            result = re.search('title=\'(.*?)\'.*?title="(.*?)">', each_lrc, re.S)
            try:
                _title = result.group(1)
                _artist = re.sub('<font.*?>|</font>|<strong>|</strong>', '', result.group(2))
                
                _url = SOSO_DOWNLOAD_URL % (urllib.quote_plus(self.change_url_to_gb(_title)), 
                                            urllib.quote_plus(self.change_url_to_gb(_artist)))
                
            except:    
                pass
            else:
                parser_list.append([_artist, _title, _url, self.net_encoder])
        return parser_list        
    
    def request(self, artist, title):
        url1_pre = "%s+%s" % (self.change_url_to_gb(title), self.change_url_to_gb(artist))
        url1 = urllib.quote_plus(url1_pre)
        url2 = '&source=1&t=7'
        url = SOSO_PREFIX_SEARCH_URL + url1 + url2
        
        try:
            fp = urllib.urlopen(url, None, self.proxy)
            info_gb = fp.read()
            fp.close()
            
        except IOError:    
            return None
        else:
            try:
                tmp = unicode(info_gb, self.net_encoder).encode("utf-8")
            except:    
                tmp = info_gb
            tmp_list = re.findall(r'class="title_song">.*?class="song_ablum">', tmp, re.S)

            if len(tmp_list) == 0:
                return None
            else:
                return self.order_results(self.parser(tmp_list), artist, title)
        

soso_engine = SOSO()            


DUOMI_SEARCH_URL = "http://search.duomiyy.com/search?t=sealrc&name=%s&ar=%s&pi=20"
DUOMI_DOWNLOAD_URL = "http://lyric.duomiyy.com/down/%d.lrc"

class DUOMI(Engine):
    def __init__(self, proxy=None, locale="utf-8"):
        super(DUOMI, self).__init__(proxy, locale)
        self.net_encoder = "gbk"
        
    def parser(self, content):    
        parser_list = []
        for each_item in content:
            try:
                _artist = each_item["sartist"]
                if each_item["sver"].strip():
                    _title = "%s(%s)" % (each_item["sname"], each_item["sver"])
                else:    
                    _title = each_item["sname"]
                _url = DUOMI_DOWNLOAD_URL % each_item["llrc"]    
            except:        
                pass
            else:
                parser_list.append([_artist, _title, _url, self.net_encoder])
        return parser_list        
            
    def request(self, artist, title):
        artist_quote = urllib.quote_plus(unicode(artist, self.locale).encode(self.net_encoder))
        title_quote = urllib.quote_plus(unicode(title, self.locale).encode(self.net_encoder))
        url = DUOMI_SEARCH_URL % (title_quote, artist_quote)
        
        try:
            fp = urllib.urlopen(url, None, self.proxy)
            info_utf8 = fp.read()
            fp.close()
            
        except IOError:    
            return None
        else:
            try:
                raw_dict = eval(info_utf8)
            except:    
                return None
            else:
                if "item" in raw_dict:
                    if len(raw_dict["item"]) < 1:
                        return None
                    else:
                        return self.order_results(self.parser(raw_dict["item"]), artist, title)
                return None    
        
duomi_engine = DUOMI()        
        
        
class YOUDAO(Engine):
    def __init__(self, proxy=None, locale="utf-8"):
        super(YOUDAO, self).__init__(proxy, locale)
        self.net_encoder = "utf-8"
        self.return_num = 3
        
    def parser(self, content):    
        parser_list = []
        for each_lrc in content:
            result = re.search('<!--title start-->(.*?)<!--title end-->.*?<!--artist start-->(.*?)<!--artist end-->.*?<!--lyric-download-link start-->(.*?)target', each_lrc)
            try:
                _title = re.sub('</font>|</a>', '', result.group(1))
                _title = re.sub('<.*>', '', _title)
                _artist = re.sub('</font>|</a>', '', result.group(2))
                _artist = re.sub('<.*>', '', _artist)
                _url = 'http://mp3.youdao.com'+re.sub('<a href=\"|\"| ', '', result.group(3))
            except:    
                pass
            else:
                parser_list.append([_artist, _title, _url])
              
        return parser_list        
    
    def request(self, artist, title):
        url1='http://mp3.youdao.com/search?q=%s+%s' % (title, artist)
        url2='&start=0&ue=utf8&keyfrom=music.nextPage&t=LRC&len=%s' % self.return_num
        url = url1 + url2
        
        try:
            fp = urllib.urlopen(url, None, self.proxy)
            info_utf8 = fp.read()
            fp.close()
        except IOError:    
            return None
        else:
            tmp_list = re.findall('<div class="info p90">.*?</a></div>', info_utf8)
            if len(tmp_list) == 0:
                return None
            else:
                return self.parser(tmp_list)
        
youdao_engine = YOUDAO()            
