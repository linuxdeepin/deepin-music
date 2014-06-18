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


# import pygst
# pygst.require("0.10")
import gst
import gobject

from logger import Logger
from nls import _

"""
    explanation of format dicts:
    default:    the default quality to use, must be a member of raw_steps.
    raw_steps:  a value defining the quality of encoding that will be passed
                to the encoder.
    kbs_steps:  a value defining the quality of encoding that will be displayed
                to the user. must be a one-to-one mapping with raw_steps.
    command:    the gstreamer pipeline to execute. should contain exactly one
                python string format operator, like %s or %f, which will be 
                replaced with the value from raw_steps.
    plugins:    the gstreamer plugins needed for this transcode pipeline
    desc:       a description of the encoder to display to the user
"""
FORMATS = {
    "Ogg_Vorbis": {
        "default" : 0.5,
        "default_index" : 5,
        "raw_steps" : [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
        "kbs_steps" : [64, 80, 96, 112, 128, 160, 192, 224, 256, 320],
        "command"   : "vorbisenc quality=%1.1f name=encoder ! oggmux",
        "extension" : "ogg",
        "plugins"   : ["vorbisenc", "oggmux"],
        "desc"      : _("Vorbis is an open source, lossy audio codec with"
                        " high quality output at a lower file size than MP3.")
        },
    
    "FLAC" : {
        "default" : 5,
        "default_index" : 5,
        "raw_steps" : [0, 1, 2, 3, 4, 5, 6, 7, 8, 9],
        "kbs_steps" : [0, 1, 2, 3, 4, 5, 6, 7, 8, 9],
        "command"   : "flacenc quality=%i name=encoder",
        "extension" : "flac",
        "plugins"   : ["flacenc"],
        "desc"      : _("Free Lossless Audio Codec (FLAC) is an open "
                        "source codec that compresses but does not degrade audio "
                        "quality.")
        },
    
    "AAC"       : {
        "default"   : 160000,
        "default_index" : 5,
        "raw_steps" : [32000, 48000, 64000, 96000, 128000, 160000,
                       192000, 224000, 256000, 320000],
        "kbs_steps" : [32, 48, 64, 96, 128, 160, 192, 224, 256, 320],
        "command"   : "faac bitrate=%i name=encoder ! ffmux_mp4",
        "extension" : "m4a",
        "plugins"   : ["faac", "ffmux_mp4"],
        "desc"      : _("Apple's proprietary lossy audio format could give "
                        "higher quality than MP3 at lower bitrates.")

        },
    
    "MP3 (VBR)" : {
        "default"   : 160,
        "default_index" : 5,
        "raw_steps" : [32, 48, 64, 96, 128, 160, 192, 224, 256, 320],
        "kbs_steps" : [32, 48, 64, 96, 128, 160, 192, 224, 256, 320],
        "command"   : "lame vbr=4 vbr-mean-bitrate=%i name=encoder",
        "extension" : "mp3",
        "plugins"   : ["lame"],
        "desc"      : _("A proprietary and older, but also popular, lossy "
                        "audio format. VBR gives higher quality than CBR, but may "
                        "be incompatible with some players.")
            },
    
    "MP3 (CBR)" : {
        "default"   : 160,
        "default_index" : 5,
        "raw_steps" : [32, 48, 64, 96, 128, 160, 192, 224, 256, 320],
        "kbs_steps" : [32, 48, 64, 96, 128, 160, 192, 224, 256, 320],
        "command"   : "lame bitrate=%i name=encoder",
        "extension" : "mp3",
        "plugins"   : ["lame"],
        "desc"      : _("A proprietary and older, but also popular, lossy "
                        "audio format. The output effect of CBR is not good as "
                        "VBR, but is compatible with any player.")

            },
    "WavPack" : {
        "default"   : 2,
        "default_index" : 1,
        "raw_steps" : [1,2,3,4],
        "kbs_steps" : [1,2,3,4],
        "command"   : "wavpackenc mode=%i name=encoder",
        "extension" : "wv",
        "plugins"   : ["wavpackenc"],
        "desc"      : _("A very fast Free lossless audio format with "
                        "good compression."),
        },
    }

# NOTE: the transcoder is NOT designed to transfer tags. You will need to
# manually write the tags after transcoding has completed.

def get_formats():
    ret = {}
    for name, value in FORMATS.iteritems():
        try:
            for plug in value["plugins"]:
                x = gst.element_factory_find(plug)
                if not x:
                    raise TranscodeError("don't support this format")
            ret[name] = value
        except:    
            pass
    return ret    
        
class TranscodeError(Exception):
    pass
                
class Transcoder(Logger):
    is_eos = False
    def __init__(self):
        self.src = None
        self.sink = None
        self.dest_format = "Ogg Vorbis"
        self.quality = 5
        self.input = None
        self.output = None
        self.encoder = None
        self.pipeline = None
        self.bus = None
        self.running = False
        self.is_pause = False
        self.__last_time = 0.0
        
        self.error_cb = None
        self.end_cb = None
        
    def set_format(self, name):    
        if name in FORMATS:
            self.dest_format = name
            
    def set_quality(self, value):        
        if value in FORMATS[self.dest_format]["raw_steps"]:
            self.quality = value
            
    def get_construct_encoder(self):        
        fmt = FORMATS[self.dest_format]
        quality = self.quality
        return fmt["command"] % quality
        
    def set_input(self, uri):    
        self.input = "filesrc location=\"%s\"" % uri
        
    def set_cd_input(self, uri):    
        device = uri[uri.find("#") + 1:]
        uri = uri[:uri.find("#")]
        track = uri[7:]
        self.input = "cdparanoiasrc device=\"%s\" track=%s" % (device, track)
        
    def set_raw_input(self, raw):    
        self.input = raw
        
    def set_output(self, uri):    
        self.output ="filesink location=\"%s\"" % uri
        
    def set_output_raw(self, raw):    
        self.output = raw
        
    def start_transcode(self):    
        elements = [ self.input, "decodebin name=decoder", "audioconvert",
                self.get_construct_encoder(), self.output ]
        pipestr = " ! ".join(elements)
        
        try:
            self.pipeline = gst.parse_launch(pipestr)
        except gobject.GError, e:
            if "faac" in e:
                e = e + "install gstreamer0.10-plugins-bad-multiverse package"
            raise TranscodeError(e)
        
        self.encoder = self.pipeline.get_by_name("encoder")
        decoder = self.pipeline.get_by_name("decoder")
        decoder.connect("unknown-type", self.unknown_type)
        
        self.bus = self.pipeline.get_bus()        
        self.bus.connect('message::error', self.on_error)
        self.bus.connect('message::eos', self.on_eof)
        self.bus.add_signal_watch()        
        state_ret = self.pipeline.set_state(gst.STATE_PLAYING)
        self.running = True

        timeout = 10
        while state_ret == gst.STATE_CHANGE_ASYNC and not self.is_eos and timeout > 0:
            state_ret, __state, __pending_state = self.pipeline.get_state(1 * gst.SECOND);
            timeout -= 1
            
        if state_ret != gst.STATE_CHANGE_SUCCESS:    
            raise TranscodeError("Failed change to playing")
        else:    
            self.loginfo("starting encoding")
        self.duration = self.get_duration()    
        
    def stop(self):
        state_ret = self.pipeline.set_state(gst.STATE_NULL)
        if state_ret != gst.STATE_CHANGE_SUCCESS:
            self.logwarn("failed stop pipeline")
            
        self.running = False
        self.is_eos = True
        try:
            self.end_cb()
        except:    
            pass
        
    def unknown_type(self, *param):
        self.logwarn("input file type unknown")
        raise TranscodeError("input file type unknown")
        
    def on_error(self, *args):    
        self.pipeline.set_state(gst.STATE_NULL)
        self.running = False
        try:
            self.error_cb()
        except:
            raise TranscodeError(args)
        
    def on_eof(self, *args):
        self.stop()

    def get_position(self):
        if gst.STATE_NULL != self.get_state():
            """ Use a encoder instead of pipeline because filesink and gnomevfs sink return wrong value in gstreamer 0.10.5"""
            try: p = self.encoder.query_position(gst.FORMAT_TIME)[0]
            except gst.QueryError: p = 0
            p //= gst.MSECOND
            return p
        return 0
        
    def get_state(self):
        __state_ret, state, __pending_state = self.pipeline.get_state();
        return state
    
    def set_state(self, status):
        if self.pipeline:
            self.pipeline.set_state(status)
            
    def pause(self):        
        if self.pipeline:
            self.pipeline.set_state(gst.STATE_PAUSED)
            self.is_pause = True
            
    def playing(self):        
        if self.pipeline:
            self.pipeline.set_state(gst.STATE_PLAYING)
            self.is_pause = False
    
    def get_duration(self):
        if self.get_state() != gst.STATE_NULL:
            try:
                query = gst.query_new_duration(gst.FORMAT_TIME)
                encoder = self.pipeline.get_by_name("encoder")
                if encoder.query(query):
                    total = query.parse_duration()[1]
                else: return 0
            except gst.QueryError: total = 0
            total //= gst.MSECOND
            return total
        else:
            return 0
        
    def get_ratio(self):
        ratio = float(self.get_position()) / float(self.duration)
        return  ratio
