#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2013 Deepin, Inc.
#               2011 ~ 2013 Hou ShaoHui
# 
# Author:     Hou ShaoHui <houshao55@gmail.com>
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

import chardet
import sys
import os
import fcntl
import cPickle
import shutil
import gtk
import gobject
import cairo
import math
import locale
import md5
import random
import gio
import glib

import hashlib

try:
    import simplejson as json
except ImportError:    
    import json

import time
from time import mktime, strptime
import threading
import subprocess
import mimetypes
mimetypes.init()


from urllib import quote, unquote, pathname2url
from urlparse import urlparse
from urllib2 import urlopen, Request
from functools import wraps

from logger import newLogger
from constant import DEFAULT_TIMEOUT
import socket
socket.setdefaulttimeout(DEFAULT_TIMEOUT)
import common

from xdg_support import get_cache_file
from mycurl import public_curl, CurlException
from ConfigParser import ConfigParser

logger = newLogger("utils")
fscoding = sys.getfilesystemencoding()

def open_file(path):
    """
        Opens a file or folder using the system configured program
    """
    platform = sys.platform
    if platform == 'win32':
        os.startfile(path)
    elif platform == 'darwin':
        subprocess.Popen(["open", path])
    else:
        subprocess.Popen(["xdg-open", path])

def open_file_directory(path):
    """
        Opens the parent directory of a file, selecting the file if possible.
    """
    f = gio.File(path)
    platform = sys.platform
    if platform == 'win32':
        subprocess.Popen(["explorer", "/select,", f.get_parse_name()])
    elif platform == 'darwin':
        subprocess.Popen(["open", f.get_parent().get_parse_name()])
    else:
        subprocess.Popen(["xdg-open", f.get_parent().get_parse_name()])
        
def open_directory(path):        
    f = gio.File(path)
    platform = sys.platform
    if platform == 'win32':
        subprocess.Popen(["explorer", "/select,", f.get_parse_name()])
    elif platform == 'darwin':
        subprocess.Popen(["open", f.get_parse_name()])
    else:
        subprocess.Popen(["xdg-open", f.get_parse_name()])

def profileit(func):
    """
        Decorator to profile a function
    """
    import hotshot, hotshot.stats
    @wraps(func)
    def wrapper(*args, **kwargs):
        prof = hotshot.Profile("profiling.data")
        res = prof.runcall(func, *args, **kwargs)
        prof.close()
        stats = hotshot.stats.load("profiling.data")
        stats.strip_dirs()
        stats.sort_stats('time', 'calls')
        print ">>>---- Begin profiling print"
        stats.print_stats()
        print ">>>---- End profiling print"
        return res
    return wrapper
        
def get_scheme(uri):
    ''' get uri type, such as 'file://', return 'file'. '''
    if not uri:
        return ""
        
    if uri.rfind("://") == -1:
        return ""
    scheme = uri[:uri.index("://")]
    return scheme.lower()

def make_uri_from_shell_arg(arg):
    if isinstance(arg, unicode):
        arg = arg.encode("utf-8")
    scheme = get_scheme(arg)
    if scheme: return arg
    else: 
        return "file://" + pathname2url(os.path.abspath(os.path.expanduser(arg)))
    
def convert_args_to_uris(args):
    uris = [ make_uri_from_shell_arg(arg) for arg in args if arg and arg.strip() ]
    return uris


def get_ext(uri, complete=True):
    ''' get uri extension, such as '123.mp3', return '.mp3|mp3'. '''
    if uri.rfind(".") == -1:
        return ""
    if get_scheme(uri) in ["http", "https"]:
        if uri.rfind("#") != -1:
            uri = uri[:uri.rindex("#")]
        if uri.rfind("?") != -1:
            uri = uri[:uri.rindex("?")]
    if complete:        
        return uri[uri.rindex("."):].lower()
    else:
        if uri[-1:] == ".": return ""
        return uri[uri.rindex(".")+1:].lower()
    
def get_path_from_uri(uri):    
    ''' get filepath from uri. '''
    if get_scheme(uri) == "file":
        uri = uri.replace("#", "%23")
    return unquote(urlparse(uri)[2])    

def get_unquote_uri(self, uri):
    return unquote(uri)

def get_uri_from_path(path, is_quote=False):
    ''' get uri from filepath. '''
    if get_scheme(path): 
        return path
    if is_quote:
        return "file://" + quote(path)
    else:
        return "file://" + path
        


def realuri(uri):
    '''Return the canonical path of the specified filename.'''
    if get_scheme(uri) == "file":
        return get_uri_from_path(os.path.realpath(get_path_from_uri(uri)))
    else:
        return uri
    
