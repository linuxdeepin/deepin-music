#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import traceback
import common
from PyQt5.QtCore import QDir, QUrl
import chardet
import copy
import json
import mutagen
from log import logger

TAG_KEYS = {
    'title' : 'title',
    'artist': 'artist',
    'album' : 'album',
    'tracknumber': 'tracknumber',
    'discnumber': 'discnumber',
    'genre': 'genre',
    'date': 'date'
}

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

    def __init__(self, url):
        super(Song, self).__init__()
        self.__dict__ = self
        for key in TAG_KEYS:
            if key in ['tracknumber', 'discnumber']:
                self[key] = 0
            elif key in ['title', 'artist', 'album']:
                self[key] = "unknown"

        from os.path import abspath, realpath, normpath
        self['url'] = normpath(realpath(abspath(url)))
        self['folder'] = os.path.dirname(self['url'])

        if self.isExisted():
            try:
                self.getTags()
            except Exception, e:
                logger.error(e)

    def isExisted(self):
        return os.path.exists(self.url)

    def isLocalFile(self):
        return QUrl.fromLocalFile(self.url).isLocalFile()

    @property
    def baseName(self):
        return os.path.basename(self.url)

    @property
    def fileName(self):
        return os.path.splitext(self.baseName)[0]

    @property
    def ext(self):
        return os.path.splitext(self.baseName)[1][1:]

    def getTags(self):
        path = self.url
        self["size"] = os.path.getsize(path)

        audio = common.MutagenFile(path, common.FORMATS)

        if audio is not None:
            tag_keys_override = TAGS_KEYS_OVERRIDE.get(
                audio.__class__.__name__, None)
            for tag, file_tag in TAG_KEYS.iteritems():
                if tag_keys_override and tag_keys_override.has_key(file_tag):
                    file_tag = tag_keys_override[file_tag]
                if audio.has_key(file_tag) and audio[file_tag]:
                    value = audio[file_tag]
                    if isinstance(value, list) or isinstance(value, tuple):
                        value = value[0]
                        if isinstance(value, mutagen.asf.ASFUnicodeAttribute):
                            value = value.value
                    fix_value = common.fix_charset(value)
                    if fix_value == "[Invalid Encoding]":
                        if tag == "title":
                            fix_value = self.fileName
                        else:
                            fix_value = ""
                    self[tag] = fix_value
                else:
                    self[tag] = 0

            for key in ['sample_rate', 'bitrate', 'length']:
                try:
                    if hasattr(audio.info, key):
                        if key == 'length':
                            self['duration'] = getattr(audio.info, key) * 1000
                        else:
                            self[key] = getattr(audio.info, key)
                except Exception, e:
                    print e

    def saveTags(self):
        ''' Save tag information to file. '''
        if not self.isLocalFile():
            self.last_error = self.url + " " + "is not a local file"
            return False
        if not self.isExisted():
            self.last_error = self.url + " doesn't exist"
            return False
        if not os.access(self.url, os.W_OK):
            self.last_error = self.url + " doesn't have enough permission"
            return False

        try:
            audio = common.MutagenFile(self.url, common.FORMATS)
            tag_keys_override = None

            if audio is not None:
                if audio.tags is None:
                    audio.add_tags()
                tag_keys_override = TAGS_KEYS_OVERRIDE.get(
                    audio.__class__.__name__, None)

                for file_tag, tag in TAG_KEYS.iteritems():
                    if tag_keys_override and tag_keys_override.has_key(file_tag):
                        file_tag = tag_keys_override[file_tag]
                    if self.get(tag):    
                        try:
                            if audio.__class__.__name__ == "MP4" and tag == "tracknumber":
                                _value = self.get(tag)
                                _vtuple = _value.split(',')
                                if len(_vtuple) == 2:
                                    value = [(int(_vtuple[0][1:].strip()), int(_vtuple[1][:-1].strip()))]
                                else:
                                    value = self.get(tag)
                            else:
                                value = self.get(tag)
                        except Exception, e:
                            value = self.get(tag).decode('utf-8')
                        audio[file_tag] = value
                    else:
                        try:
                            del(audio[file_tag]) # TEST
                        except KeyError:
                            pass
                audio.save()
            else:
                raise "w:Song:MutagenTag:No audio found"

        except Exception, e:
            print traceback.format_exc()
            print "W: Error while writting (" + self.get("url") + ")\nTracback :", e
            self.last_error = "Error while writting" + \
                ": " + self.url
            return False
        else:
            return True

    def __setitem__(self, key, value):
        if key == "tracknumber":
            value = value
            if isinstance(value, tuple):
                value = value[0]
            else:
                value = 0
            # elif value is not None and not isinstance(value,int) and value.rfind("/")!=-1 and value.strip()!="":
            #     value = str(value)
            #     value = value.strip()
            #     discnumber = value[value.rfind("/"):]
            #     try: 
            #         discnumber = int(discnumber)
            #     except: 
            #         discnumber = 0
            #     self["discnumber"] = discnumber
            #     value = int(value[:value.rfind("/")])
        elif key == 'date':
            if isinstance(value, int):
                value = str(value).decode('utf-8')

        elif key == 'genre':
            if isinstance(value, int):
                value = str(value)

        elif key == "discnumber":
            # if isinstance(value, str):
            value = 0
        elif key in ['title', 'artist', 'album']:
            if not value:
                value = 'unknown'
        if value is None:        
            if key in self:
                super(Song, self).__delitem__(key)
        else:
            super(Song, self).__setitem__(key, value)

    def getMp3FontCover(self):
        from common import EasyMP3
        ext = None
        img_data = None
        try:
            audio = common.MutagenFile(self.url, common.FORMATS)
            if isinstance(audio, EasyMP3):
                if audio.tags:
                    apics = audio.tags.getID3().getall('APIC')
                    if len(apics) > 0:
                        apic = apics[0]
                        if apic.type == 3:
                            mine = apic.mime
                            ext = mine.split('/')[-1]
                            img_data = apic.data
        except Exception, e:
            logger.error(e)
        return ext, img_data

    def pprint(self):
        keys = [
            'url',
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
