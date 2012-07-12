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

import gtk
from dtk.ui.menu import Menu

from player import Player
from widget.skin import app_theme
from helper import Dispatcher
from config import config
from nls import _

class BaseTrayIcon(object):
    '''Trayicon base, needs to be derived from.'''
    
    def __init__(self, instance):
        self.update_icon()
        self.connect_events()
        self.instance = instance
        self.menu = None
    
    def update_icon(self):
        self.set_from_pixbuf(app_theme.get_pixbuf("skin/logo.ico").get_pixbuf())
        
    def get_volume_menu(self):    
        menu_items = [
            (None, _("Increase volume"), Player.increase_volume),
            (None, _("Decrease volume"), Player.decrease_volume),
            (None, _("Mute"), Player.mute_volume),
            ]
        return Menu(menu_items)
        
    def update_menu(self):    
        menu_items = []
        if Player.is_paused():
            pixbuf_group = self.get_pixbuf_group("play")
            status_label = _("Play")
        else:    
            pixbuf_group = self.get_pixbuf_group("pause")
            status_label = _("Pause")
        menu_items.append((pixbuf_group, status_label, Player.playpause))
        menu_items.append((self.get_pixbuf_group("previous"), _("Previous"), Player.previous))
        menu_items.append((self.get_pixbuf_group("next"), _("Next"), Player.next))
        menu_items.append(None)
        menu_items.append((self.get_pixbuf_group("volume"), _("Volume"), self.get_volume_menu()))
        menu_items.append(None)    
        
        if config.getboolean("lyrics", "locked"):
            menu_items.append((self.get_pixbuf_group("unlock"), _("Unlock lyrics"), lambda : Dispatcher.unlock_lyrics()))
        else:
            menu_items.append((self.get_pixbuf_group("lock"), _("Lock lyrics"), lambda : Dispatcher.lock_lyrics()))
        
        if config.getboolean("lyrics", "status"):
            menu_items.append((None, _("Lyrics on"), lambda : Dispatcher.close_lyrics()))
        else:    
            menu_items.append((None, _("Lyrics off"), lambda : Dispatcher.show_lyrics()))
        menu_items.append(None)    
        menu_items.append((self.get_pixbuf_group("setting"), _("Preference"), lambda : Dispatcher.show_setting()))
        menu_items.append((self.get_pixbuf_group("close"), _("Quit"), lambda : Dispatcher.quit()))
        if self.menu is not None:
            del self.menu
        self.menu = Menu(menu_items, True)
        
    def get_pixbuf_group(self, name):    
        return (app_theme.get_pixbuf("tray/%s_normal.png" % name), app_theme.get_pixbuf("tray/%s_hover.png" % name))

    def connect_events(self):    
        self.connect("button-press-event", self.on_button_press_event)
    
    def on_button_press_event(self, widget, event):    
        if event.button == 1:
            if event.state == gtk.gdk.CONTROL_MASK:
                Player.previous()
            else:
                self.instance.toggle_visible()
                
        elif event.button == 2:
            Player.playpause()

        elif event.button == 3:
            if event.state == gtk.gdk.CONTROL_MASK:
                Player.next()
            else:
                menu = gtk.Menu()
                (x, y, z) =  self.get_menu_position(menu, self)
                self.update_menu()
                self.menu.show((int(x), int(y)), (0, -32))                
            
    def destroy(self):        
        self.instance.toggle_visible(True)
        self.set_visible(False)    

class TrayIcon(gtk.StatusIcon, BaseTrayIcon):    
    
    def __init__(self, instance):
        gtk.StatusIcon.__init__(self)
        BaseTrayIcon.__init__(self, instance)
        
    def get_menu_position(self, menu, icon):    
        return gtk.status_icon_position_menu(menu, icon)
