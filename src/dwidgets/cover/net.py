#! /usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Wang Yong
# 
# Author:     Wang Yong <lazycat.manatee@gmail.com>
# Maintainer: Wang Yong <lazycat.manatee@gmail.com>
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

import time
import socket
import commands
import traceback
import sys
import dbus

def is_network_connected_by_nm():
    sys_bus = dbus.SystemBus()
    proxy = sys_bus.get_object("org.freedesktop.NetworkManager","/org/freedesktop/NetworkManager")
    interface = dbus.Interface(proxy, "org.freedesktop.NetworkManager")
    
    return interface.state() == 70

def is_network_connected_by_route():
    return len(commands.getoutput("route -nNvee").split("\n")) > 2

def is_network_connected():
    '''
    First try using network-manager api, if not installed, then try route.  
    Is network connected, if nothing output from command `route -nNvee`, network is disconnected.
    
    @return: Return True if network is connected or command `route -nNvee` failed.
    '''
    try:
        return is_network_connected_by_nm()
    except:
        print "get network state by dbus failed, then try route"
        traceback.print_exc(file=sys.stdout)
        try:
            return is_network_connected_by_route()
        except Exception, e:
            print "function is_network_connected got error: %s" % e
            traceback.print_exc(file=sys.stdout)
            
            return True

def get_unused_port(address="localhost"):
    s = socket.socket()
    s.bind((address, 0))
    return s.getsockname()[1]

def check_connect_by_port(port, retry_times=6, sleep_time=0.5):
    """
    Check connect has active with given port.
    
    @param port: Test port.
    @param retry_times: Retry times.
    @param sleep_time: Sleep time between retry, in seconds.
    @return: Return True if given port is active.
    """
    ret_val = False
    test_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    retry_time = 0
    while (True):
        try:
            test_socket.connect(("localhost", port))
            ret_val = True
            break
        except socket.error:
            time.sleep(sleep_time)
            retry_time += 1
            if retry_time >= retry_times:
                break
            else:
                continue
    return ret_val
