#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import datetime
from dwidgets.peewee import *
import traceback
try:
    from dwidgets.mediatag import common
except:
    raise
    import sys
    sys.path.insert(0, os.path.join(os.path.dirname(os.getcwd()), 'dwidgets'))
    from mediatag import common

from PyQt5.QtCore import QDir, QUrl
import chardet
import copy



try:
    from config.constants import MusicDBFile
except:
    MusicDBFile = 'music.db'

db = SqliteDatabase(MusicDBFile, threadlocals=True)


class BaseModel(Model):
    class Meta:
        database = db

    @classmethod
    @db.atomic()
    def getRecord(cls,  **kwargs):
        key = getattr(cls, '__key__')
        assert key in kwargs
        try:
            ret = cls.get(getattr(cls, key) == kwargs[key])
            return ret
        except DoesNotExist:
            return None

    @classmethod
    def isRecordExisted(cls, **kwargs):
        ret = cls.getRecord(**kwargs)
        if ret:
            return True
        else:
            return False

    @classmethod
    @db.atomic()
    def updateRecord(cls, **kwargs):
        key = getattr(cls, '__key__')
        assert key in kwargs
        retId = cls.update(**kwargs).where(getattr(cls, key) == kwargs[key]).execute()
        if retId != 0:
            return True
        else:
            return False

    @classmethod
    @db.atomic()
    def createRecord(cls, **kwargs):
        key = getattr(cls, '__key__')
        assert key in kwargs
        try:
            ret = cls.create(**kwargs)
        except IntegrityError:
            ret = None
        return ret


    @classmethod
    @db.atomic()
    def get_create_Record(cls, **kwargs):
        key = getattr(cls, '__key__')
        assert key in kwargs
        try:
            ret = cls.create(**kwargs)
        except IntegrityError:
            # print('%s is already in use' % kwargs['url'])
            retId = cls.update(**kwargs).where(getattr(cls, key) == kwargs[key]).execute()
            if retId != 0:
                ret = cls.get(cls.id==retId)
            else:
                ret = None
        return ret

    @classmethod
    def get_create_Records(cls, records):
        with db.transaction():
            for record in records:
                key = getattr(cls, '__key__')
                assert key in record
                try:
                    ret = cls.create(**record)
                except IntegrityError:
                    cls.update(**record).where(getattr(cls, key) == record[key]).execute()


class Artist(BaseModel):
    name = CharField(default='', unique=True)
    area = CharField(default='')
    summary = CharField(default='')

    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'name'


class Album(BaseModel):
    name = CharField(default='', unique=True)
    artist = CharField(default='')

    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'name'


class Folder(BaseModel):
    name = CharField(default='', unique=True)

    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'name'
    

class Playlist(BaseModel):
    name = CharField(default='', unique=True)
    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'name'

    @classmethod
    def getPlaylistByName(cls, name):
        try:
            playlistRecord = cls.get(cls.name == name)
        except DoesNotExist:
            playlistRecord = None
        return playlistRecord


class Song(BaseModel):
    fartist = ForeignKeyField(Artist, related_name='songs')
    falbum = ForeignKeyField(Album, related_name='songs')
    ffolder = ForeignKeyField(Folder, related_name='songs')

    url = CharField(default='', unique=True)
    folder = CharField(default='')

    #Common attributes
    title = CharField(default='')
    artist = CharField(default='')
    album = CharField(default='')
    tracknumber = IntegerField(default=0)
    discnumber = IntegerField(default=0)
    genre = CharField(default='')
    date = CharField(default='')

    #Media attributes
    size = IntegerField(default=0)
    mediaType = CharField(default='')
    duration = IntegerField(default=0)

    #Audio attributes
    bitrate = IntegerField(default=0)
    sample_rate = IntegerField(default=0)

    #Music attributes
    cover = CharField(default='')

    #other
    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'url'

    def pprint(self):
        keys = [
            'url',
            'folder',
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
            'cover',
        ]

        p = {}
        for key in keys:
            if hasattr(self, key):
                if isinstance(getattr(self, key) , unicode):
                    p[key] = getattr(self, key).encode('utf-8')
                else:
                    p[key] = getattr(self, key)
        ret = '\n'.join(['%s: %s' % (key, p[key]) for key in keys if p.has_key(key)])
        return ret

    def toDict(self):
        keys = [
            'url',
            'folder',
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
            'cover',
        ]

        p = {}
        for key in keys:
            if hasattr(self, key):
                p[key] = getattr(self, key)
        return p


class OnlineSong(BaseModel):

    url = CharField(default='', unique=True)

    #Common attributes
    title = CharField(default='')
    artist = CharField(default='')
    album = CharField(default='')
    tracknumber = IntegerField(default=0)
    discnumber = IntegerField(default=0)
    genre = CharField(default='')
    date = CharField(default='')

    #Media attributes
    size = IntegerField(default=0)
    mediaType = CharField(default='')
    duration = IntegerField(default=0)

    #Audio attributes
    bitrate = IntegerField(default=0)
    sample_rate = IntegerField(default=0)

    #Music attributes
    cover = CharField(default='')
    albumImage_100x100 = CharField(default='')
    albumImage_500x500 = CharField(default='')

    songId = IntegerField(default=0)
    singerId =  IntegerField(default=0)
    albumId = IntegerField(default=0)

    serviceEngName = CharField(default='')
    serviceName = CharField(default='')
    serviceUrl = CharField(default='')

    playlinkUrl = CharField(default='')

    #other
    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'url'

    def pprint(self):
        keys = [
            'url',
            'folder',
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
            'cover',
        ]

        p = {}
        for key in keys:
            if hasattr(self, key):
                if isinstance(getattr(self, key) , unicode):
                    p[key] = getattr(self, key).encode('utf-8')
                else:
                    p[key] = getattr(self, key)
        ret = '\n'.join(['%s: %s' % (key, p[key]) for key in keys if p.has_key(key)])
        return ret

    def toDict(self):
        keys = [
            'url',
            'folder',
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
            'cover',
        ]

        p = {}
        for key in keys:
            if hasattr(self, key):
                p[key] = getattr(self, key)
        return p