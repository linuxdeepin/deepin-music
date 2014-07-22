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
import subprocess

from constant import PROGRAM_NAME

_home = os.path.expanduser('~')
xdg_data_home = os.environ.get('XDG_DATA_HOME') or \
            os.path.join(_home, '.local', 'share')

xdg_data_dirs = [xdg_data_home] + \
    (os.environ.get('XDG_DATA_DIRS') or '/usr/local/share:/usr/share').split(':')


xdg_config_home = os.environ.get('XDG_CONFIG_HOME') or \
            os.path.join(_home, '.config')

xdg_config_dirs = [xdg_config_home] + \
    (os.environ.get('XDG_CONFIG_DIRS') or '/etc/xdg').split(':')

xdg_cache_home = os.environ.get('XDG_CACHE_HOME') or \
            os.path.join(_home, '.cache')

xdg_data_dirs = filter(lambda x: x, xdg_data_dirs)
xdg_config_dirs = filter(lambda x: x, xdg_config_dirs)

dmusic_data_dir = os.path.join(xdg_data_home, PROGRAM_NAME)
# dmusic_dir = os.path.dirname(os.path.realpath(__file__))
dmusic_dir = os.path.split(os.path.dirname(os.path.realpath(__file__)))[0]

def save_config_path(*resource):
    """Ensure $XDG_CONFIG_HOME/<resource>/ exists, and return its path.
    'resource' should normally be the name of your application. Use this
    when SAVING configuration settings. Use the xdg_config_dirs variable
    for loading."""
    resource = os.path.join(*resource)
    assert not resource.startswith('/')
    path = os.path.join(xdg_config_home, resource)
    if not os.path.isdir(path):
        os.makedirs(path, 0700)
    return path
    
def get_config_file(filename):
    ''' get config file. '''
    return os.path.join(save_config_path(PROGRAM_NAME), filename)

def get_sub_config_dir(dir_name):
    ''' get subconfig dir. '''
    subdir = save_config_path("%s/%s" % (PROGRAM_NAME,dir_name))
    if not os.path.isdir(subdir):
        os.makedirs(subdir)
    return subdir    

def get_cache_file(path):
    ''' get cache file. '''
    cachefile = os.path.join(xdg_cache_home, PROGRAM_NAME, path)
    cachedir = os.path.dirname(cachefile)
    if not os.path.isdir(cachedir):
        os.makedirs(cachedir)
    return cachefile    

def get_cache_dir(dir_name):
    ''' get cache dir. '''
    cachedir = os.path.join(xdg_cache_home, PROGRAM_NAME, dir_name)
    if not os.path.isdir(cachedir):
        os.makedirs(cachedir)
    return cachedir    

def get_plugin_dirs():
    return [ os.path.join(p, "plugins") for p in [dmusic_data_dir, dmusic_dir]]

def get_tmp_file(filename):
    ''' get temp file. '''
    return os.path.join(os.environ.get("TMPDIR", "/tmp"), filename)

def get_music_dir():
    ''' get user music dir. '''
    musicdir = os.path.expanduser("~/Music")
    try:
        p = subprocess.Popen(["xdg-user-dir", "MUSIC"], stdout=subprocess.PIPE)
    except OSError:    
        return musicdir
    else:
        musicpath = p.communicate()[0].strip()
        if p.returncode == 0 and musicpath and musicpath != os.path.expanduser("~"):
            return musicpath    
                
        else:
            return musicdir
        
def get_song_save_path(song):        
    path = os.path.join(get_music_dir(), "deepin-music")
    if not os.path.exists(path):
        os.makedirs(path)
    path = os.path.join(path, "%s.mp3" % song.get_str("title"))    
    return path    

def get_common_image(name):
    return os.path.join(dmusic_dir, "image", name)
        
if __name__ == "__main__":
    print get_plugin_dirs()
    