#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from PyQt5.QtCore import (
    pyqtProperty, QObject, 
    pyqtSlot, pyqtSignal, 
    QThread)

from peewee import *
import datetime

db = SqliteDatabase('music.db', threadlocals=True)

class BaseModel(Model):
    class Meta:
        database = db


class Singer(BaseModel):
    name = CharField(default='', unique=True)
    area = CharField(default='')
    summary = CharField(default='')

    created_date = DateTimeField(default=datetime.datetime.now)

class Album(BaseModel):
    title = CharField(default='', unique=True)
    artist = CharField(default='')

    created_date = DateTimeField(default=datetime.datetime.now)


class Folder(BaseModel):
    path = CharField(default='', unique=True)

    created_date = DateTimeField(default=datetime.datetime.now)


class Playlist(BaseModel):
    name = CharField(default='', unique=True)
    created_date = DateTimeField(default=datetime.datetime.now)


class Song(BaseModel):

    fsinger = ForeignKeyField(Singer, related_name='song')
    falbum = ForeignKeyField(Album, related_name='song')
    ffolder = ForeignKeyField(Folder, related_name='song')

    uri = CharField(default='', unique=True)

    #Common attributes
    title = CharField(default='')
    subTitle = CharField(default='')
    author = CharField(default='')
    comment = CharField(default='')
    description = CharField(default='')
    category = CharField(default='')
    genre = CharField(default='')
    date = CharField(default='')
    year = IntegerField(default=0)
    userRating = CharField(default='')
    keywords = CharField(default='')
    language = CharField(default='')
    publisher = CharField(default='')
    copyright = CharField(default='')
    parentalRating = CharField(default='')
    ratingOrganization = CharField(default='')

    #Media attributes
    size = IntegerField(default=0)
    mediaType = CharField(default='')
    duration = IntegerField(default=0)

    #Audio attributes
    audioBitRate = IntegerField(default=0)
    audioCodec = CharField(default='')
    averageLevel = IntegerField(default=0)
    channelCount = IntegerField(default=0)
    peakValue = IntegerField(default=0)
    sampleRate = IntegerField(default=0)

    #Music attributes
    albumArtist = CharField(default='')
    albumTitle = CharField(default='')
    contributingArtist = CharField(default='')
    composer = CharField(default='')
    conductor = CharField(default='')
    lyrics = CharField(default='')
    mood = CharField(default='')
    trackNumber = CharField(default='')
    trackCount = CharField(default='')
    coverArtUrlLarge = CharField(default='')
    coverArtUrlSmall = CharField(default='')

    #other
    created_date = DateTimeField(default=datetime.datetime.now)


class SongPlaylist(BaseModel):
    song = ForeignKeyField(Song)
    playlist = ForeignKeyField(Playlist)


def getTagsByUri(uri):
    kwargs = {}
    kwargs.update({'uri': uri})
    return kwargs

