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

import gst
import gobject

from logger import Logger

mp3_quality = {
    "cbr" : (64, 96, 128, 192, 256),
    "abr" : (64, 96, 128, 192, 256),
    "vbr" : (9, 7, 5, 3, 1),
    }

mp3_mode = {
    "cbr" : (0, "bitrate"),
    "abr" : (3, "vbr-mean-bitrate"),
    "vbr" : (4, "vbr-quality")
    }

ogg_quality = (0, 0.2, 0.4, 0.6, 0.8)

FORMAT_PERCENT_SCALE = 10000
CLOCK_SPEED = 10

class TrancoderError(Exception):
    pass

class Transcoder(Logger):
    pipe = "gnomevfssrc location=\"%s\" ! decodebin name=decoder ! audioconvert ! %s name=encoder ! gnomevfssink location=\"%s\""
    is_eos = False
    def __init__(self, source, output, format="mp3", quality=3, mode="cbr"):
        
        if format == "mp3":
            encoder = "lame quality=2 xingheader=0 vbr=%d %s=%d" % (mp3_mode[mode][0], mp3_mode[mode][1], mp3_quality[mode][quality])
        elif format == "ogg":
            encoder = "vorbisenc quality=%d" % ogg_quality[quality]
        elif format == "flac":    
            encoder = "flacenc"
        else:
            raise TrancoderError("Format unknown")

        pipe = self.pipe % (source, encoder, output)
        self.loginfo("build pipeline \"%s\"", pipe)
        try: self.pipeline = gst.parse_launch (pipe);
        except gobject.GError: 
            self.logexception("failed to build the pipeline")
            raise TrancoderError
        
        
        self.encoder = self.pipeline.get_by_name("encoder")
        
        decoder = self.pipeline.get_by_name("decoder")
        decoder.connect("unknown-type", self.unknown_type)
        bus = self.pipeline.get_bus()
        bus.connect('message', self.message)
        bus.add_signal_watch()
        
        state_ret = self.pipeline.set_state(gst.STATE_PLAYING);
        timeout = 10
        while state_ret == gst.STATE_CHANGE_ASYNC and not self.is_eos and timeout > 0:
            state_ret, __state, __pending_state = self.pipeline.get_state(1 * gst.SECOND);
            timeout -= 1
            
        if state_ret != gst.STATE_CHANGE_SUCCESS:
            raise TrancoderError("Failed change to playing")
        else:
            self.loginfo("starting encoding")
        self.duration = self.get_duration()
        
    def is_finish(self):
        return self.is_eos
    
    def get_ratio(self):
        ratio = float(self.get_position()) / float(self.duration) * float(100)
        return  ratio
    
    def get_position(self):
        if gst.STATE_NULL != self.get_state():
            """ Use a encoder instead of pipeline because filesink and gnomevfs sink return wrong value in gstreamer 0.10.5"""
            try: p = self.encoder.query_position(gst.FORMAT_TIME)[0]
            except gst.QueryError: p = 0
            p //= gst.MSECOND
            return p
        return 0

    def get_duration(self):
        if self.get_state() != gst.STATE_NULL:
            try:
                query = gst.query_new_duration(gst.FORMAT_TIME)
                if self.encoder.query(query):
                    total = query.parse_duration()[1]
                else: return 0
            except gst.QueryError: total = 0
            total //= gst.MSECOND
            return total
        else:
            return 0
        
    def unknown_type(self, *param):
        self.logwarn("input file type unknown")
        raise TrancoderError
    
    def stop(self):
        self.eos()
        
    def eos(self):
        state_ret = self.pipeline.set_state(gst.STATE_NULL)
        if state_ret != gst.STATE_CHANGE_SUCCESS:
            self.Logwarn("failed stop pipeline")
        self.is_eos = True
            
    def get_state(self):
        __state_ret, state, __pending_state = self.pipeline.get_state();
        return state
    
    def message(self, bus, message):
        if message.type == gst.MESSAGE_EOS:
            self.eos()

        elif message.type == gst.MESSAGE_TAG:
            pass
            """taglist = message.parse_tag()
            for key in taglist.keys():
                if IDS.has_key(key): 
                    if key=="bitrate":
                        value = int(taglist[key]/100)
                    elif isinstance(taglist[key],long):
                        value = int(taglist[key])   
                    else:
                        value = taglist[key]
                    song[IDS[key]] = value
                    print key,":",value
            song["tagged"] = True
            is_tagged = True"""

        elif message.type == gst.MESSAGE_ERROR:
            err, debug = message.parse_error()
            self.logerror("failed decode (error: %s,%s)", err, debug)
            self.eos()
            #raise GstTagReadingError
            
if __name__ == "__main__":            
    import sys
    import gtk
    from widget.song_editor import SongEditor
    a = SongEditor()
    a.window.show_all()
    gtk.main()
