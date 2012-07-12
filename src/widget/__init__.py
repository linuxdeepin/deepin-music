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

import gtk
from widget.skin import app_theme
import locale

def get_prefix():
    (lang, encode) = locale.getdefaultlocale()
    if lang == "zh_CN":
        prefix = "cn"
    elif lang in ["zh_HK", "zh_TW"]:
        prefix = "tw"
    else:    
        prefix = "en"
    return prefix

def show_splash(show=True):
    """
        Show a splash screen

        @param show: [bool] show the splash screen
    """
    if not show: return
    
    splash_window = gtk.Window(gtk.WINDOW_POPUP)
    splash_window.set_size_request(391, 143)
    splash_window.set_position(gtk.WIN_POS_CENTER)
    splash_window.set_resizable(False)
    splash_window.set_transient_for(None)    
    splash_window.set_destroy_with_parent(True)
    splash_window.set_skip_taskbar_hint(True)
    splash_window.set_skip_pager_hint(True)
    splash_window.set_type_hint(gtk.gdk.WINDOW_TYPE_HINT_SPLASHSCREEN)
    splash_window.set_decorated(False)

    splash_image = gtk.Image()
    splash_image.set_visible(True)
    splash_image.set_from_pixbuf(app_theme.get_pixbuf("splash/%s_splash.png" % get_prefix()).get_pixbuf())
    splash_window.add(splash_image)
    
    # Show the splash screen without causing startup notification.
    gtk.window_set_auto_startup_notification(False)
    splash_window.show_all()
    gtk.window_set_auto_startup_notification(True)

    #ensure that the splash gets completely drawn before we move on
    while gtk.events_pending():
        gtk.main_iteration()
    return splash_window