def unescape_string_for_display(value):    
    ''' display of file name for users. '''
    return unquote(value)

def rebuild_uri(header_uri, base_uri):
    ''' return header_uri + base_uri '''
    header_uri = header_uri[:header_uri.rfind("/")]
    if base_uri.find("://") != -1:
        return base_uri
    elif base_uri[0] == "/":    
        return "file://" + quote(base_uri)
    else:
        return header_uri + "/" + quote(base_uri)
    

def unlink(uri):
    ''' Remove a file (same as remove(path)). '''
    if get_scheme != "file":
        return True
    path = get_path_from_uri(uri)
    if os.path.exists(path):
        return os.unlink(path)
    return False

def exists(uri):
    '''  Test whether a path exists.  Returns False for broken symbolic links. '''
    if get_scheme(uri) != "file":
        return True
    path = get_path_from_uri(uri)
    return os.path.exists(path)

def makedirs(uri, mode=0755):
    ''' Super-mkdir; create a leaf directory and all intermediate ones. '''
    path = urlparse(uri)[2]
    if not os.path.exists(path):
        os.makedirs(path, mode)
    
def get_name(uri):
    # return os.path.basename(get_path_from_uri(uri))
    return get_path_from_uri(uri).split("/")[-1]

def get_filename(uri):
    name = get_name(uri)
    return os.path.splitext(name)[0]
        

def is_local_dir(uri):    
    return os.path.isdir(get_path_from_uri(uri))

def read_entire_file(uri):
    ''' Return entire_file content. '''
    data = ""
    if get_scheme(uri) == "file":
        f = file(get_path_from_uri(uri), "r")
        data = f.read()
        f.close()
    else:    
        try:
            data = public_curl.get(uri)
        except CurlException, e:    
            logger.logexception("failed read %s, %s", uri, e)
            data = ""
    return data        
    
def auto_decode(s):    
    ''' auto detect the code and return unicode object. '''
    if isinstance(s, unicode):
        return s
    try:
        return s.decode("gbk")
    except UnicodeError:
        try:
            codedetect = chardet.detect(s)["encoding"]
            return s.decode(codedetect)
        except:    
            return "[Invalid Encoding]"    
    
def fix_charset(s):    
    '''Fix the charset. unicode error'''
    if not s: return ""
    repr_char = repr(s) 
    if repr_char.startswith("u"):
        if repr_char.find("\u") != -1:
            return s.encode("utf-8")
        return auto_decode(eval(repr_char[1:])).encode("utf-8")
    else:
        return s
    
def auto_encode(s, charset=fscoding): 
    ''' auto encode. '''
    if isinstance(s, unicode):
        return s.encode(charset, "replace")
    else:
        return auto_decode(s).encode(charset, "replace")

def get_mime_type(uri):
    ''' get mime type. '''
    if get_scheme(uri) in ["http", "https"]:
        if uri.rfind("#") != -1:
            uri = uri[:uri.rindex("#")]
        if uri.rfind("?") != -1:    
            uri = uri[:uri.rindex("?")]
    mimetype = mimetypes.guess_type(uri)[0]        
    if mimetype: return mimetype
    ext = get_ext(uri)
    if ext == ".pls": mimetype = "audio/x-scpls"
    elif ext == ".m3u": mimetype = "audio/x-mpegurl"
    elif ext == ".asx": mimetype = "video/x-ms-asf"
    else: mimetype = ""
    return mimetype

    
def duration_to_string(value, default="00:00", i=1000):
    ''' convert duration to string. '''
    if not value: return default
    if (value / i) < 1: i = 1
    duration = "%02d:%02d" % (value/(60*i), (value/i) % 60)
    if value/(60*i) / 60 >= 2:
        duration = "%03d:%02d" % (value/(60*i), (value/i) % 60)
    return duration    

def parse_folder(parent_dir):
    ''' return all uri in a folder excepted hidden one.'''
    new_dir  = get_path_from_uri(parent_dir)
    uris = [ get_uri_from_path(os.path.join(new_dir, name)) for name in os.listdir(new_dir) if name[0] != "." and os.path.isfile(os.path.join(new_dir,name))]
    return uris

def get_uris_from_m3u(uri):
    ''' Return uri in a m3u playlist.'''
    uris = []
    content = read_entire_file(uri)
    lines = content.splitlines()

    for line in lines:
        if not line.startswith("#") and line.strip() != "":
            uris.append(line.strip())
    uris = [rebuild_uri(uri, u) for u in uris]        
    return uris

