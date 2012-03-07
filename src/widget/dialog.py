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
import os
from findfile import get_music_dir


class WinDir(gtk.FileChooserDialog):
    '''Open chooser dir dialog'''
    def __init__(self, title="Import a folder in the library"):
        gtk.FileChooserDialog.__init__(self, title, None, gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                       (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                        gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        self.set_modal(True)
        self.set_current_folder(get_music_dir())
        
    def run(self):    
        response = gtk.FileChooserDialog.run(self)
        folder = None
        if response == gtk.RESPONSE_OK:
            folder = self.get_filename()
            folder = self.get_uri()
        self.destroy()    
        return folder
    
class WinFile(gtk.FileChooserDialog):    
    '''Open chooser file dialog'''
    def __init__(self,title="Import a file in the library"):    
        gtk.FileChooserDialog.__init__(self,title, None, gtk.FILE_CHOOSER_ACTION_OPEN,
                                       (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                        gtk.STOCK_OPEN, gtk.RESPONSE_OK))
             
        self.set_modal(True)
        self.set_current_folder(get_music_dir())

    def run(self):
        response = gtk.FileChooserDialog.run(self)
        folder = None
        self.set_modal(True)
        if response == gtk.RESPONSE_OK:
            folder = self.get_filename()
            folder = self.get_uri()
        self.destroy()
        return folder
    
    

    