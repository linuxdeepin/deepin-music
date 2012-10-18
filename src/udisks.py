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

import gobject

try: 
    import dbus
    dbus_version = getattr(dbus, 'version', (0,0,0))
    if dbus_version >= (0,80,0):
        from dbus.mainloop.glib import DBusGMainLoop
        DBusGMainLoop(set_as_default=True)
        dbus.SystemBus()
    else:
        print "DBus version too old"

except: dbus_imported = False
else: dbus_imported=True

import utils

from logger import Logger
from library import MediaDB
from dbus_manager import check_dbus

DIFFERD_UDISKS_STARTUP_TIME = 2000

class UDisks(gobject.GObject, Logger):
    __gsignals__ = {
        "volume-mounted" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, )),
        "volume-umounted" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (str, )),
            }

    def __init__(self):
        gobject.GObject.__init__(self)
        self.__volumes = set()
        
        if dbus_imported:
            try:
                self.bus = dbus.SystemBus()
            except:    
                self.bus = None
                
        if dbus_imported and self.bus and check_dbus(self.bus, "org.freedesktop.UDisks"):
            self.dbus_udisks = True
            self.udisks_object = self.bus.get_object("org.freedesktop.UDisks", "/org/freedesktop/UDisks")
            self.udisks_interface = dbus.Interface(self.udisks_object, "org.freedesktop.UDisks")
            
            if MediaDB.isloaded():
                self.__connect_to_db()
            else:    
                MediaDB.connect("loaded", self.__connect_to_db)
        else:        
            self.dbus_udisks = False
            self.logdebug("No UDisks support")
            
    def __connect_to_db(self, *args, **kwargs):        
        gobject.timeout_add(DIFFERD_UDISKS_STARTUP_TIME, self.__post_load)
        
    # @utils.threaded    
    def __post_load(self):
        self.udisks_interface.connect_to_signal("DeviceAdded", self.__device_added)
        self.udisks_interface.connect_to_signal("DeviceRemoved", self.__device_removed)
        self.udisks_interface.connect_to_signal("DeviceChanged", self.__device_changed)
        udis = self.udisks_interface.EnumerateDevices()
        for udi in udis:
            self.__device_added(udi, True)
            
    def umount(self, udi):        
        dev_obj = self.bus.get_object("org.freedesktop.UDisks", udi)
        dev = dbus.Interface(dev_obj, "org.freedesktop.UDisks.Device")
        dev.FilesystemUmount([])
        
    def eject(self, udi):    
        dev_obj = self.bus.get_object("org.freedesktop.UDisks", udi)
        dev = dbus.Interface(dev_obj, "org.freedesktop.UDisks.Device")
        dev.DriveEject([])
        
    def __device_added(self, udi, emit_signal=True):    
        if self.get_property(udi) or self.get_property(udi, "DeviceIsOpticalDisc"):
            self.__volumes.add(udi)
            self.loginfo("mounted volume found: %s", udi)
            if emit_signal:
                self.emit("volume-mounted", udi)

    def __device_removed(self, udi):    
        if udi in self.__volumes:
            self.__volumes.remove(udi)
            self.loginfo("device removed: %s", udi)
            self.emit("volume-umounted", udi)
            
    def __device_changed(self, udi):
        if self.get_property(udi) or self.get_property(udi, "DeviceIsOpticalDisc"):
            if udi not in self.__volumes:
                self.loginfo("mounted volume found: %s", udi)
                self.__volumes.add(udi)
                self.emit("volume-mounted", udi)
        else:        
            if udi in self.__volumes:
                self.loginfo("device removed: %s", udi)
                self.__volumes.remove(udi)
                self.emit("volume-umounted", udi)
                
    def get_mounted_devices(self):
        if not self.dbus_udisks:
            return []
        else:
            return self.__volumes
    
    def get_mount_point(self, udi):    
        value = self.get_property(udi, "DeviceMountPaths")
        if value:
            return value[0]
        
    def get_device_type(self, udi):    
        return self.get_property(udi, "DriveConnectionInterface")
        
    def get_device_path(self, udi):    
        return self.get_property(udi, "DeviceFile")
        
    def get_property(self, udi, name="DeviceIsMounted"):    
        dev_obj = self.bus.get_object("org.freedesktop.UDisks", udi)
        device_props = dbus.Interface(dev_obj, dbus.PROPERTIES_IFACE)        
        return device_props.Get('org.freedesktop.UDisks.Device', name)        

udisks = UDisks()    