def get_uris_from_pls(uri):
    ''' return uri in a pls playlist. '''
    uris = []
    content = read_entire_file(uri)
    lines = content.splitlines()
    for line in lines:
        if line.lower().startswith("file") and line.find("=") != -1:
            uris.append(line[line.find("=") + 1:].strip())
    uris = [rebuild_uri(uri, u) for u in uris]        
    return uris

from xml.sax import parseString, handler
from xml.dom.minidom import parseString as dom_parseString

class XSPFParser(handler.ContentHandler):
    def __init__(self):
        self.uris = []

    def startElement(self, name, attrs):
        self.content = ""

    def characters(self, content):
        self.content = self.content + content

    def endElement(self, name):
        if name == "location":
            if self.content.startswith("http"):
                self.uris.insert(0, self.content)
            else:    
                self.uris.append(self.content)

def get_uris_from_xspf(uri):
    try: 
        handler = XSPFParser()
        parseString(read_entire_file(uri), handler)
    except:
        logger.logexception("Failed to parse %s", uri)
        return []
    else:
        return handler.uris

def get_uris_from_asx(uri):
    uri_list = []
    uri_asx_list = [uri]
    while len(uri_asx_list) > 0:
        uri = uri_asx_list.pop()
        text = read_entire_file(uri)
        links = []
        try:
            d = dom_parseString(text)
        except Exception, e:    
            print e
        else:    
            links = [ ref.getAttribute('HREF')
                      for ref in d.getElementsByTagName('REF')
                      if ref.hasAttribute('HREF') ]
            links.extend([ ref.getAttribute('href')
                           for ref in d.getElementsByTagName('ref')
                           if ref.hasAttribute('href') ])
            links.extend([ ref.getAttribute('href')
                           for ref in d.getElementsByTagName('Ref')
                           if ref.hasAttribute('href') ])
            links.extend([ ref.getAttribute('HREF')
                           for ref in d.getElementsByTagName('Ref')
                           if ref.hasAttribute('HREF') ])
            
        for link in links:
            if link[-4:] == ".asx" or (link.find("?") != -1 and link[link.find("?") - 4:link.find("?")] == ".asx"):
                uri_asx_list.append(link)
            else:
                if link.startswith("http"):
                    uri_list.insert(0, link)
                else:    
                    uri_list.append(link)
                    
    if len(uri_list) > 0:
        for index, uri in enumerate(uri_list):
            if uri.endswith("?MSWMExt=.asf"):
                uri_list[index] = uri.replace("http", "mms")
                
    return uri_list

def print_timeing(func):
    def wrapper(*arg):
        start_time = time.time()
        res = func(*arg)
        end_time   = time.time()
        if hasattr(func, "im_class"):
            class_name = func.im_class__name__ + ":"
        print "%s took %0.3fms" % (class_name + func.func_name, (start_time - end_time) * 1000.0)    
        return res
    return wrapper

def parse_uris(uris, follow_folder=True, follow_playlist=True, callback=None, *args_cb, **kwargs_cb):
    ''' Receive a list of uris ,expand it and check if exist.
        if directory recursive add all uri in directory
        if playlist read all uri in playlist
        return all supported file found.
    '''
    if not uris:
        return []
    valid_uris = []
    for uri in uris:
        #check file exists only is file is local to speed parsing of shared/remote file
        if not uri:
            continue
        if isinstance(uri, unicode):
            uri = uri.encode("utf-8")
        uri = unquote(uri)
        if uri and uri.strip() != "" and exists(uri):
            ext = get_ext(uri)
            try:
                mime_type = get_mime_type(uri)
            except:    
                logger.logexception("falied to read file info from %s", uri)
                continue
            
            is_pls  = False
            is_m3u  = False
            is_xspf = False
            try:
                is_pls = (mime_type == "audio/x-scpls")
                is_m3u = (mime_type == "audio/x-mpegurl" or mime_type == "audio/mpegurl" or mime_type == "audio/m3u")
                is_xspf = (mime_type == "application/xspf+xml")
            except:    
                pass
            
            is_pls = is_pls or (ext == ".pls")
            is_m3u = is_m3u or (ext == ".m3u")
            is_xspf = is_xspf or (ext == ".xspf")
            
            if is_local_dir(uri) and follow_folder:
                valid_uris.extend(parse_uris(parse_folder(uri), follow_folder, follow_playlist))
            elif follow_playlist and is_pls:    
                valid_uris.extend(parse_uris(get_uris_from_pls(uri), follow_folder, follow_playlist))
            elif follow_playlist and is_m3u:
                valid_uris.extend(parse_uris(get_uris_from_m3u(uri), follow_folder, follow_playlist))    
            elif follow_playlist and is_xspf:    
                valid_uris.extend(parse_uris(get_uris_from_xspf(uri), follow_folder, follow_playlist))
            elif get_scheme(uri) != "file" or common.file_is_supported(get_path_from_uri(uri)):
                valid_uris.append(uri)
                
    logger.loginfo("parse uris found %s uris", len(valid_uris))            
    if callback:
        def launch_callback(callback, uris, args, kwargs):
            callback(uris, *args, **kwargs)
        gobject.idle_add(launch_callback, callback, list(valid_uris), args_cb, kwargs_cb)    
    else:    
        return valid_uris
    
