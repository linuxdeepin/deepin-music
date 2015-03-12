#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import traceback
import common
from PyQt5.QtCore import QDir, QUrl
import chardet
import copy

TAG_KEYS = [
    'title',
    'artist',
    'album',
    'tracknumber',
    'discnumber',
    'genre',
    'date'
]

TAGS_KEYS_OVERRIDE = {}

TAGS_KEYS_OVERRIDE['Musepack'] = {"tracknumber": "track", "date": "year"}

TAGS_KEYS_OVERRIDE['MP4'] = {
    "title": "\xa9nam",
    "artist": "\xa9ART",
    "album": "\xa9alb",
    "tracknumber": "trkn",
    "discnumber": "disk",
    "genre": "\xa9gen",
    "date": "\xa9day"
}

TAGS_KEYS_OVERRIDE['ASF'] = {
    "title": "Title",
    "artist": "Author",
    "album": "WM/AlbumArtist",
    "tracknumber": "WM/TrackNumber",
    "discnumber": "WM/PartOfSet",
    "genre": "WM/Genre",
    "date": "WM/Year"
}





class Song(dict):

    """docstring for Song"""

    def __init__(self, uri):
        super(Song, self).__init__()
        self.__dict__ = self
        from os.path import abspath, realpath, normpath
        self.uri = normpath(realpath(abspath(uri)))

        if self.isExisted():
            self.getTags()

    def isExisted(self):
        return os.path.exists(self.uri)

    def isLocalFile(self):
        return QUrl.fromLocalFile(self.uri).isLocalFile()

    @property
    def baseName(self):
        return os.path.basename(self.uri)

    @property
    def fileName(self):
        return os.path.splitext(self.baseName)[0]

    @property
    def ext(self):
        return os.path.splitext(self.baseName)[1][1:]

    def getTags(self):
        path = self.uri
        self["size"] = os.path.getsize(path)

        audio = common.MutagenFile(path, common.FORMATS)

        if audio is not None:
            tag_keys_override = TAGS_KEYS_OVERRIDE.get(
                audio.__class__.__name__, None)
            for file_tag in TAG_KEYS:
                if tag_keys_override and tag_keys_override.has_key(file_tag):
                    file_tag = tag_keys_override[file_tag]
                if audio.has_key(file_tag) and audio[file_tag]:
                    value = audio[file_tag]
                    if isinstance(value, list) or isinstance(value, tuple):
                        value = value[0]
                    fix_value = common.fix_charset(value)
                    if fix_value == "[Invalid Encoding]":
                        if tag == "title":
                            fix_value = self.fileName
                        else:
                            fix_value = ""
                    self[file_tag] = fix_value
                else:
                    self[file_tag] = None

            for key in ['sample_rate', 'bitrate', 'length']:
                try:
                    if hasattr(audio.info, key):
                        if key == 'length':
                            self['duration'] = getattr(audio.info, key)
                        else:
                            self[key] = getattr(audio.info, key)
                except Exception, e:
                    print e

    def saveTags(self):
        ''' Save tag information to file. '''
        if not self.isLocalFile():
            self.last_error = self.uri + " " + "is not a local file"
            return False
        if not self.isExisted():
            self.last_error = self.uri + " doesn't exist"
            return False
        if not os.access(self.uri, os.W_OK):
            self.last_error = self.uri + " doesn't have enough permission"
            return False

        try:
            audio = common.MutagenFile(self.uri, common.FORMATS)
            tag_keys_override = None

            if audio is not None:
                if audio.tags is None:
                    audio.add_tags()
                tag_keys_override = TAGS_KEYS_OVERRIDE.get(
                    audio.__class__.__name__, None)

                for file_tag in TAG_KEYS:
                    if tag_keys_override and tag_keys_override.has_key(file_tag):
                        file_tag = tag_keys_override[file_tag]

                    if self.get(file_tag):
                        try:
                            value = unicode(self.get(file_tag))
                        except Exception, e:
                            value = self.get(file_tag).decode('utf-8')
                        # print file_tag, value, type(value)
                        audio[file_tag] = value
                    else:
                        try:
                            del(audio[file_tag])  # TEST
                        except KeyError:
                            pass
                audio.save()
            else:
                raise "w:Song:MutagenTag:No audio found"

        except Exception, e:
            print traceback.format_exc()
            print "W: Error while writting (" + self.get("uri") + ")\nTracback :", e
            self.last_error = "Error while writting" + \
                ": " + self.uri
            return False
        else:
            return True

    def getMp3FontCover(self):
        from common import EasyMP3
        audio = common.MutagenFile(self.uri, common.FORMATS)
        ext = None
        img_data = None
        if isinstance(audio, EasyMP3):
            apics = audio.tags.getID3().getall('APIC')
            if len(apics) > 0:
                apic = apics[0]
                if apic.type == 3:
                    mine = apic.mime
                    ext = mine.split('/')[-1]
                    img_data = apic.data
        return ext, img_data

    def pprint(self):
        keys = [
            'uri',
            'title',
            'artist',
            'album',
            'date',
            'genre',
            'tracknumber',
            'discnumber',
            'sample_rate',
            'bitrate',
            'duration',
            'size',
        ]

        p = copy.deepcopy(self)
        for key in keys:
            if self.has_key(key) and isinstance(self[key], unicode):
                p[key] = self[key].encode('utf-8')
        ret = '\n'.join(['%s: %s' % (key, p[key]) for key in keys if self.has_key(key)])
        return ret


if __name__ == '__main__':
    # ydir = QDir('/home/djf/workspace/yhm/')
    # print ydir.entryList()
    # print ydir.entryInfoList()
    # song = Song('/home/djf/workspace/yhm/游鸿明-下沙.ape')
    # print song.pprint()
    # print song
    # print song.isExisted(), song.baseName, song.fileName, song.ext
    # song.title = u'dddddddddd愿得一人心'
    # song.saveTags()

    song = Song('../../../music/1.mp3')
    print song.pprint(), song
    # print song.getMp3FontCover()
