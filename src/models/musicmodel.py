#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import datetime
from peewee import *

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
    def updateRecord(cls, key, **kwargs):
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
            # print('%s is already in use' % kwargs['uri'])
            retId = cls.update(**kwargs).where(getattr(cls, key) == kwargs[key]).execute()
            if retId != 0:
                ret = cls.get(cls.id==retId)
            else:
                ret = None
        return ret


class Singer(BaseModel):
    name = CharField(default='', unique=True)
    area = CharField(default='')
    summary = CharField(default='')

    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'name'


class Album(BaseModel):
    title = CharField(default='', unique=True)
    artist = CharField(default='')

    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'title'
   


class Folder(BaseModel):
    path = CharField(default='', unique=True)

    created_date = DateTimeField(default=datetime.datetime.now)

    __key__ = 'path'
    

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

    __key__ = 'uri'

    @classmethod  
    def getSongByUri(cls, uri):
        try:
            songRecord = Song.get(Song.uri == uri)
        except DoesNotExist:
            songRecord = None
        return songRecord


class SongPlaylist(BaseModel):
    song = ForeignKeyField(Song)
    playlist = ForeignKeyField(Playlist)

    @classmethod
    def addSongToPlaylist(cls, uri, name='termpory'):


        songRecord = Song.getSongByUri(uri)
        playlistRecord = Playlist.getPlaylistByName(name)


        if songRecord and playlistRecord:
            kwargs = {
                'song': songRecord, 
                'playlist': playlistRecord
            }
            try:
                ret = cls.get(cls.song==songRecord, 
                    cls.playlist==playlistRecord)
                if ret:
                    # print('cls existed, Emit Singal')
                    pass
            except DoesNotExist:
                ret = cls.create(**kwargs)
        else:
            ret = None

        return ret

    @classmethod
    def getSongsByPlaylistName(cls, name):
        songs = []
        for song in Song.select().join(cls).join(Playlist).where(Playlist.name==name):
            songs.append(song.uri)

        return songs

    @classmethod
    def getPlaylistsBySongUri(cls, uri):
        playlists = []
        for playlist in Playlist.select().join(cls).join(Song).where(Song.uri==uri):
            playlists.append(playlist.name)
        return playlists


class DBWorker(object):

    def __init__(self):
        super(DBWorker, self).__init__()
        db.connect()
        db.create_tables([Song, Singer, Album, Folder, Playlist, SongPlaylist], safe=True)


dbWorker = DBWorker()


def createSongsTest(songs):
    for kwsong in songs:

        kwsinger = {
            'name': kwsong['author']
        }

        kwalbum = {
            'title': kwsong['albumTitle']
        }

        kwfolder = {
            'path': os.path.dirname(kwsong['uri'])
        }

        singerRecord = Singer.get_create_Record(**kwsinger)
        albumRecord = Album.get_create_Record(**kwalbum)
        folderRecord = Folder.get_create_Record(**kwfolder)

        kwsong.update({
            'fsinger': singerRecord,
            'falbum': albumRecord,
            'ffolder': folderRecord
            })

        songRecord = Song.get_create_Record(**kwsong)

def createPlaylistTest():
    names = ['termpory', 'favourite']
    for name in names:
        playlistRecord = Playlist.get_create_Record(name=name)


if __name__ == '__main__':
    basePath = '/home/djf/workspace/github/musicplayer-qml/music'
    songs = []
    for i in range(20):
        kwsong = {
            'uri': os.path.join(basePath, '%d.mp3' % i),
            'title': 'music%d%d' % (i, i),
            'author': 'yhm',
            'albumTitle': 'dxt'
        }

        songs.append(kwsong)


    # createSongsTest(songs)
    createPlaylistTest()

    for i in range(10):
        uri = os.path.join(basePath, '%d.mp3' % i)
        SongPlaylist.addSongToPlaylist(uri)
        if i % 2 == 0:
            SongPlaylist.addSongToPlaylist(uri, name='favourite')

    print Song.getRecord(**songs[0]).uri
    print Song.isRecordExisted(**songs[0])


    print SongPlaylist.getSongsByPlaylistName('termpory')
    print SongPlaylist.getPlaylistsBySongUri(os.path.join(basePath, '%d.mp3' % 2))
