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

import dbus
from player import Player
from logger import Logger
from config import config
from cover_manager import CoverManager
from nls import _

NANOSECOND = 1/0.000000001
MICROSECOND = 1/0.000001

ORG_MPRIS_MEDIAPLAYER2 = "org.mpris.MediaPlayer2"
ORG_MPRIS_MEDIAPLAYER2_PLAYER = "org.mpris.MediaPlayer2.Player"
ORG_MPRIS_MEDIAPLAYER2_TRACKLIST = "org.mpris.MediaPlayer2.TrackList"

MPRIS2_INTROSPECTION = \
"""<node name="/org/mpris/MediaPlayer2">
    <interface name="org.freedesktop.DBus.Introspectable">
        <method name="Introspect">
            <arg direction="out" name="xml_data" type="s"/>
        </method>
    </interface>
    <interface name="org.freedesktop.DBus.Properties">
        <method name="Get">
            <arg direction="in" name="interface_name" type="s"/>
            <arg direction="in" name="property_name" type="s"/>
            <arg direction="out" name="value" type="v"/>
        </method>
        <method name="GetAll">
            <arg direction="in" name="interface_name" type="s"/>
            <arg direction="out" name="properties" type="a{sv}"/>
        </method>
        <method name="Set">
            <arg direction="in" name="interface_name" type="s"/>
            <arg direction="in" name="property_name" type="s"/>
            <arg direction="in" name="value" type="v"/>
        </method>
        <signal name="PropertiesChanged">
            <arg name="interface_name" type="s"/>
            <arg name="changed_properties" type="a{sv}"/>
            <arg name="invalidated_properties" type="as"/>
        </signal>
    </interface>
    <interface name="org.mpris.MediaPlayer2.Player">
        <method name="Next"/>
        <method name="Previous"/>
        <method name="Pause"/>
        <method name="PlayPause"/>
        <method name="Stop"/>
        <method name="Play"/>
        <method name="Seek">
            <arg direction="in" name="Offset" type="x"/>
        </method>
        <method name="SetPosition">
            <arg direction="in" name="TrackId" type="o"/>
            <arg direction="in" name="Position" type="x"/>
        </method>
        <method name="OpenUri">
            <arg direction="in" name="Uri" type="s"/>
        </method>
        <signal name="Seeked">
            <arg name="Position" type="x"/>
        </signal>
        <property name="PlaybackStatus" type="s" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="LoopStatus" type="s" access="readwrite">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="Rate" type="d" access="readwrite">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="Shuffle" type="b" access="readwrite">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="Metadata" type="a{sv}" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="Volume" type="d" access="readwrite">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="false"/>
        </property>
        <property name="Position" type="x" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="false"/>
        </property>
        <property name="MinimumRate" type="d" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="MaximumRate" type="d" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="CanGoNext" type="b" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="CanGoPrevious" type="b" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="CanPlay" type="b" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="CanPause" type="b" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="CanSeek" type="b" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="true"/>
        </property>
        <property name="CanControl" type="b" access="read">
            <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="false"/>
        </property>
    </interface>
    <interface name="org.mpris.MediaPlayer2">
        <method name="Raise"/>
        <method name="Quit"/>
        <annotation name="org.freedesktop.DBus.Property.EmitsChangedSignal" value="false"/>
        <property name="CanQuit" type="b" access="read"/>
        <property name="CanRaise" type="b" access="read"/>
        <property name="HasTrackList" type="b" access="read"/>
        <property name="Identity" type="s" access="read"/>
        <property name="DesktopEntry" type="s" access="read"/>
        <property name="SupportedUriSchemes" type="as" access="read"/>
        <property name="SupportedMimeTypes" type="as" access="read"/>
    </interface>
</node>"""

class Mpris2Adapter(dbus.service.Object, Logger):
    """ interface defined by org.mpris.MediaPlayer2"""

    _properties = [
        # application
        "CanQuit",
        "CanRaise",
        "DesktopEntry",
        "HasTrackList",
        "Identity",
        "SupportedMimeTypes",
        "SupportedUriSchemes",
        # player
        "CanControl",
        "CanGoNext",
        "CanGoPrevious",
        "CanPause",
        "CanPlay",
        "CanSeek",
        "LoopStatus",
        "MaximumRate",
        "MinimumRate",
        "Metadata",
        "PlaybackStatus",
        "Position",
        "Rate",
        "Shuffle",
        "Volume",
    ]

    def __init__(self, dmusic, bus):
