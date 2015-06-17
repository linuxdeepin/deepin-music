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

import os

from mutagen import File as MutagenFile
from mutagen.asf import ASF
from mutagen.apev2 import APEv2File
from mutagen.flac import FLAC
from mutagen.id3 import ID3FileType
from mutagen.oggflac import OggFLAC
from mutagen.oggspeex import OggSpeex
from mutagen.oggtheora import OggTheora
from mutagen.oggvorbis import OggVorbis
from mutagen.trueaudio import TrueAudio
from mutagen.wavpack import WavPack
try: from mutagen.mp4 import MP4 #@UnusedImport
except: from mutagen.m4a import M4A as MP4 #@Reimport
from mutagen.musepack import Musepack
from mutagen.monkeysaudio import MonkeysAudio
from mutagen.optimfrog import OptimFROG
from easymp3 import EasyMP3


FORMATS = [EasyMP3, TrueAudio, OggTheora, OggSpeex, OggVorbis, OggFLAC,
            FLAC, APEv2File, MP4, ID3FileType, WavPack, Musepack,
            MonkeysAudio, OptimFROG, ASF]

UNTRUST_AUDIO_EXT = [
    "669", "ac3", "aif", "aiff", "ape", "amf", "au",
    "dsm", "far", "it", "med", "mka", "mpc", "mid", 
    "mod", "mtm", "midi", "oga", "ogx", "okt", "ra",
    "ram", "s3m", "sid", "shn", "snd", "spc", "spx",
    "stm", "tta", "ult", "wv", "xm"
             ]

TRUST_AUDIO_EXT = [
    "wav", "wma", "mp2", "mp3", "mp4", "m4a", "flac", "ogg"
    ]

def file_is_supported(filename, strict=False):
    ''' whther file is supported. '''
    
    try:
        if os.path.getsize(filename) < 10000:
            return False
    except:    
        return False
    
    results = os.path.basename(filename).split(".")
    if len(results) < 2:
        return False
    else:
        extension = results[-1].lower()
        if extension in TRUST_AUDIO_EXT:
            return True
        elif extension in UNTRUST_AUDIO_EXT:
            try:
                fileobj = file(filename, "rb")
            except:
                return False
            try:
                header = fileobj.read(128)
                results = [Kind.score(filename, fileobj, header) for Kind in FORMATS]
            except:    
                return False
            finally:
                fileobj.close()
            results = zip(results, FORMATS)
            results.sort()
            score, Kind = results[-1]
            if score > 0: return True
            else: return False
        else:    
            return False

def get_audio_length(path):        
    try:
        audio = MutagenFile(path, FORMATS)
        if audio is not None:
            return int(audio.info.length) * 1000
        return 0
    except:
        return 0
