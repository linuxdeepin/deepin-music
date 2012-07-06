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


import mutagen.id3
from mutagen import Metadata
from mutagen._util import DictMixin
from mutagen.id3 import ID3 as mutagenID3, error
from mutagen.mp3 import MP3
import utils

__all__ = ['EasyID3', 'Open', 'delete']


class EasyID3(DictMixin, Metadata):
    """A file with an ID3 tag.

    Like Vorbis comments, EasyID3 keys are case-insensitive ASCII
    values. Only a subset of ID3 frames (those with simple text keys)
    are supported; EasyID3.valid_keys maps human-readable EasyID3
    names to ID3 frame IDs.

    To use an EasyID3 class with mutagen.mp3.MP3:
        from mutagen.mp3 import MP3
        from mutagen.easyid3 import EasyID3
        MP3(filename, ID3=EasyID3)
    """

    valid_keys = {
        "album": "TALB",
        "composer": "TCOM",
        "genre": "TCON",
        "date": "TDRC",
        "lyricist": "TEXT",
        "title": "TIT2",
        "version": "TIT3",
        "artist": "TPE1",
        "tracknumber": "TRCK",
        "discnumber": "TPOS"
        }
    """Valid keys for EasyID3 instances."""

    def __init__(self, filename=None):
        self.__id3 = mutagenID3()
        self.load = self.__id3.load
        self.save = self.__id3.save
        self.delete = self.__id3.delete
        if filename is not None:
            self.load(filename)

    filename = property(lambda s: s.__id3.filename,
                        lambda s, fn: setattr(s.__id3, 'filename', fn))

    _size = property(lambda s: s._id3.size,
                     lambda s, fn: setattr(s.__id3, '_size', fn))

    """ Correct latin 1 string for borken mp3 tag"""
    def __distrust_latin1(self,text, encoding=0):
        assert isinstance(text, unicode)
        if encoding == 0:
            text = text.encode('iso-8859-1')
            for codec in ["gbk", "big5", 'utf-8','iso-8859-1','iso-8559-15','cp1251']:
                try: text = text.decode(codec)
                except (UnicodeError, LookupError): pass
                else:break
            else: return None
        return text
    
    def __TCON_get(self, frame):
        return frame.genres

    def __TCON_set(self, frame, value):
        frame.encoding = 3
        if not isinstance(value, list):
            value = [value]
        frame.genres = value

    def __TDRC_get(self, frame):
        return [stamp.text for stamp in frame.text]

    def __TDRC_set(self, frame, value):
        self.__id3.add(mutagen.id3.TDRC(encoding=3, text=value))

    def __text_get(self, frame):
        modified_frame = list([self.__distrust_latin1(item,frame.encoding) for item in list(frame)])
        return modified_frame
    
    def __text_set(self, frame, value):
        frame.encoding = 3
        if not isinstance(value, list):
            value = [value]
        frame.text = value

    def __getitem__(self, key):
        key = key.lower()
        if key in self.valid_keys:
            frame = self.valid_keys[key]
            getter = self.__mungers.get(frame, self.__default)[0]
            return getter(self, self.__id3[frame])
        else: raise ValueError("%r is not a valid key" % key)

    def __setitem__(self, key, value):
        key = key.lower()
        if key in self.valid_keys:
            frame = self.valid_keys[key]
            setter = self.__mungers.get(frame, self.__default)[1]
            if frame not in self.__id3:
                frame = mutagen.id3.Frames[frame](encoding=3, text=value)
                self.__id3.loaded_frame(frame)
            else:
                setter(self, self.__id3[frame], value)
        else: raise ValueError("%r is not a valid key" % key)

    def __delitem__(self, key):
        key = key.lower()
        if key in self.valid_keys:
            del(self.__id3[self.valid_keys[key]])
        else: raise ValueError("%r is not a valid key" % key)

    def keys(self):
        return [k for (k, v) in self.valid_keys.items() if v in self.__id3]

    def pprint(self):
        """Print tag key=value pairs."""
        strings = []
        for key in self.keys():
            values = self[key]
            for value in values:
                strings.append("%s=%s" % (key, value))
        return "\n".join(strings)

    __mungers = {
        "TCON": (__TCON_get, __TCON_set),
        "TDRC": (__TDRC_get, __TDRC_set),
        }

    __default = (__text_get, __text_set)

Open = EasyID3


class EasyMP3(MP3):
    def __init__(self, filename=None):
        super(EasyMP3,self).__init__(filename, ID3=EasyID3)
        
    def add_tags(self):
        if self.tags is None:
            self.tags = EasyID3()
        else:
            raise error("an ID3 tag already exists")
        

