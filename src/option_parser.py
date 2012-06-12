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

import sys
import traceback
import dbus

from optparse import OptionParser
import logger
from library import MediaDB

class DMusicOptionParser(object):
    usage = "usage: %prog [options] [FILE_TO_PLAY ...]"
    def __init__(self):
        
        self.parser = OptionParser(usage=self.usage,version="%prog 1.0")

        self.parser.add_option("-p","-a","--play",dest="file_to_play",default=None,
                  action="callback", callback=self.vararg_callback,
                               help= "play files")
                  
        self.parser.add_option("-q","","--queue",dest="file_to_queue",default=None,
                  action="callback", callback=self.vararg_callback,
                               help= "queue files in the playlist") 
        
        self.parser.add_option("--next", action="store_true", dest="next", default=False,
                                help= "play the next song in the playlist")
                                
        self.parser.add_option("--previous", action="store_true", dest="previous", default=False,
                                help= "play the previous song in the playlist")
                                
        self.parser.add_option("--play-pause", action="store_true", dest="play", default=False,
                                help= "play or pause current playing media")
        
        #self.parser.add_option("--stop", action="store_true", dest="stop", default=False,
        #                        help=_("stop current playing media"))
                                
        self.parser.add_option("--rewind", action="store_true", dest="rewind", default=False,
                                help= "rewind the played song")
                                
        self.parser.add_option("--forward", action="store_true", dest="forward", default=False,
                                help= "forward the played song")
                                
        self.parser.add_option("-c","--current-playing", action="store_true", dest="print_current", default=False,
                                help= "show current playing song")
                                
                                
        self.parser.add_option("-v", "--volume", action="store", dest="volume", default=None,
                                help= "change volume (VOLUME 0.0-1.0)")
                                
        self.parser.add_option("--mimetype-support", action="store_true", dest="print_mimetype_support", default=False,
                                help="show information about supported audio file")

        self.parser.add_option("--dump-gstplayer-state", action="store_true", dest="dump_gstplayer_state", default=False,
                                help= "Dump gstplayer state (for debugging purpose)")

        self.parser.add_option("--library-sanity-check", action="store_true", dest="library_sanity_check", default=False,
                                help="Force a sanity check of the library")

        self.parser.add_option( "--debug", action="store", dest="debug", default=None,
                                help= "change debug level (0-9)")
                                
        (self.options, self.args) = self.parser.parse_args(sys.argv[1:])
        #remove empty parameter
        self.args = filter(lambda arg: arg and arg.strip(),self.args)
        
    def run_preload(self):  
        if self.options.debug is not None:
            try: logger.setLevelNo(int(self.options.debug))
            except: 
                print "Debug level incorrect"
                sys.exit(0)

        if self.options.library_sanity_check:
            MediaDB.set_force_sanity_check()
            
        if self.options.print_current:
            "No playing song"
            sys.exit(0)
            
        if self.options.print_mimetype_support:
            from utils import FORMATS
            print "Mimetypes supported: ",
            print ",".join([ ",".join(i._mimes) for i in FORMATS ])
            print "Missing Gstreamer plugins for full support: N/A "
            sys.exit(0)

    def run_load(self, dmusic_dbus_interface=None):
        if not dmusic_dbus_interface:
            try: 
                bus = dbus.SessionBus()
                obj = bus.get_object("com.linuxdeepin.DMusic","/com/linuxdeepin/DMusic")
                dmusic_dbus_interface = dbus.Interface(obj, 'com.linuxdeepin.DMusic') 
            except:
                traceback.print_exc()
                print "Problem detected with DBus, command line interface is broken..."
                return 

        ret = None
        if self.options.dump_gstplayer_state:
            ret = dmusic_dbus_interface.dump_gstplayer_state()

        if self.options.rewind:
            ret = dmusic_dbus_interface.rewind()
            
        if self.options.forward:
            ret = dmusic_dbus_interface.forward()
            
        if self.options.next:
            ret = dmusic_dbus_interface.next()
            
        if self.options.previous:
            ret = dmusic_dbus_interface.previous()
            
        if self.options.print_current:
            ret = dmusic_dbus_interface.current_playing()
            
        if self.options.play:
            ret = dmusic_dbus_interface.play_pause()
            
        #if self.options.stop:
        #    ret = dmusic_dbus_interface.stop()
            
        if self.options.volume!=None:
            ret = dmusic_dbus_interface.volume(self.options.volume)
            
        if self.options.file_to_play:
            ret = dmusic_dbus_interface.play(self.convert_args_to_uris_string(self.options.file_to_play))
            
        if self.options.file_to_queue:
            ret = dmusic_dbus_interface.enqueue(self.convert_args_to_uris_string(self.options.file_to_queue))

        if self.args:
            ret = dmusic_dbus_interface.enqueue(self.convert_args_to_uris_string(self.args))
           
        if ret:
            print ret
    
    def convert_args_to_uris_string(self,args):
        #TODO: FIXE DEPENDENCIES LOOP
        from utils import make_uri_from_shell_arg
        uris = [ make_uri_from_shell_arg(arg) for arg in args if arg and arg.strip() ]
        return uris
    
    def vararg_callback(self,option, opt_str, value, parser):
        assert value is None
        value = []
        rargs = parser.rargs
        while rargs:
            arg = rargs[0]
        
            # Stop if we hit an arg like "--foo", "-a", "-fx", "--file=f",
            # etc.  Note that this also stops on "-3" or "-3.0", so if
            # your option takes numeric values, you will need to handle
            # this.
            if ((arg[:2] == "--" and len(arg) > 2) or
                (arg[:1] == "-" and len(arg) > 1 and arg[1] != "-")):
                break
            else:
                value.append(arg)
                del rargs[0]

        setattr(parser.values, option.dest, value)