def async_parse_uris(*args, **kwargs):    
    ''' asynchronous parse uris. '''
    t = threading.Thread(target=parse_uris, args=args, kwargs=kwargs)
    t.setDaemon(True)
    t.start()
    
def async_get_uris_from_plain_text(content, callback=None, *args_cb, **kwargs_cb):
    """ Return uri found in a string
    For now return uri only if one line is one uri
    use parse uri to follow directory and playlist """
    
    uris = [ line.strip() for line in content.splitlines() if line.strip() ]
    async_parse_uris(uris, True, True, callback, *args_cb, **kwargs_cb)

def get_uris_from_plain_text(content, callback=None, *args_cb, **kwargs_cb):
    """ Return uri found in a string
    For now return uri only if one line is one uri
    use parse uri to follow directory and playlist """
    uris = [ line.strip() for line in content.splitlines() if line.strip() ]
    return parse_uris(uris, True, True, callback, *args_cb, **kwargs_cb)
    
def synchronized(func):
    """
        A decorator to make a function synchronized - which means only one
        thread is allowed to access it at a time
    """
    @wraps(func)
    def wrapper(self, *__args, **__kw):
        try:
            rlock = self._sync_lock
        except AttributeError:
            from threading import RLock
            rlock = self.__dict__.setdefault('_sync_lock', RLock())
        rlock.acquire()
        try:
            return func(self, *__args, **__kw)
        finally:
            rlock.release()
    return wrapper
    
def move_to_trash(uri):    
    ''' move uri to trash. '''
    path = get_path_from_uri(uri)
    path = os.path.realpath(os.path.expanduser(path))
    if not os.path.exists(path):
        logger.logdebug("File %s doesn't exists.", path)
        return False
    # find the trash folder
    home_dir = os.path.realpath(os.path.expanduser("~"))
    dirs_name = path.split("/")
    dirs_name = filter(lambda name: name != "", dirs_name)
    trash_dir = None
    r = range(0, len(dirs_name))
    r.reverse()
    for i in r:
        tmp_path = os.path.realpath("/" + "/".join(dirs_name[:i]))
        if tmp_path == home_dir:
            trash_dir = home_dir + "/.local/share/Trash"
            trash_dir = os.environ.get("XDG_DATA_HOME", trash_dir)
            break
        elif os.path.ismount(tmp_path):
            trash_dir = tmp_path + "/.Trash-%d" % os.getuid()
            break
        else:
            continue
    if not trash_dir:    
        logger.logerror("trash dir not found.")
        return False
    for subdir in ["files", "info"]:
        if not os.path.exists(trash_dir + "/" + subdir):
            try:
                os.makedirs(trash_dir + "/" + subdir, 0700)
            except:    
                logger.logwarn("Failed to create trash dir %s", trash_dir + "/" + subdir)
                return False
    if path.rfind("/") != -1:        
        filename = path[path.rindex("/") + 1:]
    delete_date = time.strftime("%Y-%M-%dT%H:%M:%S")    
    destpath = trash_dir + "/files/" + filename
    infopath = trash_dir + "/info/" + filename + ".trashinfo"
    try:
        shutil.move(path, destpath)
        f = open(infopath, "w")
        f.write("[Trash Info]\nPath=%s\nDeletionDate=%s\n" % (quote(path), delete_date))
        f.close()
    except:    
        logger.logexception("Failed to move to trash %s", path)
        return False
    else:
        return True
    
def download_iterator(remote_uri, local_uri, buffer_len=4096, timeout=DEFAULT_TIMEOUT):
    
    try:
        logger.logdebug("download %s starting...", remote_uri)
        handle_read = urlopen(remote_uri, timeout=timeout)
        handle_write = file(get_path_from_uri(local_uri), "w")
        info = handle_read.info()
        try:
            total_size = int(info.getheader("content-length"))
        except:    
            total_size = 20000000
        current_size = 0    
        data = handle_read.read(buffer_len)
        handle_write.write(data)
        current_size += len(data)
        
        while data:
            data = handle_read.read(buffer_len)
            current_size += len(data)
            handle_write.write(data)
            percent = current_size * 100 / total_size
            yield (total_size, current_size, percent)
            
        handle_read.close()    
        handle_write.close()
        logger.logdebug("download %s finish" % remote_uri)
    except GeneratorExit:    
        try:
            unlink(local_uri)
        except:    
            pass
    except:    
        logger.logexception("Error while downloading %s", remote_uri)
        try:
            unlink(local_uri)
        except:
            pass
        raise IOError
    
    
