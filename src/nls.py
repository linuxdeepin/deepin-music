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

import gettext
import os


def get_parent_dir(filepath, level=1):
    '''Get parent dir.'''
    parent_dir = os.path.realpath(filepath)
    
    while(level > 0):
        parent_dir = os.path.dirname(parent_dir)
        level -= 1
    
    return parent_dir

LOCALE_DIR=os.path.join(get_parent_dir(__file__, 2), "locale")
if not os.path.exists(LOCALE_DIR):
    LOCALE_DIR="/usr/share/locale"
    
gettext.textdomain("deepin-music-player")    
gettext.bindtextdomain("deepin-music-player", LOCALE_DIR)
_ = gettext.gettext

# import locale
# import os.path

# def get_parent_dir(filepath, level=1):
#     '''Get parent dir.'''
#     parent_dir = os.path.realpath(filepath)
    
#     while(level > 0):
#         parent_dir = os.path.dirname(parent_dir)
#         level -= 1
    
#     return parent_dir

# LOCALE_DIR=os.path.join(get_parent_dir(__file__, 2), "locale")


# try:
#     # Set to user default, gracefully fallback on C otherwise
#     locale.setlocale(locale.LC_ALL, '')
# except locale.Error:
#     pass

# try:
#     print "true"
#     import gettext as gettextmod

#     gettextmod.textdomain('deepin-music-player')
#     gettextmod.bindtextdomain('deepin-music-player', LOCALE_DIR)

#     gettextfunc = gettextmod.gettext

#     def gettext(text):
#         return gettextfunc(text).decode("utf-8")

#     ngettextfunc = gettextmod.ngettext

#     def ngettext(singular, plural, n):
#         return ngettextfunc(singular, plural, n).decode('utf-8')

# except ImportError:
#     # gettext is not available.  Provide a dummy function instead
#     print "error"
#     def gettext(text):
#         return text

#     def ngettext(singular, plural, n):
#         if n == 1:
#             return singular
#         else:
#             return plural

# _ = gettext        