#        super(Mpris2Adapter, self).__init__(self, bus, unicode('/org/mpris/MediaPlayer2'))
        dbus.service.Object.__init__(self, bus, '/org/mpris/MediaPlayer2')
        self.dmusic = dmusic
        self.cover_cache = {}

    # Introspectable methods
    @dbus.service.method("org.freedesktop.DBus.Introspectable")
    def Introspect(self):
        return MPRIS2_INTROSPECTION

    ## Properties methods
    @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='ss', out_signature='v')
    def Get(self, interface, prop):
        if prop in self._properties:
            return getattr(self, prop)
        else:
            return None

    @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='s', out_signature='a{sv}')
    def GetAll(self, interface):
        res = {}
        for prop in self._properties:
            res[prop] = getattr(self, prop)
        return res

    @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='ssv')
    def Set(self, interface, prop, value):
        if prop in self._properties:
            setattr(self, prop, value)

    ## Properties signals

    @dbus.service.signal(dbus.PROPERTIES_IFACE, signature='sa{sv}as')
    def PropertiesChanged(self, interface, updated, invalid):
        pass

    def populate(self, interface, *prop_names):

        props = {}
        for p in prop_names:
            if type(p) is tuple:
                # NOTE: This is a hack to fix the early-populate problem described
                #       in the comments of on_playback_start()
                p, v = p
                props[p] = v
            else:
                props[p] = getattr(self, p)
        self.logdebug("populate: %s", repr(props))                
        self.PropertiesChanged(interface, props, [])

    ## main methods
    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2)
    def Quit(self):
        self.dmusic.force_quit()

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2)
    def Raise(self):
        self.dmusic.toggle_visible(True)

    ## main properties

    @property
    def CanQuit(self):
        return True

    @property
    def CanRaise(self):
        return True

    @property
    def HasTrackList(self):
        # TODO: implement TrackList
        return False

    @property
    def Identity(self):
        return _("Deepin Music")

    @property
    def DesktopEntry(self):
        return "deepin-music-player"

    @property
    def SupportedUriSchemes(self):
        # TODO: return supported GFile schemes
        return ['file', 'http', 'https', "mms", "rtsp"]

    @property
    def SupportedMimeTypes(self):
        # TODO: return supported GStreamer types
        return [
            'audio/mpeg',
            'audio/ogg',
            'audio/vorbis',
        ]

    ## Player methods
    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER)
    def Next(self):
        Player.next(True)

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER, in_signature='s')
    def OpenUri(self, uri):
        song_view = self.dmusic.playlist_ui.get_selected_song_view()
        if song_view:
            song_view.add_file(uri, True)

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER)
    def Pause(self):
        Player.pause()

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER)
    def Play(self):
        Player.play()

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER)
    def PlayPause(self):
        Player.playpause()

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER)
    def Previous(self):
        Player.previous()

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER, in_signature='x')
    def Seek(self, offset):
        position = Player.get_position()
        position += offset / MICROSECOND
        Player.seek(position, False)

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER, in_signature='ox')
    def SetPosition(self, track_id, position):
        if track_id == self._get_trackid(Player.song):
            position /= MICROSECOND
            Player.seek(position)
        else:
            # treat request as stale
            pass

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_PLAYER)
    def Stop(self):
        Player.stop()

    ## Player properties

    @property
    def CanControl(self):
        return True

    @property
    def CanGoNext(self):
        return True

    @property
    def CanGoPrevious(self):
        return True

    @property
    def CanPause(self):
        # MPRIS -- return true even if currently paused
        if Player.is_playable() and not Player.is_paused():
            return True
        else:
            return False

    @property
    def CanPlay(self):
        # MPRIS -- return true even if currently playing
        return Player.song is not None
    

    @property
    def CanSeek(self):
        return True

    @property
    def LoopStatus(self):
        loop_mode = config.get("setting", "loop_mode", "")
        if not loop_mode:
            return "None"
        if loop_mode == "single_mode":
            return "Track"
        else:
            return "Playlist"

    @LoopStatus.setter
    def LoopStatus(self, value):
        if value == "Playlist":
            config.set("setting", "loop_mode", "order_mode")
        elif value == "Track":    
            config.set("setting", "loop_mode", "single_mode")
        elif value == "None":    
            config.set("setting", "loop_mode", "list_mode")

    @property
    def MaximumRate(self):
        return 1.0

    @property
    def MinimumRate(self):
        return 1.0

    @property
    def PlaybackStatus(self):
        if Player.song is None:
            return "Stopped"
        elif Player.is_paused():
            return "Paused"
        else:
            return "Playing"
        
    @property
    def Position(self):
        pos = Player.get_position() * MICROSECOND
        return dbus.Int64(pos)

    @property
    def Rate(self):
        return 1.0

    @Rate.setter
    def Rate(self, value):
        # Note: Ignore attempts to set.
        # TODO: Does Dmusic support setting rate?
        pass

    @property
    def Metadata(self):
        song = Player.song
        return self.__get_metadata(song)

    @property
    def Shuffle(self):
        loop_mode = config.get("setting", "loop_mode", "")
        if loop_mode != "random_mode":
            return False
        return True

    @Shuffle.setter
    def Shuffle(self, value):
        if value:
            config.set("setting", "loop_mode", "random_mode")
        else:    
            config.set("setting", "loop_mode", "order_mode")

    @property
    def Volume(self):
        # Range: 0.0 ~ 1.0
        return Player.volume

    @Volume.setter
    def Volume(self, value):
        Player.update_volume(value)

    ## Player signals

    @dbus.service.signal(ORG_MPRIS_MEDIAPLAYER2_PLAYER, signature='x')
    def Seeked(self, position):
        pass

    ## TrackList methods

    # TODO: implement according to:
    # http://specifications.freedesktop.org/mpris-spec/latest/TrackList_Node.html

    def GetTracksMetadata(self, track_ids):
        pass

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_TRACKLIST, in_signature='sob')
    def AddTrack(self, uri, after_track, set_as_current):
        pass

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_TRACKLIST, in_signature='o')
    def RemoveTrack(self, trackId):
        pass

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_TRACKLIST, in_signature='o')
    def Goto(self, trackId):
        pass

    @dbus.service.method(ORG_MPRIS_MEDIAPLAYER2_TRACKLIST, out_signature='ao')
    def Tracks(self):
        pass

    ## TrackList properties

    def CanEditTracks(self):
        return False

    ## Helper functions

    def _get_trackid(self, song):
        try:
            idx = Player.source.get_songs().index(song)
            return "/com/linuxdeepin/DMusic/CurrentPlaylist/Track%d" % (idx+1)
        except:
            # TODO: find a better response than this
            #       (but on the other hand, this shouldn't happen anyway)
            return "/com/linuxdeepin/DMusic/NotInCurrentPlaylist"

    def _parse_trackid(self, track):
        if track.startswith("/com/linuxdeepin/DMusic/CurrentPlaylist/Track"):
            return int(track[track.find("Track") + 5:]) - 1
        else:
            return None

    def __get_metadata(self, song):
        meta = {}

        if song is not None:
            ## MPRIS v2 meta map, defined at http://xmms2.org/wiki/MPRIS_Metadata

            meta['mpris:trackid'] = dbus.ObjectPath(self._get_trackid(song))
            meta['xesam:url'] = song.get_str("uri")
            meta['xesam:title'] = song.get_str("title")
            artist = song.get_str("artist")
            if artist:
                meta['xesam:artist'] = dbus.types.Array([artist], signature='s')

            album = song.get_str("album")
            if album:
                meta['xesam:album'] = album

            genre = song.get_str("genre")
            if genre:
                meta['xesam:genre'] = dbus.types.Array([genre], signature='s')

            tracklen = song.get("#duration")
            if tracklen:
                meta['mpris:length'] = dbus.types.Int64(tracklen / 1000 * MICROSECOND)

            # this is a workaround, write data to a tmp file and return name
            meta['mpris:artUrl'] = "file://" + (CoverManager.get_cover(song, False) or CoverManager.default_cover)

        return dbus.types.Dictionary(meta, signature='sv', variant_level=1)