def direct_download(remote_uri, local_uri, timeout=DEFAULT_TIMEOUT):    
    try:
        handle_read = urlopen(remote_uri, timeout=timeout)
        data_buffer = handle_read.read()
        handle_write = open(local_uri, "w")
        handle_write.write(data_buffer)
        handle_read.close()        
        handle_write.close()
    except:    
        try:
            unlink(local_uri)
        except:    
            pass
        return False
    if not exists(local_uri):
        return False
    return True
    

std_headers = {
    'User-Agent': 'Mozilla/5.0 (Windows; U; Windows NT 6.1; '
        'en-US; rv:1.9.2) Gecko/20100115 Firefox/3.6',
    'Accept-Charset': 'ISO-8859-1,utf-8;q=0.7,*;q=0.7',
    'Accept': 'text/xml,application/xml,application/xhtml+xml,'
        'text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5',
    'Accept-Language': 'en-us,en;q=0.5',
}
    
def download(remote_uri, local_uri, net_encode=None, buffer_len=4096, timeout=DEFAULT_TIMEOUT):
    try:
        logger.logdebug("download %s starting...", remote_uri)
        request = Request(remote_uri, headers=std_headers)
        handle_read = urlopen(request, timeout=timeout)
        handle_write = file(local_uri, "w")
        
        data = handle_read.read(buffer_len)
        net_encode_status = False
        if net_encode:
            try:
                 net_data = unicode(data, net_encode).encode("utf-8")
            except:    
                net_encode_status = False
            else:
                net_encode_status = True
                data = net_data
        handle_write.write(data)
        
        while data:
            data = handle_read.read(buffer_len)
            if net_encode and net_encode_status:
                data = unicode(data, net_encode).encode("utf-8")
            handle_write.write(data)
            
        handle_read.close()    
        handle_write.close()
        logger.logdebug("download %s finish." % remote_uri)
    except Exception, e:
        logger.loginfo("Error while downloading %s, %s", remote_uri, e)
        try:
            unlink(local_uri)
        except:    
            pass
        return False
    if not exists(local_uri):
        return False
    return True
    
def threaded(f):
    """
        A decorator that will make any function run in a new thread
    """
    @wraps(f)
    def wrapper(*args, **kwargs):
        t = threading.Thread(target=f, args=args, kwargs=kwargs)
        t.setDaemon(True)
        t.start()

    return wrapper


class ThreadLoad(threading.Thread):
    def __init__(self, fetch_func, *args):
        super(ThreadLoad, self).__init__()
        self.setDaemon(True)
        self.fetch_func = fetch_func
        self.args = args
        
    def run(self):    
        self.fetch_func(*self.args)
        
class ThreadRun(threading.Thread):
    '''class docs'''
    def __init__(self,fetch_func, render_func, args=[], render_args=[]):
        '''init docs'''
        super(ThreadRun, self).__init__()
        self.setDaemon(True)
        self.fetch_func = fetch_func
        self.render_func = render_func
        self.args = args
        self.render_args = render_args

    def run(self):
        if self.args:
            result = self.fetch_func(*self.args)
        else:    
            result = self.fetch_func()
            
        if self.render_args:    
            self.render_func(result, *self.render_args)
        else:    
            self.render_func(result)
            
class ThreadFetch(threading.Thread):            
    
    def __init__(self, fetch_funcs, success_funcs=None, fail_funcs=None):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.fetch_funcs = fetch_funcs
        self.success_funcs = success_funcs
        self.fail_funcs = fail_funcs
        
    def run(self):    
        result = self.fetch_funcs[0](*self.fetch_funcs[1])
        if result:
            if self.success_funcs:
                self.success_funcs[0](result, *self.success_funcs[1])
        else:        
            if self.fail_funcs:
                self.fail_funcs[0](*self.fail_funcs[1])

