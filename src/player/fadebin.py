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

# * PlayerBin pipeline *
# 
#     Adder : 
#         adder 
#         
#     Filterbin :    
#         (sink pad ghosted) audioconvert (src pad ghosted)
#         
#     Outputbin :    
#         capsfilter ! identity(blocker element) ! audioconvert ! audioresample ! tee ! audioconvert ! volume !
#         filterbin ! audioconvert ! audioresample ! queue ! sink
#         
#     Silencebin :    
#         audiotestsrc ! audioconvert ! capsfilter (src pad ghosted)
#         
#     Final pipeline:     
#         Silencebin ! Adder ! Outputbin
#     
# * StreamBin *    
#     src [ ! queue(remote)] ! decoder ! audioconvert ! rgvolume ! audioresample ! capsfilter ! 
#     queue(preroll) ! volume(src pad ghosted)
#
     
# ************************************************************************
# Basic design (from rb code):
#
# we have a single output bin, beginning with an adder.
# connected to this are a number of stream bins, consisting of a
# source, decodebin, audio convert/resample, and a volume element used
# for fading in and out. (might be interesting to replace those with 
# high/low pass filter element?)
#
# stream bins only stay connected to the adder while actually playing.
# when not playing (prerolling or paused), the stream bin's source pad 
# is blocked so no data can flow.

# ************************************************************************
# streams go through a number of states.
# When a stream is created (in xfade_open()), it starts in PREROLLING
# state, from there:

# - xfade_open():      -> PREROLL_PLAY
# - preroll finishs:   -> WAITING 

# From WAITING:
# - xfade_play(), crossfade == 0, other stream playing:   -> WAITING_EOS
# - xfade_play(), crossfade >  0, other stream playing:   -> FADEING_IN (link to adder, unblock + fade out existing stream)
# - xfade_play(), crossfade <  0, other stream playing:   -> PLAYING (link to adder, unblock + stop existing stream)
# - xfade_play(),               existing stream paused:   -> PLAYING (link to adder, unblock + stop existing stream)
# - xfade_play(),              nothing already playing:   -> PLAYING (link to adder, unblock)


# From PREROLL_PLAY:  
# - preroll finishs, crossfade == 0, other stream playing: -> WAITING_EOS
# - preroll finishs, crossfade  > 0, other stream playing: -> FADING_IN (link to adder, unblock)
# - preroll finishs, crossfade  < 0, other stream playing: -> PLAYING (link to adder, unblock + stop existing stream)
# - preroll finishs,               existing stream paused: -> PLAYING (link to adder, unblock)
# - preroll finishs,              nothing already playing: -> PLAYING (link to adder, unblock)

# From WAITING_EOS:
# - EOS received for another stream: -> PLAYING (link to adder, unblock)

# From FADING_IN: 
# - fade in completes:               -> PLAYING
# - another stream starts fading in: -> FADING_OUT
# - xfade_pause():                   -> PAUSE (block, unlink from adder)
# - stopped for another stream:      -> PENDING_REMOVE
# - set_time():                      -> SEEKING (block, unlink)
# - reused for another stream:       -> REUSING (block, unlink)

# From PLAYING:
# - xfade_pause()                    -> FADING_OUT_PAUSE (short fade out)
# - EOS:                             -> PENDING_REMOVE
# - another stream starts fading in: -> FADING_OUT
# - stopped for another stream:      -> PENDING_REMOVE
# - set_time()                       -> SEEKING (block, unlink)
# - reused for another stream:       -> REUSING (block, unlink)

# From SEEKING:
# - xfade_pause()                       -> SEEKING_PAUSED
# - blocked perform seek, link, unblock -> PLAYING | FADING_IN

# From SEEKING_PAUSED:
# - blocked perform seek             -> PAUSED
# - xfade_play()                     -> SEEKING

# From PAUSED:
# - xfade_play()                     -> FADING_IN (link to adder, unblock short fade)
# - stopped for another stream:      -> PENDING_REMOVE
# - set_time()                       -> perform seek

# From FADING_OUT:
# - fade out finishs:                -> PENDING_REMOVE
# - EOS:                             -> PENDING_REMOVE
# - reused for another stream:       -> REUSING (block, unlink)

# From FADING_OUT_PAUSED: 
# - fade out finishs:                -> SEEKING_PAUSED (block, unlink)
# - EOS:                             -> PENDING_REMOVE
# - reused for another stream        -> REUSING (block, unlink)
# - set_time()                       -> SEEKING_PAUSED (block unlink)

# From PENDING_REMOVE:
# - set_time()                        -> SEEKING_EOS (block, seek)
# - reap_streams idle handler called: -> unlink from adder, stream destroyed

# From REUSING:
# - blocked:    emit reuse-stream     ->  PLAYING

# From SEEKING_EOS:
# - block completes(link, unblock)    -> PLAYING
# - xfade_pause()                     -> SEEKING_PAUSED

import os
import gobject
import gst
import time
from threading import Lock, Thread
from logger import Logger

MMS_STREAM_SCHEMES = [ "mms", "mmsh", "mmsu", "mmst" ]
HTTP_STREAM_SCHEME = "http"
RTSP_STREAM_SCHEMES = [ "rtspu", "rtspt", "rtsph", "rtsp" ]
CDDA_STREAM_SCHEME = "cdda"
FILE_STREAM_SCHEME = "file"
BAD_STREAM_SCHEMES = MMS_STREAM_SCHEMES + RTSP_STREAM_SCHEMES
NETWORK_SCHEMES = [HTTP_STREAM_SCHEME] + MMS_STREAM_SCHEMES + RTSP_STREAM_SCHEMES

EPSILON = 0.001
XFADE_TICK_HZ = 5
MAX_NETWORK_BUFFER_SIZE = 2048
PAUSE_FADE_LENGTH = gst.SECOND / 2

# stable states 
WAITING = 1
PLAYING = 2
PAUSED = 4

# transition states 
REUSING = 8
PREROLLING = 16
PREROLL_PLAY = 32
FADING_IN = 64
SEEKING = 128
SEEKING_PAUSED = 256
SEEKING_EOS = 512
WAITING_EOS = 1024
FADING_OUT = 2048
FADING_OUT_PAUSED = 4096
PENDING_REMOVE = 8192


class FailedBuildGstElement(Exception):
    pass

