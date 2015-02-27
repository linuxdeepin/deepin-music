#!/usr/bin/python
# -*- coding: utf-8 -*-
from peewee import *
import datetime

db = SqliteDatabase('music.db', threadlocals=True)

class BaseModel(Model):
    class Meta:
        database = db


class Signer(BaseModel):
    name = CharField(default='')
    area = CharField(default='')
    summary = CharField(default='')

    # created_date = DateTimeField(default=datetime.datetime.now)


class Song(BaseModel):

    # signer = ForeignKeyField(Signer, related_name='signer')
    uri = CharField(default='')

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
    # created_date = DateTimeField(default=datetime.datetime.now)




db.connect()
db.create_tables([Song, Signer], safe=True)


songs = [{'uri' : "/home/%d.mp3" % i} for i in range(1, 10)]
signers = [{'name': 'a%d' % i} for i in range(1, 10)]
# print signers
for signer in signers:
    Signer.create(**signer)

for song in songs:
    Song.create(**song)



# charlie = User.create(username='charlie')
# huey = User(username='huey')
# huey.save()

# No need to set `is_published` or `created_date` since they
# will just use the default values we specified.
# Tweet.create(user=charlie, message='My first tweet')
# Tweet.create(user=charlie, message='My second tweet')

# # A simple query selecting a user.
# ret = User.get(User.username == 'charlie')
# print ret
# for t in ret.tweets:
#     print t.message
# # Get tweets created by one of several users. The "<<" operator
# # corresponds to the SQL "IN" operator.
# usernames = ['charlie', 'huey', 'mickey']
# users = User.select().where(User.username << usernames)
# tweets = Tweet.select().where(Tweet.user << users)