def strdate_to_time(odate):
    date = odate
    #removing timezone
    c = date.rfind("-")
    if c!=-1:
        date = date[:c-1]

    c = date.rfind("+")
    if c!=-1:
        date = date[:c-1]

    date = date.strip()

    c = date[-3:]
    if c in ["GMT","CST","EST","PST","EDT","PDT","MST","MDT"]:
        date = date[:-3]
    date = date.strip()

    # Remove day because some times field have incorrect string
    c = date.rfind(",")
    if c!=-1:
        date = date [c+1:]
    date = date.strip()

    #trying multiple date formats
    new_date = None

    #Set locale to C to parse date
    locale.setlocale(locale.LC_TIME, "C")

    formats = ["%d %b %Y %H:%M:%S",#without day, short month
                "%d %B %Y %H:%M:%S",#without day, full month
                "%d %b %Y",#only date , short month
                "%d %B %Y",#only date , full month
                "%b %d %Y %H:%M:%S",#without day, short month
                "%B %d %Y %H:%M:%S",#without day, full month
                "%b %d %Y",#only date , short month
                "%B %d %Y",#only date , full month
                "%b/%d/%Y",#only date
                "%b/%d/%y",#only date
                "%b-%d-%Y",#only date
                "%b-%d-%y",#only date
                "%Y",#only years
                "%y",#only years
                ]
    for str_format in formats:
        try: new_date = strptime(date, str_format)
        except : continue

    locale.setlocale(locale.LC_TIME, '')

    
    if not new_date: return None
    else: 
        try:
            result_time = mktime(new_date)
        except:
            logger.logexception("problem to convert %s (%s) in date format",odate,new_date)
            return None
        else:
            return result_time


def save_db(objs, fn):
    '''Save object to db file.'''
    try:
        f = open(fn + ".tmp", "w")
        fcntl.flock(f.fileno(), fcntl.LOCK_EX)
        cPickle.dump(objs, f, cPickle.HIGHEST_PROTOCOL)
        f.close()
        os.rename(fn + ".tmp", fn)
    except:    
        pass
    
def load_db(fn):    
    '''Load object from db file.'''
    objs = None
    if os.path.exists(fn):
        f = open(fn, "rb")
        try:
            objs = cPickle.load(f)
        except:    
            logger.logexception("%s is not a valid database.", fn)
            try:
                shutil.copy(fn, fn + ".not-valid")
            except: pass    
            objs = None
        f.close()    
    return objs    

def run_command(command):
    '''Run command.'''
    subprocess.Popen("nohup %s > /dev/null 2>&1" % (command), shell=True)
    
def color_hex_to_cairo(color):            
    gdk_color = gtk.gdk.color_parse(color)
    return (gdk_color.red / 65535.0, gdk_color.green / 65535.0, gdk_color.blue / 65535.0)

def str_size(nb, average=0, base=1024):
    if not nb:
        return "unknow"
    if average != 0:
        average += 1
    nb = float(nb)    
    size_format = ""
    units = ("B KB MB GB").split()
        
    for size_format in units:    
        if len("%d" % int(nb)) <= 3:
            break
        nb = float(nb) / float(base)
    nb = "%f" % round(nb, 1)    
    nb = nb[:nb.rfind(".") + average] + size_format
    return nb

"""
XML simple un/escape caracter
"""

def xmlescape(stri):
    """Escape a string in a manner suitable for XML/Pango."""
    stri = str(stri)
    stri = stri.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    return stri


def xmlunescape(stri):
    """Unescape a string in a manner suitable for XML/Pango."""
    stri = str(stri)
    return stri.replace("&lt;", "<").replace("&gt;", ">").replace("&amp;", "&")

def export_playlist(list_song, filename, p_type="m3u"):
    if p_type=="m3u":
        fileout = open(filename, "w")
        fileout.write("\n".join([song.get_path() for song in list_song if song.get_path() != None]))
        fileout.close()
    elif p_type=="pls":
        fileout = open(filename, "w")
        [ fileout.write("file%d=%s\n"%(i,song.get_path()))  for i,song in enumerate(list_song)  if song.get_path() != None  ]
        fileout.close()
    elif p_type=="xspf":
        fileout = open(filename, "w")
        fileout.write('<?xml version="1.0" encoding="UTF-8"?>\n<playlist version="0" xmlns = "http://xspf.org/ns/0/">\n<trackList>')
        [ fileout.write("<track><location>%s</location></track>\n"%song.get("uri"))  for song in list_song if song.get("uri") != None  ]
        fileout.write('  </trackList>\n</playlist>\n' )
        fileout.close()
    else:
        raise TypeError, "Unknow playlist type"
    
    
class RGBTuple(tuple):
    """ Tuple for RGB values """
    @classmethod
    def from_hexstring(cls, value):
        """ Returns a RGB tuple from hexstring ('#RRGGBB') """
        value = value.lstrip('#')
        return cls(int(value[i:i+2], 16) for i in (0, 2, 4))

    def to_string(self):
        """ Returns the hexstring representation ('#RRGGBB') of ``self``"""
        def _exp(v):
            return v if len(v) == 2 else '0'+v
        return '#' + ''.join(map(lambda x:_exp(hex(x)[2:]).upper(), self))

    def __str__(self):
        return self.to_string()
    