class PlayerBin(gobject.GObject, Logger):
    __gsignals__ = {
        "error" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_STRING,)),
        "eos" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_STRING,)),
        "tick" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_INT, gobject.TYPE_INT)),
        "tags-found" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_PYOBJECT,)),
        "spectrum-data-found" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_PYOBJECT,)),
        "playing-stream" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_STRING,)),
        "queue-running" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_STRING,)),
        "playing" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                ()),
        "buffering" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_INT,)),
        "prepare-source" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                ()),
        "can-reuse-stream" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
        "reuse-stream" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                ()),
        "filter-inserted" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_OBJECT, gobject.TYPE_OBJECT)),
        "filter-removed" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_OBJECT, gobject.TYPE_OBJECT)),
        "tee-inserted" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_OBJECT, gobject.TYPE_OBJECT)),
        "tee-removed" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                (gobject.TYPE_OBJECT, gobject.TYPE_OBJECT)),
        }
    def __init__(self):
        gobject.GObject.__init__(self)

        self.stream_list_lock = Lock()
        self.sink_lock = Lock()

        self.streams = []
        self.buffer_size = 64

        self.__curvolume = 1.0
        self.__stream_reap_id = None
        self.__stop_sink_id = None
        self.tick_timeout_id = None
        self.bus_watch_id = None
        self.linked_streams = 0

        caps = gst.caps_from_string("audio/x-raw-int, channels=2, rate=44100, width=16, depth=16")
        
        self.pipeline = gst.Pipeline("DMusicPlayer")
        self.add_bus_watch()
        
        try:
            self.__output = gst.Bin("output")
            self.__adder = gst.element_factory_make("adder", "outputadder")
            self.__blocker = gst.element_factory_make("identity", "blocker")
            self.__capsfilter = gst.element_factory_make ("capsfilter", "outputcapsfilter")

            audioconvert = gst.element_factory_make ("audioconvert", "outputconvert")
            audioresample = gst.element_factory_make ("audioresample", "outputresample")
            postaudioconvert = gst.element_factory_make ("audioconvert", "postoutputconvert")
            postaudioresample = gst.element_factory_make ("audioresample", "postoutputresample")
            self.__tee = gst.element_factory_make ("tee", "outputtee")
            queue = gst.element_factory_make ("queue")
            self.__volume = gst.element_factory_make ("volume", "outputvolume")
            self.__filterbin = gst.Bin("filterbin")
            
            customsink = os.environ.get("DMUSIC_GST_SINK", "gconfaudiosink")
            try: self.__sink = gst.element_factory_make (customsink)
            except :
                self.__sink = gst.element_factory_make ("autoaudiosink")

        except (gobject.GError, gst.PluginNotFoundError):
            raise FailedBuildGstElement("Player Bin")

        self.__volume.set_property("volume", self.__curvolume)

        try: self.__sink.set_property("profile", 1)
        except TypeError: pass
        self.__capsfilter.set_property("caps", caps)

        audioconvert2 = gst.element_factory_make("audioconvert")
        self.__filterbin.add(audioconvert2)
        pad = audioconvert2.get_pad("sink")
        self.__filterbin.add_pad(gst.GhostPad("sink", pad))
        pad = audioconvert2.get_pad("src")
        self.__filterbin.add_pad(gst.GhostPad("src", pad))

        queue.set_property("max-size-buffers", 10)
        
        # self.spectrum = gst.element_factory_make ("spectrum", "spectrum")

        self.__output.add(self.__capsfilter, audioconvert , audioresample , \
                self.__tee , self.__volume, self.__filterbin, postaudioconvert, postaudioresample, queue, self.__sink)
        gst.element_link_many(self.__capsfilter, audioconvert, audioresample, \
                self.__tee, self.__volume, self.__filterbin, postaudioconvert, postaudioresample, queue, self.__sink)


        
        filterpad = self.__capsfilter.get_pad("sink")
        outputghostpad = gst.GhostPad("sink", filterpad)
        self.__output.add_pad(outputghostpad)

        # create silence bin
        self.__silence = gst.Bin("silencebin")
        audiotestsrc = gst.element_factory_make("audiotestsrc", "silence")
        audiotestsrc.set_property("wave", 4)

        audioconvert = gst.element_factory_make("audioconvert", "silenceconvert")

        capsfilter = gst.element_factory_make("capsfilter", "silencecapsfilter")
        capsfilter.set_property("caps", caps)

        self.__silence.add(audiotestsrc, audioconvert, capsfilter)
        gst.element_link_many(audiotestsrc, audioconvert, capsfilter)

        filterpad = capsfilter.get_pad("src")
        ghostpad = gst.GhostPad("src", filterpad)
        self.__silence.add_pad(ghostpad)

        # assembe stuff:
        # - add everything to the pipeline
        # - link adder to output bin
        # - link silence bin to adder

        self.pipeline.add(self.__adder, self.__output, self.__silence)
        addersrcpad = self.__adder.get_pad("src")
        addersrcpad.link(outputghostpad)

        reqpad = self.__adder.get_request_pad("sink%d")
        ghostpad.link(reqpad)
        
        # set element need to be blocked to tee other element
        self.__blocker = self.__adder

        self.sink_state = "SINK_STOPPED"
    
    def emit_error(self, stream):
        self.logwarn("Error emit from stream %s %s", stream.cutted_uri, stream.error)
        self.emit("error", stream.uri)

    def add_bus_watch(self):
        bus = self.pipeline.get_bus()
        self.bus_watch_id = bus.add_watch(self.xfade_bus_cb)

    def get_adder_pad(self):
        return self.__adder.get_request_pad("sink%d")
    
    # caller must hold stream lock
    def find_stream_by_uri(self, uri):
        if not uri: return None
        for stream in self.streams:
            if stream.uri == uri: return stream
        return None

    # caller must hold stream lock
    def find_stream_by_state(self, state_mask):
        for stream in self.streams:
            if (stream.state & state_mask) != 0:
                return stream
        return None

    # caller must hold stream lock
    def find_stream_by_element(self, element):
        for stream in self.streams:
            e = element
            while e:
                if e == stream: return stream
                e = e.get_parent()
        return None
    
    
    def dispose_streams(self):
        self.stream_list_lock.acquire()
        streams = self.streams[:]
        self.stream_list_lock.release()
        for stream in streams:
            stream.unlink_and_dispose_stream()
        

    def xfade_open(self, uri):
        
        buffer_streams = filter(lambda item: item.is_in_buffer(), self.streams)
        if len(buffer_streams) > 0:
            self.dispose_streams()
        
        try: 
            stream = StreamBin(self, uri)
        except Exception:
            self.logexception("Failed to create pipeline for %s", uri)
            return False
        
        self.stream_list_lock.acquire()
        self.streams.insert(0, stream)
        self.dump_stream_list()
        self.stream_list_lock.release()

        if not stream.preroll_stream():
            self.logwarn("Unable to preroll pipeline for %s", uri)
            return False

        return True

    def xfade_close(self, uri=None):
        ret = True
        uri = None
        if not uri:
            self.stream_list_lock.acquire()
            streams = self.streams[:]
            self.stream_list_lock.release()
            for stream in streams:
                self.logdebug("close stream %s", stream.cutted_uri)
                stream.unlink_and_dispose_stream()
        else:
            self.stream_list_lock.acquire()
            stream = self.find_stream_by_uri(uri)
            self.stream_list_lock.release()
            
            if stream: 
                self.logdebug("close stream %s", stream.cutted_uri)
                stream.unlink_and_dispose_stream()
            else: 
                self.logdebug("can't find stream for %s", uri)
                ret = False

        return ret

    def xfade_opened(self):
        opened = False
        self.stream_list_lock.acquire()
        stream = self.find_stream_by_state(PREROLLING | PREROLL_PLAY | WAITING_EOS | WAITING | FADING_IN | PLAYING | PAUSED)
        if stream:
            opened = True
        self.stream_list_lock.release()
        return opened

    def xfade_play(self, crossfade):
        ret = True

        self.stream_list_lock.acquire()
        if not self.streams:
            self.stream_list_lock.release()
            return False
            
        stream = self.streams[0]
        self.stream_list_lock.release()

        if not self.start_sink(): return False
        
        stream.lock.acquire()
        self.logdebug("changing stream state of %s, crossfade %d, current state %s", stream.cutted_uri, crossfade, stream.get_str_state())
        if stream.state in [ PREROLLING, PREROLL_PLAY ]:
            self.logdebug ("stream %s is prerolling; will start playback once prerolling is complete -> PREROLL_PLAY", stream.uri)
            stream.crossfade = crossfade
            stream.state = PREROLL_PLAY
        elif stream.state in [ SEEKING_PAUSED ]:
            self.logdebug ("unpausing seeking stream %s", stream.uri)
            stream.state = SEEKING
        elif stream.state in [ REUSING ]:
            self.logdebug ("currently reusing stream %s; will play when done", stream.uri); 
            pass
        elif stream.state in [ PENDING_REMOVE ]:
            self.logdebug("hmm, can't play streams in PENDING_REMOVE state..")

        stream_state = stream.state
        stream.lock.release()

        if stream_state in [ FADING_IN, FADING_OUT, FADING_OUT_PAUSED, PLAYING, SEEKING, SEEKING_EOS]:
            self.emit("playing")
        elif stream_state in [ PAUSED ]:
            stream.start_fade(0.0 , 1.0 , PAUSE_FADE_LENGTH)
            ret = stream.link_and_unblock_stream()
        elif stream_state in [ WAITING_EOS, WAITING ]:
            stream.crossfade = crossfade
            ret = stream.actually_start_stream()

        self.logdebug("changing stream state of %s, new crossfade %d, new state %s", stream.cutted_uri, stream.crossfade, stream.get_str_state())

        return ret

    def xfade_pause(self):
        self.logdebug("xfade pause")
        done = False
        to_fade = []
        fade_out_start = 1.0
        fade_out_time = PAUSE_FADE_LENGTH

        self.stream_list_lock.acquire()
        for stream in self.streams:
            if stream.state in [ WAITING, WAITING_EOS]:
                self.logdebug("stream %s is not yet playing, can't pause", stream.cutted_uri)
                pass
            elif stream.state in [ PREROLLING, PREROLL_PLAY ]:
                self.logdebug("stream %s is prerolling, can't pause", stream.cutted_uri)
                pass
            elif stream.state in [ PAUSED, SEEKING_PAUSED, FADING_OUT_PAUSED ]:
                self.logdebug("stream %s is already paused", stream.cutted_uri)
                done = True
            elif stream.state in [ PLAYING, FADING_IN ]:
                self.logdebug("stream %s is playing or fadein change to fadeout", stream.cutted_uri)
                to_fade.insert(0, stream)
                done = True
            elif stream.state in [ SEEKING ]:
                self.logdebug("stream %s is seeking change to paused", stream.cutted_uri)
                stream.state = SEEKING_PAUSED
                done = True
            elif stream.state in [ SEEKING_EOS ]:
                self.logdebug("stream %s is seeking after EOS -> SEEKING_PAUSED", stream.cutted_uri)
                stream.state = SEEKING_PAUSED
                done = True
            elif stream.state in [ FADING_OUT ]:
                self.logdebug("stream %s is fading out, can't be bothered pausing it", stream.cutted_uri)
                pass
            elif stream.state in [ PENDING_REMOVE ]:
                self.logwarn("stream %s is done, can't pause", stream.cutted_uri)
                pass
            
            if done: break

        self.stream_list_lock.release()

        for stream in to_fade:
            if stream.state in [ FADING_IN, PLAYING ]:
                if stream.state == FADING_IN:
                    fade_out_start = float(stream.get_volume())
                    fade_out_time = (PAUSE_FADE_LENGTH * fade_out_start)
                stream.state = FADING_OUT_PAUSED
                stream.start_fade(fade_out_start , 0.0, fade_out_time)

        if not done: 
            self.logwarn("couldn't find a stream to pause")

    def __xfade_in_transition(self):
        playing = False
        transition = False
        self.stream_list_lock.acquire()
        for stream in self.streams:
            if stream.state in [ PLAYING, FADING_IN, FADING_OUT, SEEKING, \
                    SEEKING_PAUSED, REUSING ]:
                if playing:
                    transition = True
                else:
                    playing = True

        self.stream_list_lock.release()
        return transition

    def xfade_playing(self):
        playing = False
        if self.sink_state != "SINK_PLAYING": return False
        self.stream_list_lock.acquire()
        stream = self.find_stream_by_state(PLAYING | FADING_IN)
        if stream:
            playing = True
        self.stream_list_lock.release()
        return playing
    
    def set_volume(self, volume):
        if self.__volume:
            self.__volume.set_property("volume", volume)
        self.__curvolume = volume

    def get_volume(self):
        return self.__curvolume

    def xfade_seekable(self):
        can_seek = True
        self.stream_list_lock.acquire()
        stream = self.find_stream_by_state(FADING_IN | PAUSED | PLAYING)
        self.stream_list_lock.release()
        if stream:
            query = gst.query_new_seeking (gst.FORMAT_TIME)
            try: res_query = stream.query_stream(query)
            except gst.QueryError:
                res_query = False
            if res_query:
                query.parse_seeking()#False,True, False, False)
            else:
                query = gst.query_new_duration (gst.FORMAT_TIME)
                try: can_seek = stream.query_stream(query)
                except gst.QueryError:
                    can_seek = False
        return can_seek

    def xfade_set_time(self, time):

        self.stream_list_lock.acquire()
        stream = self.find_stream_by_state(FADING_IN | PLAYING | PAUSED | FADING_OUT_PAUSED | PENDING_REMOVE)
        self.stream_list_lock.release()

        if not stream: 
            self.logdebug("got seek while no playing streams exist") 
            return

        stream.seek_target = time * gst.SECOND
        
        if stream.state == PAUSED:
            self.logdebug("seeking in paused stream %s; target %f", stream.cutted_uri , stream.seek_target)
            stream.perform_seek()
        elif stream.state == FADING_OUT_PAUSED:
            self.logdebug("seeking in paused stream %s; target %f", stream.cutted_uri , stream.seek_target)
            stream.state = SEEKING_PAUSED
            stream.unlink_and_block_stream()
        elif stream.state in [ FADING_IN, PLAYING ]:
            stream.state = SEEKING
            self.logdebug("unlinking playing stream %s to seek to %f, current state %s", stream.cutted_uri , stream.seek_target, stream.get_str_state())
            stream.perform_seek()
        elif stream.state == PENDING_REMOVE:
            self.logdebug("seeking in EOS stream %s; target %f" , stream.cutted_uri , stream.seek_target)
            stream.state = SEEKING_EOS
            stream.post_eos_block_src_pad()
            stream.perform_seek()
        else:
            self.logerror("xfade_set_time: invalid stream state")

    def xfade_get_time(self):
        ret = self.get_times_and_stream()
        if ret:
            _stream, pos, _duration = ret
        else:
            pos = 0.0
        return pos
    
    def xfade_get_duration(self):
        ret = self.get_times_and_stream()
        if ret:
            _stream, _pos, duration = ret
        else:
            duration = 0.0
        return duration

    def xfade_get_current_uri(self):
        ret = self.get_times_and_stream()
        if ret:
            stream, _pos, _duration = ret
            return stream.uri
        else:
            return None

    def stop_sink_later(self):

        self.sink_lock.acquire()
        self.__stop_sink_id = None
        if self.linked_streams == 0:
            self.stop_sink()
        self.sink_lock.release()
        return False
    
    def maybe_stop_sink(self):
        if os.environ.get('DMUSIC_NO_PIPELINE_STOP', False):
            self.logdebug("DMUSIC_NO_PIPELINE_STOP enable : don't really stop the pipeline")
            return
        self.sink_lock.acquire()
        if not self.__stop_sink_id :
            self.__stop_sink_id = gobject.timeout_add(1000, self.stop_sink_later)
        self.sink_lock.release()

    def reap_streams(self):
        self.logdebug("reap_streams")
        self.stream_list_lock.acquire()
        self.__stream_reap_id = None
        self.dump_stream_list()
        reaps = [ stream for stream in self.streams if stream.state == PENDING_REMOVE ]
        self.stream_list_lock.release()

        for reap in reaps:
            self.logdebug ("reaping stream %s", reap.cutted_uri)
            reap.unlink_and_dispose_stream()

        return False

    def schedule_stream_reap(self):
        self.stream_list_lock.acquire()
        if not self.__stream_reap_id :
            self.dump_stream_list()
            self.__stream_reap_id = gobject.idle_add(self.reap_streams)
        self.stream_list_lock.release()

    def xfade_bus_cb(self, bus, message):
        self.stream_list_lock.acquire()
        stream = self.find_stream_by_element(message.src)
        self.stream_list_lock.release()
        if message.type == gst.MESSAGE_ERROR:
            if stream:
                error = message.parse_error()[0]
                if not stream.emitted_error and error.domain == gst.STREAM_ERROR and error.code == gst.STREAM_ERROR_FAILED:
                    self.logerror("From stream: %s (%s)", error, message.parse_error()[1].replace("\n", " \ "))
                stream.emitted_error = True
                stream.emit_stream_error(message.parse_error())
        elif message.type == gst.MESSAGE_TAG:
            #self.logdebug("Tag Found")
            self.emit("tags-found", message.parse_tag())
        elif message.type == gst.MESSAGE_DURATION:
            self.logdebug("duration: %s", message.parse_duration())
        elif message.type == gst.MESSAGE_APPLICATION:
            if stream :
                structure = message.structure
                name = structure.get_name()
                if name == "STREAM_PLAYING_MESSAGE":
                    self.logdebug("got stream playing message for %s", stream.cutted_uri)
                    self.emit("playing-stream", stream.uri)
                    
                elif name == "STREAM_QUEUE_RUNNING":
                    self.emit("queue-running", stream.uri)
                elif name == "FADE_IN_DONE_MESSAGE":
                    pass
                elif name == "FADE_OUT_DONE_MESSAGE":
                    if stream.state == FADING_OUT:
                        stream.state = PENDING_REMOVE
                        self.logdebug("got fade-out-done for stream %s -> PENDING_REMOVE", stream.cutted_uri)
                        self.schedule_stream_reap()
                    elif stream.state == FADING_OUT_PAUSED:
                        self.logdebug("fade out paused  done")
                        format = gst.FORMAT_TIME
                        try:
                            pos, format = stream.query_stream_position(format)
                        except gst.QueryError:
                            pos = -1
                        if pos < 0:
                            stream.state = PAUSED
                            self.logdebug("got fade-out-done for stream %s -> PAUSED (position query failed)", stream.cutted_uri)
                        elif pos > PAUSE_FADE_LENGTH:
                            stream.seek_target = pos > PAUSE_FADE_LENGTH and pos - PAUSE_FADE_LENGTH or 0
                            stream.state = SEEKING_PAUSED
                            self.logdebug("got fade-out-done for stream %s -> SEEKING_PAUSED [%d]", stream.cutted_uri, stream.seek_target)

                        stream.unlink_and_block_stream()
                    else:
                        self.logerror("STATE not reached for %s, state: %s", stream.cutted_uri, stream.get_str_state())

                elif name == "STREAM_EOS_MESSAGE":
                    self.logdebug("got EOS message for stream %s -> PENDING_REMOVE", stream.cutted_uri)
                    stream.needs_unlink = True
                    self.emit("eos", stream.uri)
                    stream.state = PENDING_REMOVE
                    stream.unlink_blocked_cb(True)
                else:
                    self.logdebug("got not managed message (%s) for stream %s", name, stream.cutted_uri)

        elif message.type == gst.MESSAGE_BUFFERING:
            s = message.structure
            progress = s["buffer-percent"]
            if not stream:
                self.logdebug ("got buffering message for unknown stream (%d)", progress)
            else:
                self.emit("buffering", progress)
        elif message.type == gst.MESSAGE_ELEMENT:
            # FOR imperfect stream messages not implemented in python version
            s = message.structure
            name = s.get_name()
            
            if name == "spectrum":
                waittime = 0
                if s.has_key("running_time") and s.has_key("duration"):
                    timestamp = s["running_time"]
                    duration = s["duration"]
                    waittime = timestamp + duration / 2
                elif s.has_key("endtime"):    
                    waittime = s["endtime"]
                    
                if waittime:    
                    magnitude_list = s['magnitude']
                    # basetime = self.spectrum.get_base_time()                    
                    # clock_id = self.sync_clock.new_single_shot_id(waittime + basetime)    
                    # self.delayed_spectrum_update(spect)
                    self.emit("spectrum-data-found", magnitude_list)

        elif message.type == gst.MESSAGE_STATE_CHANGED:
            pass
        else:
            #print message.type, message
            pass

        # FIXME: missing in python need code it...
        #bus.async_signal_func(message)
        

        return True

    def get_times_and_stream(self):
        pos = 0.0
        duration = 0.0
        buffering = False

        if not self.pipeline:
            return None

        self.stream_list_lock.acquire()
        stream = self.find_stream_by_state(PREROLLING | PREROLL_PLAY)
        if stream:
            if not stream.emitted_fake_playing:
                stream = None
            else:
                self.logdebug ("found buffering stream %s as current", stream.cutted_uri)
                buffering = True
                self.dump_stream_list()
        if not stream:
            stream = self.find_stream_by_state(FADING_IN | PLAYING | FADING_OUT_PAUSED | PAUSED | PENDING_REMOVE)

        self.stream_list_lock.release()
        if stream:
            if buffering:
                pos = 0.0
            elif stream.state == PAUSED:
                try: 
                    res = stream.query_stream_position(gst.FORMAT_TIME)
                except gst.QueryError:
                    self.logerror(" failed query position from stream")
                    res = None
                if res:
                    pos, format = res
                    pos /= gst.MSECOND
            else:
                try: 
                    res = self.pipeline.query_position(gst.FORMAT_TIME)
                except gst.QueryError:
                    self.logerror(" failed query position from pipeline")
                    res = None
                if res:
                    pos, format = res
                    pos -= stream.base_time
                    pos /= gst.MSECOND

            try: 
                res = stream.query_stream_duration(gst.FORMAT_TIME)
            except gst.QueryError: 
                res = None
            if res:
                duration, format = res
                duration /= gst.MSECOND
        else:
            return None

        return stream, pos, duration

    def tick_timeout(self):
        res = self.get_times_and_stream()
        if res:
            _stream, pos, duration = res
            self.emit("tick", pos, duration)
        return True
    
    def start_sink_locked(self):
        messages = []
        self.logdebug("start_sink %s", self.sink_state)

        st = self.__output.set_state(gst.STATE_PAUSED)
        if st == gst.STATE_CHANGE_FAILURE:
            self.logerror("Failed start output")
            return (False, messages)
        
        st = self.__adder.set_state(gst.STATE_PAUSED)
        if st == gst.STATE_CHANGE_FAILURE:
            self.logerror("Failed start adder")
            return (False, messages)

        st = self.__silence.set_state(gst.STATE_PAUSED)
        if st == gst.STATE_CHANGE_FAILURE:
            self.logerror("Failed start silence")
            return (False, messages)

        waiting = True
        bus = self.pipeline.get_bus()
        while waiting:
            message = bus.timed_pop(5 * gst.SECOND)
            if not message :
                self.logdebug("sink is taking too long to start..")
                del bus
                return (False, messages)
            if message.type == gst.MESSAGE_ERROR:
                stream = self.find_stream_by_element(message.src)
                if not stream:
                    self.logerror("got an error from a stream; passing it to the bus handler")
                    messages.append(message)
                else:
                    self.logerror("got error from bus %s:", message.parse_error())
                    self.__output.set_state(gst.STATE_NULL)
                    self.__adder.set_state(gst.STATE_NULL)
                    self.__silence.set_state(gst.STATE_NULL)
                    return (False, messages)
            elif message.type == gst.MESSAGE_STATE_CHANGED:
                _oldstate, newstate, pending = message.parse_state_changed()
                if newstate == gst.STATE_PAUSED and pending == gst.STATE_VOID_PENDING:
                    if message.src == self.__output:
                        self.logdebug("output is PAUSED")
                        waiting = False
                    elif message.src == self.__silence:
                        self.logdebug("silence is PAUSED")
                    elif message.src == self.__adder:
                        self.logdebug("adder is PAUSED")
            else:
                messages.append(message)
            
        del bus

        st = self.__silence.set_state(gst.STATE_PLAYING)
        if st == gst.STATE_CHANGE_FAILURE:
            self.logerror("Failed playing silence")
            return (False, messages)

        st = self.__adder.set_state(gst.STATE_PLAYING)
        if st == gst.STATE_CHANGE_FAILURE:
            self.logerror("Failed playing adder")
            return (False, messages)

        st = self.__output.set_state(gst.STATE_PLAYING)
        if st == gst.STATE_CHANGE_FAILURE:
            self.logerror("Failed playing output")
            return (False, messages)
        
        self.logdebug("sink playing")

        self.sink_state = "SINK_PLAYING"
        
        if not self.tick_timeout_id:
            ms_period = 1000 / XFADE_TICK_HZ
            self.tick_timeout_id = gobject.timeout_add(ms_period, self.tick_timeout)

        self.logdebug("state: %s", self.sink_state)
        return (True, messages)


    def start_sink(self):
        ret = False
        messages = []
        self.sink_lock.acquire()
        if self.sink_state == "SINK_NULL":
            self.logerror("Huuu start a NULL state SINK !!! ")
        elif self.sink_state == "SINK_STOPPED":
            gobject.source_remove(self.bus_watch_id)
            ret, messages = self.start_sink_locked()
            self.add_bus_watch()
        elif self.sink_state == "SINK_PLAYING":
            ret = True

        self.sink_lock.release()

        bus = self.pipeline.get_bus()
        if messages:
            self.logdebug("passing messages to bus callback %s", ",".join([ "%s" % message.type for message in messages ]))
        for message in messages:
            self.xfade_bus_cb(bus, message)
        return ret


    def stop_sink(self):

        if self.sink_state in [ "SINK_PLAYING" ]:
            self.logdebug("stopping sink")

            if self.tick_timeout_id:
                gobject.source_remove(self.tick_timeout_id)
                self.tick_timeout_id = None

            sr = self.__output.set_state(gst.STATE_READY)
            if sr == gst.STATE_CHANGE_FAILURE:
                self.logerror("couldn't stop output bin")
                return False

            sr = self.__adder.set_state(gst.STATE_READY)
            if sr == gst.STATE_CHANGE_FAILURE:
                self.logerror("couldn't stop silence bin")
                return False

            sr = self.__silence.set_state(gst.STATE_READY)
            if sr == gst.STATE_CHANGE_FAILURE:
                self.logerror("couldn't stop silence bin")
                return False


            # try stopping the sink, but don't worry if we can't 
            sr = self.__sink.set_state(gst.STATE_NULL)
            if sr == gst.STATE_CHANGE_FAILURE:
                self.logerror("couldn't set audio sink to NULL state")
                #return False

            self.sink_state = "SINK_STOPPED"

        elif self.sink_state in [ "SINK_STOPPED", "SINK_NULL" ]:
            pass
        
        return True

    def dump_state(self):
        self.logdebug("Player state: %s", self.sink_state)
        #self.dump_stream_list_lock()
        self.dump_elements_state()
        return "Read dump in deepin-music console"

    def dump_stream_list_lock(self):
        self.stream_list_lock.acquire()
        self.dump_stream_list()
        self.stream_list_lock.release()

    def dump_stream_list(self):
        self.logdebug("")
        if not self.streams:
            self.logdebug("No streams")
        else:
            self.logdebug("Stream list:")
            for stream in self.streams:
                statename = stream.get_str_state()
                self.logdebug("  - [%s] %s", statename, stream.cutted_uri)
    
    def dump_elementbin_state(self, bin):
        elems = bin.elements()
        while True:
            try:
                elem = elems.next()
                self.logdebug(" - %s : %s", elem.get_name(), elem.get_state(gst.SECOND)[1])
            except StopIteration:
                break

    def dump_elements_state(self):
        # TODO: check self.__tee
        for bin in [ self.pipeline, self.__filterbin, self.__output ]: 
            self.logdebug("Subelements of %s:", bin.get_name())
            self.dump_elementbin_state(bin)
        pads = self.__tee.sink_pads()
        try: pad = pads.next()
        except StopIteration:
            while True:
                try:
                    pad = pads.next()
                    pad = pad.get_peer()
                    elem = pad.get_parent_element()
                    self.dump_elementbin_state(elem)
                except StopIteration:
                    break

        for stream in self.streams:
            statename = stream.get_str_state()
            self.logdebug("Subelements of  [%s] %s:", statename, stream.cutted_uri)
            decoder_linked, player_linked = stream.get_linkage()
            self.logdebug("* player linked: %s", player_linked)
            self.logdebug("* decoder linked: %s", decoder_linked)
            self.logdebug("* blocked: %s", stream.src_blocked)
            self.logdebug("* pas is blocked: %s", stream.is_blocked())
            elems = stream.elements()
            while True:
                try:
                    elem = elems.next()
                    self.logdebug(" - %s : %s", elem.get_name(), elem.get_state()[1])
                except StopIteration:
                    break

        if not self.streams:
            self.logdebug("No streams")



    def xfade_add_tee(self, element):
        return self.__pipeline_op(element, self.__blocker, self.__really_add_tee)

    def xfade_remove_tee(self, element):
        return self.__pipeline_op(element, self.__blocker, self.__really_remove_tee)

    def __really_add_tee(self, pad, blocked, element):
        bin = gst.Bin()
        queue = gst.element_factory_make("queue")
        audioconvert = gst.element_factory_make("audioconvert")

        queue.set_property("max-size-buffers", 3)
        bin.add(queue, audioconvert, element)
        gst.element_link_many(queue, audioconvert, element)

        sinkpad = queue.get_pad("sink")
        ghostpad = gst.GhostPad("sink", sinkpad)
        bin.add_pad(ghostpad)

        # The bin contains elements that change state asynchronously
        # and not as part of a state change in the entire pipeline.
        # With GStreamer core 0.10.13+, we need to ask the bin to
        # handle this specifically using its 'async-handling' property.
        try: bin.set_property("async-handling", True)
        except: pass
        self.__output.add(bin)
        self.__tee.link(bin)
        if blocked:
            bin.set_state(gst.STATE_PLAYING)
            self.__output.set_state(gst.STATE_PLAYING)
            self.logdebug("tee_pad -> unblock stream cb:pipeline_op_done state:%s", self.sink_state)
            pad.set_blocked_async(False, self.__pipeline_op_done, ghostpad, "tee-inserted", self.__tee, element)
        else:
            bin.set_state(gst.STATE_PAUSED)
            self.__pipeline_op_done(None, False, ghostpad, "tee-inserted", self.__tee, element)
    
    def __really_remove_tee(self, pad, blocked, element):
        bin = element.get_parent()
        self.__tee.unlink(bin)
        self.__output.remove(bin)
        bin.set_state(gst.STATE_NULL)
        bin.remove(element)

        if blocked:
            self.logdebug("remove_tee -> unblock stream cb:pipeline_op_done state:%s", self.sink_state)
            pad.set_blocked_async(False, self.__pipeline_op_done, None, "tee-removed", self.__tee, element)
        else:
            self.__pipeline_op_done(None, False, None, "tee-removed", self.__tee, element)

    def __pipeline_op_done(self, pad, blocked, new_pad=None, signal=None, *args):
        if new_pad != None:
            segment = gst.event_new_new_segment(True, 1.0, gst.FORMAT_DEFAULT, 0, gst.CLOCK_TIME_NONE / 2 , 0)
            new_pad.send_event(segment)

        if signal: self.emit(signal, *args)
        
    def __pipeline_op(self, element, previous_element, callback):
        block_pad = previous_element.get_pad("src")
        if self.sink_state == "SINK_PLAYING":
            self.logdebug("pipeline_op -> block stream callback:%s state:%s", callback, self.sink_state)
            can_blocked = block_pad.set_blocked_async(True, callback, element)
            if not can_blocked: self.logwarn("pad already blocked")
        else:
            callback(block_pad, False, element)
        return True

    def __really_add_filter(self, pad, blocked, element):
        
        sink_pad_found = False
        stop_scan = False
        sink_pads = element.sink_pads()
        while not sink_pad_found and not stop_scan:
            try:
                element_sink_pad = sink_pads.next()
                sink_pad_found = not element_sink_pad.is_linked()
            #except Resync:
            #    sink_pads.resync()
            except StopIteration:
                stop_scan = True
            #except:
            #    stop_scan = True
        
        if not sink_pad_found:
            self.logwarn("Could not find a free sink pad on filter")
            return

        bin = gst.Bin()
        audioconvert = gst.element_factory_make("audioconvert")
        audioresample = gst.element_factory_make("audioresample")
        bin.add(element, audioconvert, audioresample)
        element.link(audioconvert)
        audioconvert.link(audioresample)

        binsinkpad = gst.GhostPad("sink", element_sink_pad)
        bin.add_pad(binsinkpad)

        realpad = audioresample.get_pad("src")
        binsrcpad = gst.GhostPad("src", realpad)
        bin.add_pad(binsrcpad)

        self.__filterbin.add(bin)

        ghostpad = self.__filterbin.get_pad("src")
        realpad = ghostpad.get_target()
        ghostpad.set_target(binsrcpad)

        try:
            realpad.link(binsinkpad)
        except:
            self.logwarn("could not link new filter into pipeline")

        if blocked:
            bin.set_state(gst.STATE_PLAYING)
            self.logdebug("filter_pad -> unblock stream cb:pipeline_op_done state:%s", self.sink_state)
            pad.set_blocked_async(False, self.__pipeline_op_done, None, "filter-inserted", self.__filterbin, element)
        else:
            bin.set_state(gst.STATE_PAUSED)
            self.__pipeline_op_done(None, False, None, "filter-inserted", self.__filterbin, element)

    def __really_remove_filter(self, pad, blocked, element):
        bin = element.get_parent()
        if not bin: return

        bin.set_state(gst.STATE_NULL)

        mypad = bin.get_pad("sink")
        prevpad = mypad.get_peer()
        prevpad.unlink(mypad)

        ghostpad = bin.get_pad("src")
        nextpad = self.__filterbin.get_pad("src")

        targetpad = nextpad.get_parent()
        if targetpad == ghostpad:
            nextpad.set_target(prevpad)
        else:
            nextpad = ghostpad.get_peer()
            ghostpad.unlink(nextpad)
            try:
                prevpad.link(nextpad)
            except :
                self.logwarn("Failed to relink the filter")
        
        self.__filterbin.remove(bin)

        if blocked:
            self.logdebug("remove_filter -> unblock stream cb:pipeline_op_done state:%s", self.sink_state)
            pad.set_blocked_async(False, self.__pipeline_op_done, None, "filter-removed", self.__filterbin, element)
        else:
            self.__pipeline_op_done(None, False, None, "filter-removed", self.__filterbin, element)
    
    def xfade_add_filter(self, element):
        return self.__pipeline_op(element, self.__filterbin, self.__really_add_filter)

    def xfade_remove_filter(self, element):
        return self.__pipeline_op(element, self.__filterbin, self.__really_remove_filter)

