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
import gobject
from dtk.ui.menu import Menu

import utils
from config import config
from player import Player
from widget.skin import app_theme

class BaseTrayIcon(object):
    '''Trayicon base, needs to be derived from.'''
    
    def __init__(self, main):
        self.main = main
        self.update_icon()
        self.setup_menu()
        self.connect_events()
    
    def update_icon(self):
        self.set_from_pixbuf(app_theme.get_pixbuf("skin/logo.ico").get_pixbuf())
        
    def set_from_icon_name(self, icon_name):
        """
            Updates the tray icon
        """
        pass

    def set_tooltip(self, tooltip_text):
        """
            Updates the tray icon tooltip
        """
        pass

    def set_visible(self, visible):
        """
            Shows or hides the tray icon
        """
        pass
    
    def get_menu_position(self, menu, icon):
        """
            Returns coordinates for
            the best menu position
        """
        return (0, 0, False)
    
    def setup_menu(self):    
        menu_items = [
            (None, "播放/暂停", Player.playpause),
            (None, "上一首", Player.previous),
            (None, "下一首", Player.next),
            None,
            (None, "播放模式", None),
            None,
            (None, "锁定歌词", None),
            (None, "关闭歌词", None),
            None,
            (None, "选项设置", None),
            (None, "退出", None)
            ]
        self.menu = Menu(menu_items, True)
        
    def connect_events(self):    
        self.connect("button-press-event", self.on_button_press_event)
    
    def on_button_press_event(self, widget, event):    
        if event.button == 1:
            self.toggle_visible()
        if event.button == 2:
            pass
        if event.button == 3:
            self.menu.show((int(event.x_root), int(event.y_root)))
            
    def toggle_visible(self):    
        if self.main.get_property("visible"):
            self.main.hide()
        else:    
            self.main.present()
            
    def destroy(self):        
        if not self.main.get_property("visible"):
            self.main.deiconify()
            self.main.present()
        self.set_visible(False)    
    
class TrayIcon(gtk.StatusIcon, BaseTrayIcon):    
    
    def __init__(self, main):
        gtk.StatusIcon.__init__(self)
        BaseTrayIcon.__init__(self, main)
        
    def get_menu_position(self, menu, icon):    
        return gtk.status_icon_position_menu(menu, icon)
        

