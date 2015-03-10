#!/usr/bin/python
# -*- coding: utf-8 -*-

import chardet
import os
import common


coreTags = [
    'title', 
    'artist', 
    'album',
    'tracknumber', 
    'discnumber',
    'genre',
    'date'
]

TAGS_KEYS_OVERRIDE = {}

TAGS_KEYS_OVERRIDE['Musepack'] = {"tracknumber":"track","date":"year"}

TAGS_KEYS_OVERRIDE['MP4'] = {
        "title":"\xa9nam",
        "artist":"\xa9ART",
        "album":"\xa9alb",
        "tracknumber":"trkn",
        "discnumber":"disk",
        "genre":"\xa9gen",
        "date":"\xa9day"
        }

TAGS_KEYS_OVERRIDE['ASF'] = {
        "title":"Title",
        "artist":"Author",
        "album":"WM/AlbumArtist",
        "tracknumber":"WM/TrackNumber",
        "discnumber":"WM/PartOfSet",
        "genre":"WM/Genre",
        "date":"WM/Year"
        }


def auto_decode(s):    
    ''' auto detect the code and return unicode object. '''
    if isinstance(s, unicode):
        return s
    try:
        return s.decode("gbk")
    except UnicodeError:
        try:
            codedetect = chardet.detect(s)["encoding"]
            return s.decode(codedetect)
        except:    
            return "[Invalid Encoding]"    
    
def fix_charset(s):    
    '''Fix the charset. unicode error'''
    if not s: return ""
    repr_char = repr(s) 
    if repr_char.startswith("u"):
        if repr_char.find("\u") != -1:
            return s.encode("utf-8")
        return auto_decode(eval(repr_char[1:])).encode("utf-8")
    else:
        return s

class Song(dict):
    """docstring for Song"""
    def __init__(self, uri):
        super(Song, self).__init__()
        self.__dict__ = self
        from os.path import abspath, realpath, normpath
        self.uri = normpath(realpath(abspath(uri)))
        self.getTags()

    def getTags(self):
        path = self.uri
        self["size"]  = os.path.getsize(path)

        audio = common.MutagenFile(path, common.FORMATS)
        # print audio.tags.getID3().getall('TLEN')
        if audio is not None:
            tag_keys_override = TAGS_KEYS_OVERRIDE.get(audio.__class__.__name__, None)
            for file_tag in coreTags:
                if tag_keys_override and tag_keys_override.has_key(file_tag):
                    file_tag = tag_keys_override[file_tag]
                if audio.has_key(file_tag) and audio[file_tag]:
                    value = audio[file_tag]
                    if isinstance(value, list) or isinstance(value, tuple):
                        value = value[0]
                    fix_value = fix_charset(value)    
                    if fix_value == "[Invalid Encoding]":
                        if tag == "title":
                            fix_value = song.get_filename()
                        else:    
                            fix_value = ""
                    self[file_tag] = fix_value
                else:
                    self[file_tag] = None

            for key in ['sample_rate', 'bitrate', 'length']:
                try:
                    if key == 'length':
                        self['duration'] = getattr(audio.info, key)
                    else:
                        self[key] = getattr(audio.info, key)
                except Exception, e:
                    print e

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
        ret = '\n'.join([ '%s: %s' % (key, self[key]) for key in keys])
        return ret


song = Song('../music/1.mp3')

print song.pprint()
# print song.getMp3FontCover()