class StreamBin(gst.Bin, Logger):

    def __init__(self, player, uri):
        gst.Bin.__init__(self)       
        self.__error_id = None
        self.__error = None
        self.__queue = None
        self.__underrun_id = None
        self.__queue_threshold = None
        self.__queue_threshold_id = None
        self.__queue_probe_id = None
        self.__adjust_probe_id = None
        self.__decoder_pad = None

        self.stream_data = None
        self.stream_data_desktroy = None

        self.fading = False
        self.fade_end = 0
        self.emitted_error = False
        self.crossfade = 0

        self.replaygain_scale = 1.0

        self.needs_unlink = False

        self.lock = Lock()

        self.emitted_playing = False
        self.emitted_fake_playing = False
        self.base_time = 0.0
        self.src_blocked = False
        
        self.bad_stream_timeout = 2
        
        # get uri scheme
        self.uri_scheme = self.get_uri_scheme(uri)

        # Only keep it for debug
        self.uri = uri
        if len(uri) > 43:
            self.cutted_uri = '"' + uri[:15] + "..." + uri[-25:] + '"'
        else:
            self.cutted_uri = '"' + uri + '"'

        self.state = WAITING
        self.__player = player

        self.__adder_pad = None
        self.__decoder_linked = False

        try:
            if self.uri_scheme == CDDA_STREAM_SCHEME:
                # Set cddb stream params.
                device = uri[uri.find("#") + 1:]
                uri = uri[:uri.find("#")]
                self.__src = gst.element_make_from_uri("src", uri)
                self.__src.set_property("device", device)
            else:    
                # other stream.
                self.__src = gst.element_make_from_uri("src", uri)
            
            if self.uri_scheme == HTTP_STREAM_SCHEME:    
                try:
                    self.__src.set_property("iradio-mode", True)
                except:    
                    pass
            
            self.__audioconvert1 = gst.element_factory_make ("audioconvert", None)
            self.__replaygain_volume = gst.element_factory_make ("rgvolume", None)
            self.__replaygain_volume.set_property("album-mode", True)
            self.__audioconvert2 = gst.element_factory_make ("audioconvert", None)
            self.__audioresample = gst.element_factory_make ("audioresample", None)
            self.__capsfilter = gst.element_factory_make ("capsfilter", None)
            caps = gst.caps_from_string("audio/x-raw-int, channels=2, rate=44100, width=16, depth=16")

            try: self.__decoder = gst.element_factory_make ("decodebin2")
            except (gobject.GError, gst.PluginNotFoundError), err:
                self.__decoder = gst.element_factory_make ("decodebin")

            self.__volume = gst.element_factory_make ("volume", None)
            self.__preroll = gst.element_factory_make ("queue", None)
            self.__queue = gst.element_factory_make ("queue", None)

        except (gobject.GError, gst.PluginNotFoundError), err:
            raise FailedBuildGstElement(err)
       
        self.__decoder.connect("new-decoded-pad", self.__new_decoded_pad_cb)
        self.__decoder.connect("pad-removed", self.__pad_removed_cb)
        self.__capsfilter.set_property("caps" , caps)

        self.__volume_signal_id = self.__volume.connect("notify::volume", self.__volume_change_cb)
        self.__fader = gst.Controller(self.__volume, "volume")
        self.__fader.set_interpolation_mode("volume", gst.INTERPOLATE_LINEAR)

        self.__preroll.set_property("min-threshold-time", gst.SECOND)
        self.__preroll.set_property("max-size-buffers", 1000)

        if self.uri_scheme in NETWORK_SCHEMES:
            self.logdebug("Create a remote stream bin")
            self.__queue_threshold = self.__player.buffer_size * 1024;
            self.__queue.set_properties(
                    "min-threshold-bytes",
                    self.__queue_threshold,
                    "max-size-bytes",
                    MAX_NETWORK_BUFFER_SIZE * 2 * 1024,
                    "max-size-buffers", 0,
                    "max-size-time", long(0))

            self.__underrun_id = self.__queue.connect("underrun", self.__queue_underrun_cb)
            self.add(self.__src, self.__queue, self.__decoder, self.__audioconvert1, 
                     self.__replaygain_volume, self.__audioconvert2, self.__audioresample, 
                     self.__capsfilter, self.__preroll, self.__volume)
            
            if self.uri_scheme in RTSP_STREAM_SCHEMES:
                self.__src.connect("pad-added", self.__on_rtspsrc_pad_added)
                self.__src.connect("pad-removed", self.__on_rtspsrc_pad_removed)
                gst.element_link_many(self.__queue, self.__decoder)
                gst.element_link_many(self.__audioconvert1, self.__replaygain_volume, 
                                      self.__audioconvert2, self.__audioresample, self.__capsfilter,
                                      self.__preroll, self.__volume)
            
            else:    
                gst.element_link_many(self.__src, self.__queue, self.__decoder)
                gst.element_link_many(self.__audioconvert1, self.__replaygain_volume, 
                                      self.__audioconvert2, self.__audioresample, self.__capsfilter, 
                                      self.__preroll, self.__volume)
        else:
            self.logdebug("Create a local stream bin")
            self.__queue = None

            self.add(self.__src, self.__decoder, self.__audioconvert1, 
                     self.__replaygain_volume, self.__audioconvert2,
                     self.__audioresample, self.__capsfilter, self.__preroll, self.__volume)
            gst.element_link_many(self.__src, self.__decoder)
            gst.element_link_many(self.__audioconvert1, self.__replaygain_volume, 
                                  self.__audioconvert2, self.__audioresample,
                                  self.__capsfilter, self.__preroll, self.__volume)

        self.__src_pad = self.__volume.get_pad("src")
        self.__ghost_pad = gst.GhostPad("src", self.__src_pad)
        self.add_pad(self.__ghost_pad)

        self.__src_pad.add_event_probe(self.__src_event_cb)

        self.set_bus(self.__player.pipeline.get_bus())
        
    def __on_rtspsrc_pad_added(self, rtspsrc, pad):    
        pad.link(self.__queue.get_pad("sink"))
        
    def __on_rtspsrc_pad_removed(self, rtspsrc, pad):    
        pad.unlink(self.__queue.get_pad("sink"))
        
    def get_uri_scheme(self, uri):    
        colon_index = uri.find(":")
        if colon_index == -1:
            return ""
        else:
            return uri[:colon_index]

    def get_str_state(self):
        if self.state == WAITING:       statename = "waiting"
        elif self.state == PLAYING:       statename = "playing"
        elif self.state == PAUSED:        statename = "paused"

        elif self.state == REUSING:       statename = "reusing"
        elif self.state == PREROLLING:    statename = "prerolling"
        elif self.state == PREROLL_PLAY:  statename = "preroll->play"
        elif self.state == FADING_IN:     statename = "fading in"
        elif self.state == SEEKING:       statename = "seeking"
        elif self.state == SEEKING_PAUSED:    statename = "seeking->paused"
        elif self.state == SEEKING_EOS:    statename = "seeking post EOS"
        elif self.state == WAITING_EOS:   statename = "waiting for EOS"
        elif self.state == FADING_OUT:    statename = "fading out"
        elif self.state == FADING_OUT_PAUSED:    statename = "fading->paused"

        elif self.state == PENDING_REMOVE:    statename = "pending remove"
        else: statename = "Unknown!!"
        return statename

    def __emit_stream_error_cb(self):
        self.__player.emit_error(self)
        self.__error = None
        self.__error_id = None

    def emit_stream_error(self, error):
        if self.__error_id : 
            pass
        else:
            self.__error = error
            self.__error_id = gobject.idle_add(self.__emit_stream_error_cb)

    def __volume_change_cb(self, element, gspec):
        message = None
        self.lock.acquire()
        vol = self.__volume.get_property("volume")
        if self.state == FADING_IN:
            if vol >= (self.fade_end - EPSILON) and self.fading :
                #message = "FADE_IN_DONE_MESSAGE"
                self.fading = False
                self.state = PLAYING

        elif self.state in [ FADING_OUT, FADING_OUT_PAUSED ]:
            if vol < (self.fade_end + EPSILON):
                if self.fading:
                    self.logdebug("stream %s fully faded out (at %f)", self.cutted_uri, vol); 
                    message = "FADE_OUT_DONE_MESSAGE"
                    self.fading = False
            else:
                #self.logdebug("fading %s out:%f, %f",self.cutted_uri,( float(pos) / gst.SECOND ),vol)
                self.__volume.set_passthrough(False)

        self.lock.release()
        if message:
            st = gst.Structure(message)
            msg = gst.message_new_application(element, st)
            self.__volume.post_message(msg)

    def unlink_and_dispose_stream(self):

        was_linked = False

        sr = self.set_state(gst.STATE_NULL)
        if sr == gst.STATE_CHANGE_ASYNC:
            self.logerror("stream isn't cooperating %s", self.cutted_uri)
            self.get_state(gst.CLOCK_TIME_NONE)
    
        self.lock.acquire()

        if self.__adder_pad is not None:
            self.logdebug("unlink added in stream %s to dispose", self.cutted_uri)
            self.__ghost_pad.unlink(self.__adder_pad)
            self.__adder_pad.get_parent().release_request_pad(self.__adder_pad)
            self.__adder_pad = None
            was_linked = True

        was_in_pipeline = self.get_parent() == self.__player.pipeline

        self.lock.release()
        
        if was_in_pipeline:
            self.__player.pipeline.remove(self)

        if was_linked:
            #self.__player.sink_lock.acquire()
            self.__player.linked_streams -= 1
            self.logdebug("now have %d linked streams", self.__player.linked_streams)
            last = (self.__player.linked_streams == 0)
            #self.__player.sink_lock.release()

            if last:
                self.__player.maybe_stop_sink()

        try:        
        # self.__player.stream_list_lock.acquire()
            self.__player.streams.remove(self)
            self.__player.dump_stream_list()
        except:    
            pass
        
        # self.__player.stream_list_lock.release()

    def unlink_and_block_stream(self):
        if self.__adder_pad is None:
            self.logdebug("stream is not linked %s", self.cutted_uri)
            return

        if self.src_blocked: cb = "None"
        else: cb = "__unlink_blocked_cb"
        self.logdebug("unlink_and_block_stream -> block (%s) stream %s __src_pad cb:%s state:%s", self.src_blocked, self.cutted_uri, cb, self.get_str_state())

        self.needs_unlink = True
        if self.src_blocked:
            self.unlink_blocked_cb(True)
        else:
            self.__src_pad.set_blocked_async(True, self.__unlink_blocked_cb)

    def link_and_unblock_stream(self):

        if not self.__player.start_sink():
            self.logdebug("sink didn't start, so we're not going to link the stream")
            return False
        
        if self.__adder_pad is not None: 
            self.logdebug("stream %s is already linked", self.cutted_uri)
            return True
    
        self.needs_unlink = False
        self.logdebug("linking stream %s", self.cutted_uri)
        if not self.get_parent():
            self.__player.pipeline.add(self)
        
        self.__adder_pad = self.__player.get_adder_pad()
        if self.__adder_pad is None: 
            self.logerror("Failed get adder_pad")
            return False
        try:
            self.__ghost_pad.link(self.__adder_pad)
        except:
            try: self.__adder_pad.get_parent().release_request_pad(self.__adder_pad)
            except: pass
            self.logwarn("Failed to link stream to pipeline")
            self.__adder_pad = None
            return False

        self.__player.linked_streams += 1
        self.logdebug("now have %d linked streams", self.__player.linked_streams)

        if self.src_blocked:
            state = self.get_str_state()
            self.logdebug("link_and_unblock_stream -> stream %s __src_pad, cb:__link_unblocked_cb state:%s", self.cutted_uri, state)
            can_unblock = self.__src_pad.set_blocked_async(False, self.__link_unblocked_cb)

            self.logdebug("link_and_unblock_stream, can_unblock: %s", can_unblock)
            return True
        else:
            self.logdebug("??? stream %s is already unblocked -> PLAYING", self.cutted_uri);
            self.state = PLAYING
            self.__adjust_stream_base_time()
            scr = self.set_state(gst.STATE_PLAYING)
            self.post_stream_playing_message(False)
            if scr == gst.STATE_CHANGE_FAILURE:
                self.logerror("Failed to start stream of %s", self._uri)
                return False
            return True

    def __link_unblocked_cb(self, pad, blocked):
        self.logdebug("try link_unblocked stream %s -> %s", self.cutted_uri, self.get_str_state())

        self.lock.acquire()

        if self.state in [ FADING_IN, PLAYING ]:
            self.logdebug("stream %s is already in FADING_IN | PLAYING state", self.cutted_uri)
            self.lock.release()
            return

        self.logdebug("stream %s is unblocked -> FADING_IN | PLAYING", self.cutted_uri);

        self.src_blocked = False
        if self.fading:
            self.state = FADING_IN
        else:
            self.state = PLAYING
        self.lock.release()

        self.__adjust_stream_base_time()

        # Change Stream state to PLAYING
        sr = self.set_state(gst.STATE_PLAYING)
        self.logdebug("stream %s state change returned: %s", self.cutted_uri, sr)

        self.post_stream_playing_message(False)
    
    def unlink_blocked_cb(self, blocked):
        self.__unlink_blocked_cb(self.__src_pad, blocked)

    def __unlink_blocked_cb(self, pad, blocked):
        self.lock.acquire()

        if not self.needs_unlink or self.__adder_pad is None:
            self.logwarn("stream is already unlinked.  huh? %s", self.cutted_uri)
            self.lock.release()
            return 
        
        self.__ghost_pad.unlink(self.__adder_pad)

        self.needs_unlink = False

        self.__adder_pad.get_parent().release_request_pad(self.__adder_pad)
        self.__adder_pad = None

        self.src_blocked = True
        self.emitted_playing = False
        
        stream_state = self.state

        self.lock.release()

        self.__player.linked_streams -= 1
        last = (self.__player.linked_streams == 0)
        
        self.logdebug("%d linked streams left", self.__player.linked_streams)
        self.logdebug("unlink added in stream %s to block", self.cutted_uri)

        if stream_state == SEEKING_PAUSED:
            gobject.idle_add(self.perform_seek_idle)
        if last:
            self.__player.maybe_stop_sink()

        return True
        
    def __src_event_cb(self, pad , event):
        if event.type == gst.EVENT_EOS:
            s = gst.Structure("STREAM_EOS_MESSAGE")
            msg = gst.message_new_application(self, s)
            self.post_message(msg)

            self.logdebug("stream eos %s", self.cutted_uri)

            to_start = []
            self.__player.stream_list_lock.acquire()
            for pstream in self.__player.streams:
                if pstream.state == WAITING_EOS:
                    to_start.insert(0, pstream)
            self.__player.stream_list_lock.release()

            for stream in to_start:
                self.logdebug("starting stream %s on EOS from previous", stream.cutted_uri)
                if not stream.link_and_unblock_stream():
                    stream.emit_stream_error("link_and_unblock_stream")

        elif event.type == gst.EVENT_NEWSEGMENT:
            self.__adjust_stream_base_time ()
        elif event.type in [ gst.EVENT_FLUSH_STOP, gst.EVENT_FLUSH_START ]:
            return False
        elif event.type in [ gst.EVENT_LATENCY ]:
            # Pass this signal silency
            pass
        else:
            self.logdebug("event %s in stream %s", event.type, self.cutted_uri)

        return True

    def __adjust_base_time_probe_cb (self, pad, data):
        self.logdebug("attempting to adjust base time for stream %s", self.cutted_uri)
        self.__adjust_stream_base_time()
        return True

    def __adjust_stream_base_time(self):
        format = gst.FORMAT_TIME

        self.lock.acquire()
        if self.__adder_pad is None:
            self.logdebug("stream isn't linked, can't adjust base time")
            self.lock.release()
            return 
        try: 
            res = self.__adder_pad.get_parent().query_position(format)
        except gst.QueryError:
            res = None
        if res:
            output_pos, format = res 
            self.base_time = output_pos

        try:
            res = self.__volume.query_position(format)
        except gst.QueryError:
            res = None
        if res:
            stream_pos, format = res
            self.base_time -= stream_pos

            if self.__adjust_probe_id is not None:
                self.__ghost_pad.remove_buffer_probe(self.__adjust_probe_id)
                self.__adjust_probe_id = None
        else:
            if self.__adjust_probe_id is None:
                self.__adjust_probe_id = self.__ghost_pad.add_buffer_probe(self.__adjust_base_time_probe_cb)

        self.lock.release()

    def __queue_probe_cb(self, data, queue):
        level = self.__queue.get_property("current-level-bytes")
        if self.__queue_threshold > 0:
            progress = (level * 99) / self.__queue_threshold
            if progress > 99:
                progress = 99
        else:
            progress = 99
        self.logdebug ("%s: buffer level: %u; threshold %u - %u%%", self.cutted_uri, level, self.__queue_threshold, progress)
        self.post_buffering_message(progress)
        return True

    def __queue_threshold_cb(self, queue):
        self.logdebug("%s: queue running", self.cutted_uri)
        
        sinkpad = self.__queue.get_pad("sink")
        sinkpad.remove_buffer_probe(self.__queue_probe_id)
        self.__queue_probe_id = None

        self.__queue.set_property("min-threshold-bytes", 0)

        self.__queue.disconnect(self.__queue_threshold_id)
        self.__queue_threshold_id = None
        self.post_buffering_message(100)
        
        s = gst.Structure("STREAM_QUEUE_RUNNING")
        msg = gst.message_new_application(self, s)
        self.post_message(msg)
        
    def is_in_buffer(self):    
        return self.__queue_probe_id is not None

    def __queue_underrun_cb(self, queue):
        self.logdebug("%s: queue underrun", self.cutted_uri)
        self.__queue.set_property("min-threshold-bytes", self.__queue_threshold)
        
        if not self.__queue_probe_id:
            sinkpad = self.__queue.get_pad("sink")
            self.__queue_probe_id = sinkpad.add_buffer_probe(self.__queue_probe_cb)
            
        if not self.__queue_threshold_id:
            self.__queue_threshold_id = self.__queue.connect("running", self.__queue_threshold_cb)
            self.post_buffering_message(0)

    def post_buffering_message(self, level):
        if not self.emitted_fake_playing:
            self.emitted_fake_playing = True
            self.post_stream_playing_message(True)
        msg = gst.message_new_buffering(self.__queue, level)
        self.__queue.post_message(msg)

    def post_stream_playing_message(self, fake):
        if self.emitted_playing: return 
        s = gst.Structure("STREAM_PLAYING_MESSAGE")
        msg = gst.message_new_application(self, s)
        if not fake: 
            self.emitted_playing = True
        else: 
            self.logdebug("send a fake playing message")
        self.post_message(msg)

    def __new_decoded_pad_cb(self, decoder, pad, last):
        caps = pad.get_caps()
        if caps.is_empty() or caps.is_any():
            self.logwarn("get a empty/any decoded caps. hum?")
            return

        structure = caps[0]
        mediatype = structure.get_name()
        if not mediatype.startswith("audio/x-raw"):
            self.logwarn("caps is not a audio caps: %s", mediatype)
            return
        elif self.__decoder_linked:
            self.logwarn("Decoder is already linked ?!!")
        else:
            vpad = self.__audioconvert1.get_pad("sink")
            pad.link(vpad)
            self.__decoder_linked = True
            self.__decoder_pad = pad

    def __pad_removed_cb(self, decoder, pad):
        if pad == self.__decoder_pad:
            self.logdebug("active output pad for stream %s removed", self.cutted_uri)
            self.__decoder_linked = False
            self.__decoder_pad = None
        else:
            self.logdebug("non-active output pad for stream %s removed .. what?", self.cutted_uri)

    def start_fade(self, start, end , time):
        format = gst.FORMAT_TIME

        try: 
            res = self.__volume.query_position(format)
            #b = self.__volume.get_base_time()
        except gst.QueryError: 
            res = None
        if res: 
            pos, qformat = res
        else: 
            pos, qformat = 0.0, format

        if qformat != format : pos = 0.0
        if pos < 0: pos = 100000.0
        pos = float(pos)

        start *= self.replaygain_scale
        end *= self.replaygain_scale

        self.__volume.handler_block_by_func(self.__volume_change_cb)
        self.__volume.set_passthrough(False)

        self.fade_end = end

        self.__volume.set_property("volume", start)
        self.__fader.unset_all("volume")

        if not self.__fader.set("volume", pos , start):
            self.logwarn("controller didn't like our start point")

        if not self.__fader.set("volume", 0.0 , start):
            self.logwarn("controller didn't like our 0 start point")

        if not self.__fader.set("volume", pos + time, end):
            self.logwarn("controller didn't like our end point")
 
        self.__volume.handler_unblock_by_func(self.__volume_change_cb)

        self.fading = True
        self.__volume.set_passthrough(False)
        #self.logdebug("New Volume data: %s",self.__fader.get_data("volume"))
    
    def reuse(self):
        self.__player.emit("reuse-stream", self.new_uri, self.cutted_uri)
        self.cutted_uri = self.new_uri
        self.new_uri = None
        if not self.link_and_unblock_stream():
            self.emit_stream_error("link_and_unblock_stream")

    def get_volume(self):
        return self.__volume.get_property("volume")

    def actually_start_stream(self):
        ret = True
        need_reap = False

        if self.crossfade > 0:
            to_fade = []
            self.logdebug("searching stream to remove...")
            self.__player.stream_list_lock.acquire()
            for pstream in self.__player.streams:
                if pstream == self: continue
                if pstream.state in [ FADING_IN, PLAYING ]:
                    to_fade.insert(0, pstream)
                elif pstream.state in [ PAUSED, WAITING_EOS, SEEKING, \
                        SEEKING_PAUSED, PREROLLING, \
                        PREROLL_PLAY, PENDING_REMOVE ]:

                    pstream.state = PENDING_REMOVE
                    need_reap = True

            self.__player.stream_list_lock.release()
            self.logdebug("Finish searching stream to remove, %d need to fade out", len(to_fade))
            
            for pstream in to_fade:
                fade_out_start = 1.0
                fade_out_time = float(self.crossfade) * gst.SECOND
                if pstream.is_in_buffer():
                    pstream.unlink_and_block_stream()
                    pstream.unlink_and_dispose_stream()
                    continue
                
                if pstream.state in [ FADING_IN ] :
                    fade_out_start = float(pstream.get_volume())
                    fade_out_time = (float(self.crossfade) * fade_out_start) * gst.SECOND
                if pstream.state in [ PLAYING, FADING_IN ]:
                    pstream.start_fade(fade_out_start, 0.0 , fade_out_time)
                    pstream.state = FADING_OUT
                    self.start_fade(0.0 , 1.0 , float(self.crossfade) * gst.SECOND)

            if not self.fading:
                self.__fader.unset_all("volume")
                self.__volume.set_property("volume", 1.0)
                if not self.__fader.set("volume", 0 , 1.0):
                    self.logwarn("controller didn't like our start point")

            ret = self.link_and_unblock_stream()

        elif self.crossfade == 0:
            playing = False
            self.__player.stream_list_lock.acquire()
            for pstream in self.__player.streams:
                if pstream == self: continue
                if pstream.state in [ PLAYING, FADING_OUT, FADING_IN ]:
                    playing = True
                elif pstream.state in [ PAUSED, PENDING_REMOVE ]:
                    pstream.state = PENDING_REMOVE
                    need_reap = True
            self.__player.stream_list_lock.release()
            if playing:
                self.state = WAITING_EOS
            else:
                # no stream found, start stream directly
                ret = self.link_and_unblock_stream()

        else:
            self.__player.stream_list_lock.acquire()
            for pstream in self.__player.streams:
                if pstream == self: continue
                if pstream.state in [ PLAYING, PAUSED, FADING_IN, PENDING_REMOVE ]:
                    need_reap = True
                    pstream.state = PENDING_REMOVE
            self.__player.stream_list_lock.release()

            ret = self.link_and_unblock_stream()

        if need_reap: self.__player.schedule_stream_reap()

        return ret 
    

    def __src_blocked_cb(self, pad, blocked):
        start_stream = False
        self.lock.acquire()
        self.logdebug("__src_blocked_cb -> stream %s state:%s", self.cutted_uri, self.get_str_state())
        if self.src_blocked:
            self.logwarn("stream %s already blocked", self.cutted_uri)
            self.lock.release()
            return

        self.src_blocked = True
        self.__preroll.set_property("min-threshold-time", 0)
        self.__preroll.set_property("max-size-buffers", 200)
        if self.state == PREROLLING:
            self.logdebug ("stream %s is prerolled, not starting yet -> WAITING", self.cutted_uri)
            self.state = WAITING
        elif self.state == PREROLL_PLAY:
            self.logdebug("stream %s is prerolled, need to start it", self.cutted_uri)
            start_stream = True
        else:
            self.logdebug("didn't expect to get preroll completion callback in this state (%s)", self.get_str_state()); 
            
        self.lock.release()
        if start_stream:
            if not self.actually_start_stream():
                self.emit_stream_error("actually_start_stream")
                self.logerror("Failed start Stream %s", self.cutted_uri)

    def is_blocked(self):
        return self.__src_pad.is_blocked()

    def get_linkage(self):
        """ For debug purpose """
        return self.__decoder_linked, self.__adder_pad is not None

    def preroll_stream(self):
        ret = True
        unblock = False
        self.logdebug("preroll_stream -> block stream %s self.__src_pad  cb:self.__src_blocked_cb state:%s", 
                      self.cutted_uri , self.get_str_state())
        self.__src_pad.set_blocked_async(True, self.__src_blocked_cb)
        self.emitted_playing = False
        self.state = PREROLLING
        if self.uri_scheme in BAD_STREAM_SCHEMES:
            self.fake_state = None
            def fake_set_state():
                if self.uri_scheme in RTSP_STREAM_SCHEMES:
                    self.fake_state = self.set_state(gst.STATE_PLAYING)
                else:    
                    self.fake_state = self.set_state(gst.STATE_PAUSED)
                    
            start = time.time()        
            fake_thread = Thread(target=fake_set_state, args=())
            fake_thread.setDaemon(True)
            fake_thread.start()
            
            while time.time() - start < self.bad_stream_timeout and fake_thread.isAlive():
                time.sleep(0.3)
                
            if self.fake_state is None:    
                return False
            else:
                state = self.fake_state
        # if self.uri_scheme in RTSP_STREAM_SCHEMES:
        #     state = self.set_state(gst.STATE_PLAYING)
        else:        
            state = self.set_state(gst.STATE_PAUSED)

        if state == gst.STATE_CHANGE_FAILURE:
            self.logdebug ("preroll for stream %s failed (state change failed)", self.cutted_uri)
            ret = False
        elif state == gst.STATE_CHANGE_NO_PREROLL:
            self.logdebug ("no preroll for stream %s -> WAITING", self.cutted_uri)
            unblock = True
            self.state = WAITING
        elif state == gst.STATE_CHANGE_SUCCESS:
            if self.__decoder_linked:
                self.logdebug ("stream %s prerolled synchronously -> WAITING", self.cutted_uri)
                self.state = WAITING
                assert(self.src_blocked)
                unblock = True
            else:
                self.logwarn("stream %s did not preroll; probably missing a decoder", self.cutted_uri)
                ret = False
        elif state == gst.STATE_CHANGE_ASYNC:
            self.logdebug("state change async for stream %s", self.cutted_uri)
            pass
        else:
            self.logwarn("State not reached %s", self.cutted_uri)
            ret = False

        if unblock:
            self.logdebug("preroll_stream -> unblock stream %s __src_pad cb:None state:%s", self.cutted_uri, self.get_str_state())
            self.__src_pad.set_blocked_async(False)
        
        if not ret:
            if self.uri_scheme == CDDA_STREAM_SCHEME:
                self.emit_stream_error("Failed to preroll stream")
            self.logerror("Failed to preroll stream")
        return ret

    def query_stream_position(self, format):
        return self.__volume.query_position(format)

    def query_stream_duration(self, format):
        return self.__volume.query_duration(format)

    def query_stream(self, query):
        return self.__volume.query(query)

    def perform_seek(self):
        self.logdebug("sending seek event.., %s", self.cutted_uri)
        event = gst.event_new_seek(1.0, gst.FORMAT_TIME, gst.SEEK_FLAG_FLUSH, gst.SEEK_TYPE_SET, self.seek_target, gst.SEEK_TYPE_NONE, -1)
        ret = self.__src_pad.send_event(event)
        if not ret:
            self.logwarn("failed seek %s", self.cutted_uri)
        if self.state == SEEKING:
            self.state = PLAYING;
        elif self.state == SEEKING_PAUSED:
            self.logdebug("leaving paused stream %s unlinked", self.cutted_uri)
            self.state = PAUSED
        elif self.state == SEEKING_EOS:
            self.logdebug("waiting for pad block to complete for %s before unlinking", self.cutted_uri)
            
        if self.state == PLAYING:    
            self.actually_start_stream()

    def perform_seek_idle(self):
        self.perform_seek()
        return False

    def post_eos_seek_blocked_cb (self, pad, blocked):
        self.lock.acquire()
        self.logdebug ("stream %s is blocked; linking and unblocking", self.cutted_uri);
        self.src_blocked = True;
        if not self.link_and_unblock_stream (): 
            self.emit_stream_error ("link_and_unblock_stream failed")
        self.lock.release()


    def post_eos_block_src_pad(self):
        # blocked must be True
        self.__src_pad.set_blocked_async(True, self.post_eos_seek_blocked_cb)
    
    def send_event(self, element, event):
        ghost_pad = element.get_pad("src")
        pad = ghost_pad.get_target()

        ret = pad.get_parent().send_event(event)
        return ret

    def dispose_stream_data(self):
        if self.stream_data and self.stream_data_desktroy:
            self.stream_data_desktroy(self.stream_data)
        self.stream_data = None
        self.stream_data_desktroy = None


