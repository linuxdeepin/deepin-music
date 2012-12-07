

import gobject
import gtk

import dbus #@UnusedImport
import dbus.glib #@UnusedImport


from player import Player
from logger import Logger

OBJ_PATH = '/im/pidgin/purple/PurpleObject'
INTERFACE = 'im.pidgin.purple.PurpleInterface2'
SERVICE = 'im.pidgin.purple.PurpleService'

class PidginStatusPlugin(Logger):
    PLUGIN_NAME = "PidginStatus"
    PLUGIN_DESC = "Plugin to show Current player track in pidgin"
    PLUGIN_VERSION = "0.1"
    PLUGIN_AUTHOR = "RDeAngelis"
    PLUGIN_WEBSITE = ""
    
    def __init__(self):
        self.__lastsong = None
        self.sbus = dbus.SessionBus()
        Player.connect("instant-new-song", self.on_new_song)
        self.__connected_to_pidgin = False
        self.__check_pidgin_presence()
        self.on_new_song(Player, Player.song)
        gobject.timeout_add(10000, self.__check_pidgin_presence)
        gtk.quit_add(0,self.reset_status)

    def reset_status(self):
        if not self.__connected_to_pidgin: return
        try: 
            status = self.get_meth("")
        except:
            return
        self.change_meth(status, "")

    def __check_pidgin_presence(self):
        try: 
            obj = self.sbus.get_object(SERVICE, OBJ_PATH)
        except:
            if self.__connected_to_pidgin:
                self.__connected_to_pidgin = False
                self.loginfo("disconnect from pidgin")
        else:
            interface = dbus.Interface(obj, INTERFACE)
            self.change_meth = interface.__getattr__("PurpleSavedstatusSetMessage")
            self.get_meth = interface.__getattr__("PurpleSavedstatusGetCurrent")
            if not self.__connected_to_pidgin:
                self.__connected_to_pidgin = True
                self.loginfo("connect to pidgin")
                if self.__lastsong:
                    self.on_new_song(None, self.__lastsong)
        return False

    def on_new_song(self, player, song):
        self.__lastsong = song
        if not song or not self.__connected_to_pidgin: return
        try: 
            status = self.get_meth("")
        except:
            self.__check_pidgin_presence()
            self.on_new_song(player,song)
            return

        title = ""
        artist = ""
        album = ""
        
        if song.get_str("title"):
            title = song.get_str("title")
        if song.get_str("artist"):
            artist = song.get_str("artist")
        if song.get_str("album"):
            album = song.get_str("album")
        
        status_msg = "\xe2\x99\xaa %s: %s (%s) \xe2\x99\xaa" %(artist, title, album)
        self.loginfo("Change pidgin status to \"%s\"",status_msg)
        self.change_meth(status, status_msg)
