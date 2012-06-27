#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Hou Shaohui
# 
# Author:     Hou Shaohui <lazycat.manatee@gmail.com>
# Maintainer: Hou Shaohui <lazycat.manatee@gmail.com>
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
import gobject

class VolumeButton(gtk.EventBox):
    
    __gsignals__ = {
        "value-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_INT,))
        }
    
    def __init__(self, init_value=100, min_value=0, max_value=100,
                 point_dpixbuf=None,
                 bg_dpixbuf=None,
                 fg_dpixbuf=None,
                 zero_dpixbuf_group=None,
                 lower_dpixbuf_group=None,
                 middle_dpixbuf_group=None,
                 high_dpixbuf_group=None,
                 mute_dpixbuf_group=None
                 ):
        super(VolumeButton, self).__init__()
        
    def get_dpixbuf_group(self, name):    
        