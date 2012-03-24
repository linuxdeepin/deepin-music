#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 Deepin, Inc.
#               2011 Wang Yong
#
# Author:     Wang Yong <lazycat.manatee@gmail.com>
#             hou  shaohui <houshaohui@linuxdeepin.com>
#
# Maintainer: hou shaohui <houshaohui@linuxdeepin.com>
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

import pygtk
pygtk.require('2.0')
import gtk
# from dtk.ui.window import Window
from ui_toolkit import *
from dtk.ui.utils import *
from dtk.ui.paned import *


class PaneWindow:
    '''Horizontal Pane test '''
    def __init__(self):
        window = Window()
        window.set_title('Panes')
        window.set_border_width(10)
        window.set_size_request(225, 150)

        window.connect('destroy', lambda w: gtk.main_quit())

        hpaned = gtk.HPaned()
        # hpaned.connect("button-press-event", self.hpaned_press_cb)
        # hpaned.connect("expose-event", self.hpaned_expose_cb)
        button1 = gtk.Button('Resize')
        button1.set_size_request(100, -1)
        button2 = gtk.Button('Me!')

        hpaned.add1(button1)
        hpaned.add2(button2)
        # hpaned.pack1(button1, True, True)
        # hpaned.pack2(button2, True, True)

        window.window_frame.pack_start(hpaned)
        window.show_all()

        gtk.main()
        
    def hpaned_press_cb(self, widget, event):
        print event.x, event.y
        
    def hpaned_expose_cb(self, widget, event):    
        print widget.get_children()
        # cr = widget.window.cairo_create()
        # pixbuf = app_theme.get_pixbuf("skin/fill.png").get_pixbuf()
        # rect = widget.allocation
        
        # # Clear color to transparent window.
        # cr.set_source_rgba(0.0, 0.0, 0.0, 0.0)
        # cr.set_operator(cairo.OPERATOR_SOURCE)
        # cr.paint()
        
        # draw_pixbuf(cr, pixbuf, rect.x, rect.y)
        
        # propagate_expose(widget, event)
        # return True

        

    

if __name__ == '__main__':
    PaneWindow()



