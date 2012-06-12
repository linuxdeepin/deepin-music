#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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

import sys
from logger import Logger

class DeepinMusicApp(Logger):
    app_instance = None
    app_ready = False
    db_ready = False
    
    def __init__(self):
        
        (self.options, self.args) = self.get_options().parse_args()
        
        if self.options.ShowVersion:
            self.version()
            return
        
        # initial mainloop setup. The actual loop is started later,
        self.mainloop_init()
        
        #initialize DbusManager
        import dbus_manager
        result = dbus_manager.check_exit(self.options, self.args)

        if result == "exit":
            sys.exit(0)
        elif result == "command":    
            sys.exit(0)
            
        self.dbus = dbus_manager.DeepinMusicDBus()    
        
        #load the rest.
        self.__init()

        
    def get_options(self):    
        
        from optparse import OptionParser, OptionGroup, IndentedHelpFormatter
        class OverrideHelpFormatter(IndentedHelpFormatter):
            """
                Merely for translation purposes
            """
            def format_usage(self, usage):
                return '%s\n' % usage
            
        usage = "Usage: deepin-music-player [OPTION]... [URI]"    
        p = OptionParser(usage=usage, add_help_option=False,
            formatter=OverrideHelpFormatter())
        
        group = OptionGroup(p, 'Playback Options')
        group.add_option("-n", "--next", dest="Next", action="store_true", default=False, help="Play the next track")
        group.add_option("-p", "--prev", dest="Prev", action="store_true", default=False, help="Play the previous track")
        group.add_option("-t", "--play-pause", dest="PlayPause", action="store_true", default=False, help="Pause or resume playback")        
        group.add_option("-f", "--forward", dest="Forward", action="store_true", default=False, help="Forward playback")
        group.add_option("-R", "--rewind", dest="Rewind", action="store_true", default=False, help="Rewind playback")
        p.add_option_group(group)
        
        group = OptionGroup(p, 'Track Options')
        group.add_option("--get-title", dest="GetTitle", action="store_true", default=False, help="Print the title of current track")
        group.add_option("--get-album", dest="GetAlbum", action="store_true", default=False, help="Print the album of current track")
        group.add_option("--get-artist", dest="GetArtist", action="store_true", default=False, help="Print the artist of current track")
        group.add_option("--get-length", dest="GetLength", action="store_true", default=False, help="Print the length of current track")
        group.add_option("--get-path", dest="GetPath", action="store_true", default=False, help="Print the path of current track")
        group.add_option("--current-position", dest="CurrentPosition",
                action="store_true", default=False, help="Print the current playback position as time")
        p.add_option_group(group)
        
        group = OptionGroup(p, 'Other Options')
        group.add_option("--new", dest="NewInstance", action="store_true",
                default=False, help="Start new instance")
        group.add_option("-h", "--help", action="help",
                help="Show this help message and exit")
        group.add_option("--version", dest="ShowVersion", action="store_true",
                help="Show program's version number and exit.")
        group.add_option("--toggle-visible", dest="GuiToggleVisible",
                action="store_true", default=False,
                help="Toggle visibility of the GUI (if possible)")
        p.add_option_group(group)
        
        return p
    
    def version(self):
        print "Deepin Music Player 1.0"
        
        
    def mainloop_init(self):    
        import gobject
        gobject.threads_init()
        
        # dbus_init.
        import dbus, dbus.mainloop.glib
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        dbus.mainloop.glib.threads_init()
        dbus.mainloop.glib.gthreads_init()
        
        # gtk_init.
        import gtk
        gtk.gdk.threads_init()
        
    def __init(self):        
        # Loaded theme file.
        self.loginfo("Loading application theme...")
        from widget.skin import app_theme        
        
        # Loaded configure.
        self.loginfo("Loading settings...")
        from config import config        
        config.load()

        # Loaded MediaDB.
        self.loginfo("Loading MediaDB...")
        from library import MediaDB
        MediaDB.connect("loaded", self.on_db_loaded)
        MediaDB.load()
        
        # Loaded Chinese to Pinyin DB.
        from pinyin import TransforDB        
        TransforDB.load()        
        
        # initialize Gui
        self.loginfo("Initialize Gui...")
        from widget.main import DeepinMusic        
        app_instance = DeepinMusic()
        app_instance.connect("ready", self.on_ready_cb)
        import gtk
        gtk.main()

    def on_ready_cb(self, app):
        self.app_ready = True
        self.post_start()
        
    def on_db_loaded(self, *args, **kwargs):    
        self.db_ready = True
        self.post_start()
        
    def post_start(self):    
        if self.db_ready and self.app_ready:
            from player import Player            
            Player.load()
        

if __name__ == "__main__":
    DeepinMusicApp()
    
    
    