class DBWorker(QObject):

    def __init__(self):
        super(DBWorker, self).__init__()
        db.connect()
        db.create_tables([Song, Singer, Album, Folder, Playlist, SongPlaylist], safe=True)

    @classmethod
    @db.atomic()
    def get_create_SongInstance(cls, **kwargs):
        try:
            ret = Song.create(**kwargs)
        except IntegrityError:
            # print('%s is already in use' % kwargs['uri'])
            songId = Song.update(**kwargs).where(Song.uri == kwargs['uri']).execute()
            if songId != 0:
                ret = Song.get(Song.id==songId)
            else:
                ret = None
        return ret

    @classmethod
    @db.atomic()
    def get_create_SingerInstance(cls, **kwargs):
        try:
            ret = Singer.create(**kwargs)
        except IntegrityError:
            # print('%s is already in use' % kwargs['name'])
            try:
                ret = Singer.get(name=kwargs['name'])
            except DoesNotExist:
                ret = None

        return ret

    @classmethod
    @db.atomic()
    def get_create_AlbumInstance(cls, **kwargs):
        try:
            ret = Album.create(**kwargs)
        except IntegrityError:
            # print('%s is already in use' % kwargs['title'])
            try:
                ret = Album.get(title=kwargs['title'])
            except DoesNotExist:
                ret = None
        return ret

    @classmethod
    @db.atomic()
    def get_create_FolderInstance(cls, **kwargs):
        try:
            ret = Folder.create(**kwargs)
        except IntegrityError:
            # print('%s is already in use' % kwargs['path'])
            try:
                ret = Folder.get(path=kwargs['path'])
            except DoesNotExist:
                ret = None

        return ret

    @classmethod
    @db.atomic()
    def get_create_PlaylistInstance(cls, **kwargs):
        try:
            ret = Playlist.create(**kwargs)
        except IntegrityError:
            # print('%s is already in use' % kwargs['name'])
            try:
                ret = Playlist.get(name=kwargs['name'])
            except DoesNotExist:
                ret = None
        return ret

    @classmethod
    @db.atomic()
    def get_create_SongPlaylistInstance(cls, **kwargs):
        try:
            ret = SongPlaylist.create(**kwargs)
        except IntegrityError:
            # print(kwargs, 'is already in use')
            try:
                ret = SongPlaylist.get(**kwargs)
            except DoesNotExist:
                ret = None
        return ret

    @classmethod  
    def getSongByUri(cls, uri):
        try:
            songInstance = Song.get(Song.uri == uri)
        except DoesNotExist:
            songInstance = None
        return songInstance

    @classmethod
    def getPlaylistByName(cls, name):
        try:
            playlistInstance = Playlist.get(Playlist.name == name)
        except DoesNotExist:
            playlistInstance = None
        return playlistInstance

    @classmethod
    def addSongToPlaylist(cls, uri, name='temporary'):
        songInstance = cls.getSongByUri(uri)
        playlistInstance = cls.getPlaylistByName(name)

        if songInstance and playlistInstance:
            kwargs = {
                'song': songInstance, 
                'playlist': playlistInstance
            }
            try:
                ret = SongPlaylist.get(SongPlaylist.song==songInstance, 
                    SongPlaylist.playlist==playlistInstance)
                if ret:
                    # print('SongPlaylist existed, Emit Singal')
                    pass
            except DoesNotExist:
                ret = SongPlaylist.create(**kwargs)
        else:
            ret = None

        return ret

    @classmethod
    def getSongsByPlaylistName(cls, name):
        songs = []
        for song in Song.select().join(SongPlaylist).join(Playlist).where(Playlist.name==name):
            songs.append(song.uri)

        return songs

    @classmethod
    def getPlaylistsBySongUri(cls, uri):
        playlists = []
        for playlist in Playlist.select().join(SongPlaylist).join(Song).where(Song.uri==uri):
            playlists.append(playlist.name)
        return playlists


dbWorker = DBWorker()

def createPlaylists():
    names = ['temporary', 'favourite']
    for name in names:
        playlistInstance = dbWorker.get_create_PlaylistInstance(name=name)

def createSongs():

    for i in range(1000):
        kwsong = {
            'uri': os.path.join(basePath, '%d.mp3' % i),
            'title': 'music%d%d' % (i, i),
            'author': 'yhm',
            'albumTitle': 'dxt'
        }

        kwsinger = {
            'name': kwsong['author']
        }

        kwalbum = {
            'title': kwsong['albumTitle']
        }

        kwfolder = {
            'path': os.path.dirname(kwsong['uri'])
        }

        singerInstance = dbWorker.get_create_SingerInstance(**kwsinger)
        albumInstance = dbWorker.get_create_AlbumInstance(**kwalbum)
        folderInstance = dbWorker.get_create_FolderInstance(**kwfolder)

        kwsong.update({
            'fsinger': singerInstance,
            'falbum': albumInstance,
            'ffolder': folderInstance
            })

        songInstance = dbWorker.get_create_SongInstance(**kwsong)


basePath = '/home/djf/workspace/github/musicplayer-qml/music'

createPlaylists()
# createSongs()

for i in range(10):
    uri = os.path.join(basePath, '%d.mp3' % i)
    dbWorker.addSongToPlaylist(uri)
    if i % 2 == 0:
        dbWorker.addSongToPlaylist(uri, name='favourite')

# for i in Song.select().order_by(Song.uri).paginate(1, 5).execute():
#     print i.uri


# for s in Song.select().join(Singer).where(Singer.name == "yhm"):
#     print s.uri
# print '+++++++++++++'

# for s in Song.select().join(Album).where(Album.title == "dxt"):
#     print s.uri
# print '+++++++++++++'

# for s in Song.select().join(Folder).where(Folder.path == "/home/djf/workspace/github/musicplayer-qml/music"):
#     print s.uri

print dbWorker.getSongsByPlaylistName('temporary')
print dbWorker.getPlaylistsBySongUri(os.path.join(basePath, '%d.mp3' % 2))