def get_cookie_file(username):
    return get_cache_file(hashlib.md5(username).hexdigest())

def parser_json(raw):
    try:
        data = json.loads(raw)
    except:    
        try:
            data = eval(raw, type("Dummy", (dict,), dict(__getitem__=lambda s,n: n))())
        except:    
            data = {}
    return data    
    
def encode_multipart(fields, files):
    """
    fields is a sequence of (name, value) elements for regular form fields.
    files is a sequence of (name, filename, value) elements for data to be uploaded as files
    Return (content_type, body) ready for httplib.HTTP instance
    """
    def get_mime_type(filename):
        return mimetypes.guess_type(filename)[0] or 'application/octet-stream'    
    
    BOUNDARY = '----------ThIs_Is_tHe_bouNdaRY_$'
    CRLF = '\r\n'
    L = []
    for (key, value) in fields:
        L.append('--' + BOUNDARY)
        L.append('Content-Disposition: form-data; name="%s"' % key)
        L.append('')
        L.append(value)
    for (key, filename, value) in files:
        L.append('--' + BOUNDARY)
        L.append('Content-Disposition: form-data; name="%s"; filename="%s"' % (key, filename))
        L.append('Content-Type: %s' % get_mime_type(filename))
        L.append('')
        L.append(value)
        L.append('--' + BOUNDARY + '--')
        L.append('')
    body = CRLF.join(L)
    content_type = 'multipart/form-data; boundary=%s' % BOUNDARY
    return content_type, body

def clip_surface(image_file):    
    image_surface = cairo.ImageSurface.create_from_png(image_file)
    w , h = image_surface.get_width(), image_surface.get_height()
    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, w, h)
    cr = cairo.Context(surface)
    cr.arc(w / 2, h / 2, w / 2 , 0,  2  * math.pi)
    cr.clip()
    cr.set_source_surface(image_surface, 0, 0)
    cr.set_operator(cairo.OPERATOR_SOURCE)
    cr.paint()
    surface.write_to_png(image_file)

    del image_surface
    del surface
    
def post_gui(func):
    '''
    Post GUI code in main thread.

    You should use post_gui wrap graphics function if function call from other threads.
    
    Usage:

    >>> @post_gui
    >>> def graphics_fun():
    >>>     ....
    '''
    def wrap(*a, **kw):
        gtk.gdk.threads_enter()
        ret = func(*a, **kw)
        gtk.gdk.threads_leave()
        return ret
    return wrap

def get_optimum_pixbuf(pixbuf, expect_width, expect_height, cut_middle_area=True):
    pixbuf_width, pixbuf_height = pixbuf.get_width(), pixbuf.get_height()
    if pixbuf_width >= expect_width and pixbuf_height >= expect_height:
        if float(pixbuf_width) / pixbuf_height == float(expect_width) / expect_height:
            scale_width, scale_height = expect_width, expect_height
        elif float(pixbuf_width) / pixbuf_height > float(expect_width) / expect_height:
            scale_height = expect_height
            scale_width = int(float(pixbuf_width) * expect_height / pixbuf_height)
        else:
            scale_width = expect_width
            scale_height = int(float(pixbuf_height) * expect_width / pixbuf_width)
            
        if cut_middle_area:
            subpixbuf_x = (scale_width - expect_width) / 2
            subpixbuf_y = (scale_height - expect_height) / 2
        else:
            subpixbuf_x = 0
            subpixbuf_y = 0
            
        return pixbuf.scale_simple(
            scale_width, 
            scale_height, 
            gtk.gdk.INTERP_BILINEAR).subpixbuf(subpixbuf_x,
                                               subpixbuf_y,
                                               expect_width, 
                                               expect_height)
    elif pixbuf_width >= expect_width:
        scale_width = expect_width
        scale_height = int(float(expect_width) * pixbuf_height / pixbuf_width)
        
        if cut_middle_area:
            subpixbuf_x = (scale_width - expect_width) / 2
            subpixbuf_y = max((scale_height - expect_height) / 2, 0)
        else:
            subpixbuf_x = 0
            subpixbuf_y = 0
            
        return pixbuf.scale_simple(
            scale_width,
            scale_height,
            gtk.gdk.INTERP_BILINEAR).subpixbuf(subpixbuf_x,
                                               subpixbuf_y,
                                               expect_width, 
                                               min(expect_height, scale_height))
    elif pixbuf_height >= expect_height:
        scale_width = int(float(expect_height) * pixbuf_width / pixbuf_height)
        scale_height = expect_height
        
        if cut_middle_area:
            subpixbuf_x = max((scale_width - expect_width) / 2, 0)
            subpixbuf_y = (scale_height - expect_height) / 2
        else:
            subpixbuf_x = 0
            subpixbuf_y = 0
        
        return pixbuf.scale_simple(
            scale_width,
            scale_height,
            gtk.gdk.INTERP_BILINEAR).subpixbuf(subpixbuf_x,
                                               subpixbuf_y,
                                               min(expect_width, scale_width), 
                                               expect_height)
    else:
        return pixbuf
    
    
