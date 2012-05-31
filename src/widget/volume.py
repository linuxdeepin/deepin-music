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
from dtk.ui.button import ToggleButton

from widget.scalebar import HScalebar
from widget.skin import app_theme


class VolumeButton(gtk.HBox):
    
    def __init__(self, init_value=100, min_value=0, max_value=100):
        
        super(VolumeButton, self).__init__()
        self.volume_progressbar = HScalebar(
            app_theme.get_pixbuf("volume/fg.png"),
            app_theme.get_pixbuf("volume/bg.png"),
            app_theme.get_pixbuf("volume/point_normal.png"),
            )
        
        self.volume_button = ToggleButton(
            app_theme.get_pixbuf("volume/high_normal.png"),
            app_theme.get_pixbuf("volume/mute_normal.png"),
            app_theme.get_pixbuf("volume/high_hover.png"),
            app_theme.get_pixbuf("volume/mute_hover.png"),
            app_theme.get_pixbuf("volume/high_press.png"),
            app_theme.get_pixbuf("volume/mute_press.png"),
            )
        
        # Init widget.
        self.volume_progressbar.set_size_request(42, 8)
        self.volume_progressbar.set_range(min_value, max_value)
        self.volume_progressbar.set_value(init_value)
        
        # Signals.
        self.volume_button.connect("enter-notify-event", self.enter_volume_button)
        self.volume_progressbar.connect("leave-notify-event", self.leave_volume_progressbar)
        self.volume_progressbar.get_adjustment().connect("value-changed", self.moniter_volume_change)
        
        # hide.
        self.volume_progressbar.set_no_show_all(True)        
        
        # Pack.
        volume_align = gtk.Alignment()
        volume_align.set(0.5, 0.5, 0, 0)
        volume_align.add(self.volume_progressbar)
        
        button_align = gtk.Alignment()
        button_align.set(0.5, 0.5, 0, 0)
        button_align.add(self.volume_button)

        self.pack_start(button_align, False, False)
        self.pack_start(volume_align, False, False)
        
    def enter_volume_button(self, widget, event):
        self.change_progressbar_status()
    
    def leave_volume_progressbar(self, widget, event):
        self.change_progressbar_status(True)
    
    def change_progressbar_status(self, hide=False):
        if not hide:
            self.volume_progressbar.set_no_show_all(False)
            self.volume_progressbar.show_all()
        else:    
            self.volume_progressbar.hide_all()
            self.volume_progressbar.set_no_show_all(True)
            
    def update_volume_button_pixbuf(self, name):
        group = (
            app_theme.get_pixbuf("volume/%s_normal.png" % name),
            app_theme.get_pixbuf("volume/%s_hover.png" % name),
            app_theme.get_pixbuf("volume/%s_press.png" % name)
            )
        self.volume_button.set_inactive_pixbuf_group(group)
        
    
    def moniter_volume_change(self, adjustment):
        high_value = adjustment.get_upper()
        lower_value = adjustment.get_lower()
        total_value = high_value - lower_value
        current_value = adjustment.get_value() - lower_value
        
        if current_value > 0:
            if self.volume_button.get_active():
                self.volume_button.set_active(False)
        if current_value == 0:
            self.update_volume_button_pixbuf("zero")
        elif 0 < current_value  <= total_value * 0.3: 
            self.update_volume_button_pixbuf("lower")
        elif total_value * 0.3 < current_value <= total_value * 0.6:
            self.update_volume_button_pixbuf("middle")
        elif total_value * 0.6 < current_value <= total_value:
            self.update_volume_button_pixbuf("high")
