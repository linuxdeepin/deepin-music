#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

import dbus

NOTIFICATIONS_SERVICE_NAME = "org.freedesktop.Notifications"
NOTIFICATIONS_PATH = "/org/freedesktop/Notifications"

def check_dbus(bus, interface):
    obj = bus.get_object('org.freedesktop.DBus', '/org/freedesktop/DBus')
    dbus_iface = dbus.Interface(obj, 'org.freedesktop.DBus')
    avail = dbus_iface.ListNames()
    return interface in avail


class DbusNotify(object):
    
    def __init__(self, app_name, icon=None):
        self.app_name = app_name
        self.icon = icon or app_name
        self.summary = ""
        self.body = ""
        self.hints = {}
        self.actions = []
        self.timeout = 3
        
    def set_summary(self, summary):    
        self.summary = summary
        
    def set_body(self, body):    
        self.body = body
        
    def set_icon_from_pixbuf(self, pixbuf):
        pass
    
    def pixbuf_to_dbus(self, pixbuf):
        pass
        
    def set_icon_from_path(self, image_path):    
        self.hints["image-path"] = image_path
        
    def notify(self):
        bus = dbus.SessionBus()
        # if not check_dbus(bus, NOTIFICATIONS_SERVICE_NAME):
        #     return False
        
        try:
            proxy = bus.get_object(NOTIFICATIONS_SERVICE_NAME,
                                   NOTIFICATIONS_PATH)
            notify_interface = dbus.Interface(proxy, NOTIFICATIONS_SERVICE_NAME)
            notify_interface.Notify(self.app_name, 1, self.icon, self.summary, self.body,
                                    self.actions, self.hints, self.timeout)
        except:    
            pass
            

dbus_notify = DbusNotify("deepin-music-player", "deepin-music-player")        
