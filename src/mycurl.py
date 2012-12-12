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

import pycurl
import StringIO

from logger import Logger

class CurlException(Exception):
    pass

class MyCurl(Logger):
    '''Curl Class'''
    def __init__(self, cookie_file=None, header=None, proxy_host=None, proxy_port=None):
        self.cookie_file = cookie_file
        self.header = header
        self.proxy_host = proxy_host
        self.proxy_port = proxy_port
        
    def set_cookie_file(self, cookie_file):    
        self.cookie_file = cookie_file
    
    def get(self, url, header=None, proxy_host=None, proxy_port=None, cookie_file=None):
        '''
        open url width get method
        @param url: the url to visit
        @param header: the http header
        @param proxy_host: the proxy host name
        @param proxy_port: the proxy port
        '''
        crl = pycurl.Curl()
        #crl.setopt(pycurl.VERBOSE,1)
        crl.setopt(pycurl.NOSIGNAL, 1)

        # set proxy
        # crl.setopt(pycurl.PROXYTYPE, pycurl.PROXYTYPE_SOCKS5)         
        
        rel_proxy_host = proxy_host or self.proxy_host
        if rel_proxy_host:
            crl.setopt(pycurl.PROXY, rel_proxy_host)
            
        rel_proxy_port = proxy_port or self.proxy_port
        if rel_proxy_port:
            crl.setopt(pycurl.PROXYPORT, rel_proxy_port)
            
            
        # set cookie    
        rel_cookie_file = cookie_file or self.cookie_file
        if rel_cookie_file:
            crl.setopt(pycurl.COOKIEFILE, rel_cookie_file)            
            crl.setopt(pycurl.COOKIEJAR, rel_cookie_file)            
            
        # set ssl
        crl.setopt(pycurl.SSL_VERIFYPEER, 0)
        crl.setopt(pycurl.SSL_VERIFYHOST, 0)
        crl.setopt(pycurl.SSLVERSION, 3)
         
        crl.setopt(pycurl.CONNECTTIMEOUT, 10)
        crl.setopt(pycurl.TIMEOUT, 300)
        crl.setopt(pycurl.HTTPPROXYTUNNEL,1)

        rel_header = header or self.header
        if rel_header:
            crl.setopt(pycurl.HTTPHEADER, rel_header)

        crl.fp = StringIO.StringIO()
         
        if isinstance(url, unicode):
            url = str(url)
        crl.setopt(pycurl.URL, url)
        crl.setopt(crl.WRITEFUNCTION, crl.fp.write)
        try:
            crl.perform()
        except Exception, e:
            raise CurlException(e)
            
        crl.close()
        return crl.fp.getvalue()
    
    def post(self, url, data, header=None, proxy_host=None, proxy_port=None, cookie_file=None):
        '''
        open url width post method
        @param url: the url to visit
        @param data: the data to post
        @param header: the http header
        @param proxy_host: the proxy host name
        @param proxy_port: the proxy port
        '''
        crl = pycurl.Curl()
        #crl.setopt(pycurl.VERBOSE,1)
        crl.setopt(pycurl.NOSIGNAL, 1)

        # set proxy
        rel_proxy_host = proxy_host or self.proxy_host
        if rel_proxy_host:
            crl.setopt(pycurl.PROXY, rel_proxy_host)
        rel_proxy_port = proxy_port or self.proxy_port
        if rel_proxy_port:
            crl.setopt(pycurl.PROXYPORT, rel_proxy_port)
            
        # set cookie    
        rel_cookie_file = cookie_file or self.cookie_file
        if rel_cookie_file:
            crl.setopt(pycurl.COOKIEFILE, rel_cookie_file)            
            crl.setopt(pycurl.COOKIEJAR, rel_cookie_file)            
            
        # set ssl
        crl.setopt(pycurl.SSL_VERIFYPEER, 0)
        crl.setopt(pycurl.SSL_VERIFYHOST, 0)
        crl.setopt(pycurl.SSLVERSION, 3)
         
        crl.setopt(pycurl.CONNECTTIMEOUT, 10)
        crl.setopt(pycurl.TIMEOUT, 300)
        crl.setopt(pycurl.HTTPPROXYTUNNEL,1)

        rel_header = header or self.header
        if rel_header:
            crl.setopt(pycurl.HTTPHEADER, rel_header)

        crl.fp = StringIO.StringIO()
         
        crl.setopt(crl.POSTFIELDS, data)  # post data

        if isinstance(url, unicode):
            url = str(url)
        crl.setopt(pycurl.URL, url)
        crl.setopt(crl.WRITEFUNCTION, crl.fp.write)
        try:
            crl.perform()
        except Exception, e:
            raise CurlException(e)
        crl.close()
        return crl.fp.getvalue()
    
    def upload(self, url, data, header=None, proxy_host=None, proxy_port=None, cookie_file=None):
        '''
        open url with upload
        @param url: the url to visit
        @param data: the data to upload
        @param header: the http header
        @param proxy_host: the proxy host name
        @param proxy_port: the proxy port
        '''
        crl = pycurl.Curl()
        #crl.setopt(pycurl.VERBOSE,1)
        crl.setopt(pycurl.NOSIGNAL, 1)

        # set proxy
        rel_proxy_host = proxy_host or self.proxy_host
        if rel_proxy_host:
            crl.setopt(pycurl.PROXY, rel_proxy_host)
        rel_proxy_port = proxy_port or self.proxy_port
        if rel_proxy_port:
            crl.setopt(pycurl.PROXYPORT, rel_proxy_port)
            
        # set cookie    
        rel_cookie_file = cookie_file or self.cookie_file
        if rel_cookie_file:
            crl.setopt(pycurl.COOKIEFILE, rel_cookie_file)            
            crl.setopt(pycurl.COOKIEJAR, rel_cookie_file)            
            
        # set ssl
        crl.setopt(pycurl.SSL_VERIFYPEER, 0)
        crl.setopt(pycurl.SSL_VERIFYHOST, 0)
        crl.setopt(pycurl.SSLVERSION, 3)
         
        crl.setopt(pycurl.CONNECTTIMEOUT, 10)
        crl.setopt(pycurl.TIMEOUT, 300)
        crl.setopt(pycurl.HTTPPROXYTUNNEL,1)
        
        rel_header = header or self.header
        if rel_header:
            crl.setopt(pycurl.HTTPHEADER, rel_header)
            
        crl.fp = StringIO.StringIO()
              
        if isinstance(url, unicode):
            url = str(url)
        crl.setopt(pycurl.URL, url)
        crl.setopt(pycurl.HTTPPOST, data)   # upload file
        crl.setopt(crl.WRITEFUNCTION, crl.fp.write)
        try:
            crl.perform()
        except Exception, e:
            raise CurlException(e)
        crl.close()
        return crl.fp.getvalue()
    
public_curl = MyCurl(header=['User-agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.4 ' \
                                 '(KHTML, like Gecko) Chrome/22.0.1229.94 Safari/537.4',])    