def get_fixed_value(string, num):    
    md5_string = md5.md5(string).hexdigest()
    return int(md5_string[0:5], 16) % num

class DistanceRandom(object):
    '''
    >>> r = DistanceRandom(range(10))
    >>> for i in range(200):
    >>>     print r.get()
    '''
    def __init__(self, v_range, v_distance=None):
        if v_distance == None:
            v_distance = len(v_range)
        assert(len(v_range) >= v_distance >= 1)
        self.freshed = v_range
        self.dist = v_distance
        self.used = []

    def get(self):
        if len(self.used) >= self.dist:
            t = self.used.pop()
            self.freshed.append(t)

        v = self.freshed.pop(random.randint(0, len(self.freshed) - 1))
        self.used.insert(0, v)

        return v

    
def get_system_lang():    
    (lang, encode) = locale.getdefaultlocale()
    return lang

def parser_plugin_infos(plugin_path):
    vaild_keys = ["Version", "Authors", "Name", "Description"]
    plugin_infos = { key:"" for key in vaild_keys }
    section_name = "Plugin Info"
    lang = get_system_lang()
    
    plugin_parser = ConfigParser()
    try:
        plugin_parser.read(plugin_path)
    except Exception, e:    
        print e
        return plugin_infos
    else:    
        for key in vaild_keys:
            lang_key = "%s[%s]" % (key, lang)
            if plugin_parser.has_option(section_name, lang_key):
                plugin_infos[key] = plugin_parser.get(section_name, lang_key)
            elif plugin_parser.has_option(section_name, key):    
                plugin_infos[key] = plugin_parser.get(section_name, key)
        return plugin_infos        
    
    
def get_screen_size():    
    root_window = gtk.gdk.get_default_root_window()
    return root_window.get_size()

def get_song_attr(song):    
    if song.get_type() in ["cue", "local", "cdda"]:
        try:
            gio_file = gio.File(song.get_path())
            gio_file_info = gio_file.query_info(",".join([gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,]))
            info_attr = gio_file_info.get_attribute_as_string(gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE)                
            return gio.content_type_get_description(info_attr)
        except:    
            return song.get_ext(False).upper()
    else:    
        return song.get_ext(False).upper()
    
def _glib_wait_inner(timeout, glib_timeout_func):
    id = [None] # Have to hold the value in a mutable structure because
                # python's scoping rules prevent us assigning to an
                # outer scope directly.
    def waiter(function):
        def delayer(*args, **kwargs):
            if id[0]: glib.source_remove(id[0])
            id[0] = glib_timeout_func(timeout, function, *args, **kwargs)
        return delayer
    return waiter

def glib_wait(timeout):
    """
        Decorator to make a function run only after 'timeout'
        milliseconds have elapsed since the most recent call to the
        function.

        For example, if a function was given a timeout of 1000 and
        called once, then again half a second later, it would run
        only once, 1.5 seconds after the first call to it.

        Arguments are supported, but which call's set of arguments
        is used is undefined, so this is not useful for much beyond
        passing in unchanging arguments like 'self' or 'cls'.

        If the function returns a value that evaluates to True, it
        will be called again under the same timeout rules.
    """
    # 'undefined' is a bit of a white lie - it's always the most
    # recent call's args. However, I'm reserving the right to change
    # the implementation later for the moment, and really I don't
    # think it makes sense to use functions that have changing args
    # with this decorator.
    return _glib_wait_inner(timeout, glib.timeout_add)

def glib_wait_seconds(timeout):
    """
        Same as glib_wait, but uses glib.timeout_add_seconds instead
        of glib.timeout_add and takes its timeout in seconds. See the
        glib documention for why you might want to use one over the
        other.
    """
    return _glib_wait_inner(timeout, glib.timeout_add_seconds)
    
    
global MAIN_WINDOW            
MAIN_WINDOW = None

def get_main_window():
    global MAIN_WINDOW
    return MAIN_WINDOW

def set_main_window(win):
    global MAIN_WINDOW
    MAIN_WINDOW = win

if __name__ == "__main__":    
    print duration_to_string(252049)
