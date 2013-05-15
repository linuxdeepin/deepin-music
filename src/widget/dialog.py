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

import utils
from nls import _
from helper import Dispatcher


class WinDir(gtk.FileChooserDialog):
    '''Open chooser dir dialog'''

    def __init__(self, return_uri=True, title=_("Select Directory")):
        gtk.FileChooserDialog.__init__(self, title, None, gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                       (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                        gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        self.return_uri = return_uri
        # self.set_modal(True)
        
    def run(self):    
        Dispatcher.emit("dialog-run")
        response = gtk.FileChooserDialog.run(self)
        folder = None
        if response == gtk.RESPONSE_OK:
            if self.return_uri:
                folder = self.get_uri()
            else:
                folder = self.get_filename()
        self.destroy()    
        Dispatcher.emit("dialog-close")
        return folder
    
class WinFile(gtk.FileChooserDialog):    
    '''Open chooser file dialog'''
    def __init__(self, return_uri=True, title=_("Select file")):    
        gtk.FileChooserDialog.__init__(self,title, None, gtk.FILE_CHOOSER_ACTION_OPEN,
                                       (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                        gtk.STOCK_OPEN, gtk.RESPONSE_OK))
             
        self.set_modal(True)
        self.__return_uri = return_uri
        # self.set_current_folder(get_music_dir())

    def run(self):
        Dispatcher.emit("dialog-run")
        response = gtk.FileChooserDialog.run(self)
        folder = None
        # self.set_modal(True)
        if response == gtk.RESPONSE_OK:
            if self.__return_uri:
                folder = self.get_uri()                
            else:    
                folder = self.get_filename()                
        self.destroy()
        Dispatcher.emit("dialog-close")
        return folder
    
class WindowExportPlaylist(gtk.FileChooserDialog):
    def __init__(self,songs):
        self.songs = songs
        gtk.FileChooserDialog.__init__(self, _("Export playlist"), None,
             gtk.FILE_CHOOSER_ACTION_SAVE,
             (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,gtk.STOCK_SAVE, gtk.RESPONSE_OK))
             
        self.set_modal(True)
        self.set_current_name("playlist.m3u")
        self.set_current_folder(os.path.expanduser("~/"))
        
        option_menu = gtk.OptionMenu()
        option_menu.set_size_request(155, -1)
        self.menu = gtk.Menu()
        self.menu.set_size_request(155, -1)
        self.make_menu_item("M3U (*.m3u)", "m3u")
        self.make_menu_item("pls (*.pls)", "pls")
        self.make_menu_item("xspf (*.xspf)", "xspf")
        option_menu.set_menu(self.menu)
        hbox = gtk.HBox()
        hbox.pack_end(option_menu, False, False)
        self.vbox.pack_start(hbox, False, False)
        hbox.show_all()
        
    def make_menu_item(self, name, data):    
        item = gtk.MenuItem(name)
        item.connect("activate", self.set_save_filetype, data)
        item.show()
        self.menu.append(item)
        
    def set_save_filetype(self, item, filetype):
        self.set_current_name("%s.%s" % ("playlist", filetype))

    def run(self):
        Dispatcher.emit("dialog-run")
        response = gtk.FileChooserDialog.run(self)
        self.set_modal(True)
        if response == gtk.RESPONSE_OK:
            filename = self.get_filename()
            if utils.get_ext(filename) in [".m3u",".pls",".xspf"]:
                utils.export_playlist(self.songs,filename,utils.get_ext(filename,False))
            else:
                pl_type = ".m3u"
                filename = filename+"."+pl_type
                utils.export_playlist(self.songs,filename,pl_type)
        self.destroy()
        Dispatcher.emit("dialog-close")
        
class WindowLoadPlaylist(gtk.FileChooserDialog):
    def __init__(self):
        
        gtk.FileChooserDialog.__init__(self, _("Load playlist"), None,
             gtk.FILE_CHOOSER_ACTION_OPEN,
             (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN, gtk.RESPONSE_OK))
             

        ffilter = gtk.FileFilter()
        ffilter.set_name("Playlist files")
        ffilter.add_mime_type("audio/mpegurl")
        ffilter.add_mime_type("audio/x-mpegurl")
        ffilter.add_mime_type("audio/m3u")
        ffilter.add_mime_type("audio/x-scpls")
        ffilter.add_mime_type("application/xspf+xml")
        ffilter.add_pattern("*.m3u")
        ffilter.add_pattern("*.pls")
        ffilter.add_pattern("*.xspf")
        # self.add_filter(ffilter)
        
        self.set_current_folder(os.path.expanduser("~/"))

    def run(self):
        uri = None
        Dispatcher.emit("dialog-run")
        response = gtk.FileChooserDialog.run(self)
        self.set_modal(True)
        if response == gtk.RESPONSE_OK:
            uri =  self.get_uri()
        self.destroy()
        Dispatcher.emit("dialog-close")
        return uri
    
    
