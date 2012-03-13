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
import gtk
from dtk.ui.application import Application
from dtk.ui.theme import Theme
from dtk.ui.box import EventBox, ImageBox, TextBox
from dtk.ui.button import Button, ImageButton, ToggleButton
from dtk.ui.progressbar import ProgressBar 
from dtk.ui.scalebar import HScalebar
from dtk.ui.volume_button import VolumeButton
from dtk.ui.frame import HorizontalFrame
from dtk.ui.listview import *
from dtk.ui.scrolled_window import *

app_theme = Theme(os.path.join((os.path.dirname(os.path.realpath(__file__))), "../app_theme"))

song_scalebar = HScalebar(
    app_theme.get_pixbuf("hscalebar/left_fg.png"),
    app_theme.get_pixbuf("hscalebar/left_bg.png"),
    app_theme.get_pixbuf("hscalebar/middle_fg.png"),
    app_theme.get_pixbuf("hscalebar/middle_bg.png"),
    app_theme.get_pixbuf("hscalebar/right_fg.png"),
    app_theme.get_pixbuf("hscalebar/right_bg.png"),
    app_theme.get_pixbuf("hscalebar/point.png"),
    )

playlist_button = ToggleButton(
    app_theme.get_pixbuf("control/playlist_normal.png"),
    app_theme.get_pixbuf("control/playlist_hover.png"))

lyrics_button = ToggleButton(
    app_theme.get_pixbuf("control/lyrics_normal.png"),
    app_theme.get_pixbuf("control/lyrics_hover.png"))

musicbox_button = ToggleButton(
    app_theme.get_pixbuf("control/musicbox_normal.png"),
    app_theme.get_pixbuf("control/musicbox_hover.png"))

media_button = ToggleButton(
    app_theme.get_pixbuf("control/media_normal.png"),
    app_theme.get_pixbuf("control/media_hover.png"))
