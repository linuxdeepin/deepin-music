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

from nls import _

CONFIG_FILENAME = "config"
PROGRAM_NAME = "deepin-music-player"
PROGRAM_VERSION = "2.0"
PROGRAM_NAME_LONG = _("Deepin Music")
PROGRAM_NAME_SHORT = _("DMusic")

DEFAULT_TIMEOUT = 10
DEFAULT_FONT_SIZE = 9
AUTOSAVE_TIMEOUT = 1000 * 60 * 5  # 5min

# Lyric mode
LRC_DESKTOP_MODE = 1
LRC_WINDOW_MODE = 2

PREDEFINE_COLORS = {
   "fresh_green"     : ["#e4dcb9", "#ffea93", "#ffd631", "#efede6", "#b3fc9c", "#77d035"],
   "playful_pink"    : ["#ffffff", "#70b8e5", "#3788c0", "#ffe0ee", "#ffa1ca", "#ff2586"],
   "cool_blue"       : ["#f8f8f8", "#dadada", "#bdbdbd", "#ffffff", "#60c0ff", "#19a1ff"],
   "vitality_yellow" : ["#e4dcb9", "#ffea93", "#ffd631", "#f7f4ea", "#77d1ff", "#4199d5"],
    }

FULL_DEFAULT_WIDTH = 886
FULL_DEFAULT_HEIGHT = 625
SIMPLE_DEFAULT_WIDTH = 322
SIMPLE_DEFAULT_HEIGHT = 625
HIDE_PLAYLIST_WIDTH = 210
LIST_WIDTH = 312
CATEGROYLIST_WIDTH = 104
PLAYLIST_WIDTH = 198


TAB_LOCAL = 1
TAB_WEBCAST = 2
TAB_RADIO = 3


EMPTY_WEBCAST_ITEM = 1
EMPTY_RADIO_ITEM = 2
