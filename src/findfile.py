#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 Deepin, Inc.
#               2011 Hou Shaohui
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

import os
import glib
# from subprocess import Popen,PIPE
from xdg.BaseDirectory import xdg_cache_home, save_config_path

from constant import PROGRAM_NAME
    
def get_config_file(filename):
    ''' get config file. '''
    return os.path.join(save_config_path(PROGRAM_NAME), filename)

def get_sub_config_dir(filename):
    ''' get subconfig dir. '''
    subdir = save_config_path("%s/%s" % (PROGRAM_NAME,filename))
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

def get_cache_dir(path):
    ''' get cache dir. '''
    cachedir = os.path.join(xdg_cache_home, PROGRAM_NAME, path)
    if not os.path.isdir(cachedir):
        os.makedirs(cachedir)
    return cachedir    

def get_tmp_file(filename):
    ''' get temp file. '''
    return os.path.join(os.environ.get("TMPDIR", "/tmp"), filename)

def get_music_dir():
    ''' get user music dir. '''
    # musicdir = os.path.expanduser("~/Music")
    # try:
    #     p = Popen(["xdg-user-dir", "MUSIC"], stdout=PIPE)
    # except OSError:    
    #     return musicdir
    # else:
    #     musicpath = p.communicate()[0].strip()
    #     if p.returncode == 0 and musicpath and musicpath != os.path.expanduser("~"):
    #         return musicpath    
                
    #     else:
    #         return musicdir
    return glib.get_user_special_dir(3)
