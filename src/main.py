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
from cover_download import SimpleFetchManager
from nls import _
from constant import PROGRAM_VERSION


class DeepinMusicApp(Logger):
    app_instance = None
    app_ready = False
    db_ready = False
    splash = None
    
    def __init__(self):
        
        (self.options, self.args) = self.get_options().parse_args()
        
        # Run preload.
        self.run_preload()
        
        # initial mainloop setup. The actual loop is started later,
        self.mainloop_init()
        
        #initialize DbusManager
        import dbus_manager
        self.check_result = dbus_manager.check_exit(self.options, self.args)

        if self.check_result == "exit":
            
            # fix from dock start program cursor bus bug
            import gtk
            gtk.gdk.notify_startup_complete()
            
            sys.exit(0)
            
        elif self.check_result == "command":    
            if not self.options.StartAnyway:
                sys.exit(0)
            
        self.dbus = dbus_manager.DeepinMusicDBus()    
        
        #load the rest.
        self.__init()
        
        from widget.instance import mainloop    
        mainloop()
            
        
    def run_preload(self):  
        if self.options.Debug is not None:
            import logger
            try: logger.setLevelNo(int(self.options.Debug))
            except: 
                print "Debug level incorrect"
                sys.exit(0)
            
        if self.options.MimetypeSupport:
            from common import FORMATS
            print "Mimetypes supported: ",
            print ",".join([ ",".join(i._mimes) for i in FORMATS ])
            print "Missing Gstreamer plugins for full support: N/A "
            sys.exit(0)
            
        if self.options.ShowVersion:
            self.version()
            sys.exit(0)
        
    def get_options(self):    
        
        from optparse import OptionParser, OptionGroup, IndentedHelpFormatter
        class OverrideHelpFormatter(IndentedHelpFormatter):
            """
                Merely for translation purposes
            """
            def format_usage(self, usage):
                return '%s\n' % usage
            
        usage = _("Usage: deepin-music-player [OPTION]... [URI]")    
        optionlabel = _('Options') # Merely for translation purposes
        p = OptionParser(usage=usage, add_help_option=False,
            formatter=OverrideHelpFormatter())
        
        group = OptionGroup(p, _('Control Options'))
        group.add_option("-n", "--next", dest="Next", 
                         action="store_true", default=False, help=_("Play the next track"))
        group.add_option("-p", "--prev", dest="Prev",
                         action="store_true", default=False, help=_("Play the previous track"))
        group.add_option("-t", "--play-pause", dest="PlayPause", 
                         action="store_true", default=False, help=_("Pause or resume playback"))        
        group.add_option("-f", "--forward", dest="Forward", 
                         action="store_true", default=False, help=_("Seek Forward"))
        group.add_option("-r", "--rewind", dest="Rewind", 
                         action="store_true", default=False, help=_("Seek Backward"))
        group.add_option("-s", "--stop", dest="Stop", 
                         action="store_true", default=False, help=_("Stop playback"))
        
        p.add_option_group(group)
        
        group = OptionGroup(p, _('Volume Options'))
        group.add_option("-v", "--change-vol", dest="ChangeVolume",
                         action="store", default=None, help=_("Change Volume (VOLUME 0.0-1.0)"))
        p.add_option_group(group)
        
        group = OptionGroup(p, _('Track information Options'))
        group.add_option("--get-title", dest="GetTitle", 
                         action="store_true", default=False, help=_("Print the title of current track"))
        group.add_option("--get-album", dest="GetAlbum",
                         action="store_true", default=False, help=_("Print the album of current track"))
        group.add_option("--get-artist", dest="GetArtist", 
                         action="store_true", default=False, help=_("Print the artist of current track"))
        group.add_option("--get-length", dest="GetLength", 
                         action="store_true", default=False, help=_("Print the length of current track"))
        group.add_option("--get-path", dest="GetPath", 
                         action="store_true", default=False, help=_("Print the path of current track"))
        group.add_option("--current-position", dest="CurrentPosition",
                action="store_true", default=False, help=_("Print current playback position"))
        p.add_option_group(group)
        
        group = OptionGroup(p, _('Other Options'))
        group.add_option("-h", "--help", action="help",
                help=_("Show this help message and exit"))
        group.add_option("--new", dest="NewInstance", action="store_true",
                default=False, help=_("Start new instance"))
        group.add_option("--version", dest="ShowVersion", action="store_true",
                help=_("Show program's version number and exit."))
        group.add_option("--start-minimized", dest="StartMinimized",
                action="store_true", default=False, help=_("Start minimized"))
        group.add_option("--toggle-visible", dest="GuiToggleVisible",
                action="store_true", default=False,
                help=_("Toggle visibility of the GUI (if possible)"))
        group.add_option("--start-anyway", dest="StartAnyway",
                action="store_true", default=False, 
                         help=_("Make control options like --play start program if it is not running"))
        p.add_option_group(group)
        
        group = OptionGroup(p, _('Development/Debug Options'))
        group.add_option("--debug", dest="Debug", action="store", default=None, help=_("Change debug level (0-9)"))
        # group.add_option('--startgui', dest='StartGui', action='store_true', default=False)
        # group.add_option('--no-dbus', dest='Dbus', action='store_false', default=True, help="Disable D-Bus support")
        group.add_option("--mimetype-support", action="store_true", dest="MimetypeSupport", default=False, 
                         help=_("Show information about supported audio file"))
        p.add_option_group(group)
        
        return p
    
    def version(self):
        print "%s %s" % (_("DMusic"), PROGRAM_VERSION)
        
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
        # Loading configure.
        self.loginfo("Loading settings...")
        from config import config        
        config.load()
        
        # Show splash.
        self.__show_splash()
        
        # Loading theme file.
        self.loginfo("Loading application theme...")
        from widget.skin import app_theme        
        

        # Loading MediaDB.
        self.loginfo("Loading MediaDB...")
        from library import MediaDB
        MediaDB.connect("loaded", self.on_db_loaded)
        MediaDB.load()
        
        # Loading WebcastDB
        from webcast_library import WebcastDB
        WebcastDB.load()
        
        # Loading Chinese to Pinyin DB.
        from pinyin import TransforDB        
        TransforDB.load()        
        
        # initialize Gui
        self.loginfo("Initialize Gui...")
        from widget.instance import DeepinMusic        
        self.app_instance = DeepinMusic()
        self.app_instance.connect("ready", self.on_ready_cb)

        if self.options.StartMinimized:
            self.app_instance.window.iconify()
        
    def on_ready_cb(self, app):
        self.app_ready = True
        import glib
        if self.splash is not None:
            glib.idle_add(self.splash.destroy)
        self.post_start()
        
    def on_db_loaded(self, *args, **kwargs):    
        self.db_ready = True
        self.post_start()
        
    def post_start(self):    
        if self.db_ready and self.app_ready:
            # restore = True
            if self.app_instance:
                current_view = self.app_instance.playlist_ui.get_selected_song_view()
                from utils import convert_args_to_uris
                args = convert_args_to_uris(self.args)
                if len(args) > 0:
                    if current_view:
                        # restore = False
                        current_view.add_file(args[0], play=True)
                if args[1:]:
                    if current_view:
                        current_view.async_add_uris(args[1:], False)
            # if restore:
            #     from player import Player            
            #     Player.load()
                
            if self.options.StartAnyway and self.check_result == "command":
                import dbus_manager
                dbus_manager.run_commands(self.options, self.dbus)
                
            self.start_fetch_manager()
            
    def start_fetch_manager(self):
        SimpleFetchManager()
                
    def __show_splash(self):            
        import widget
        from config import config
        self.splash = widget.show_splash(config.getboolean("setting", "use_splash"))

if __name__ == "__main__":
    DeepinMusicApp()
    
    
    
