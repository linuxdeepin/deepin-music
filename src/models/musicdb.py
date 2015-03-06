#!/usr/bin/python
# -*- coding: utf-8 -*-
import os

from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty)
from controllers import registerContext
from .qmodel import ModelMetaclass


class Song(object):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('Uri', str),

        ('Title', str),
        ('SubTitle', str),
        ('Author', str),
        ('Comment', str),
        ('Description', str),
        ('Category', str),
        ('Genre', str),
        ('Date', str),
        ('Year', str),
        ('UserRating', str),
        ('Keywords', str),
        ('Language', str),
        ('Publisher', str),
        ('Copyright', str),
        ('ParentalRating', str),
        ('RatingOrganization', str),

        ('Size', int),
        ('MediaType', str),
        ('Duration', int),

        ('AudioBitRate', int),
        ('AudioCodec', str),
        ('AverageLevel', int),
        ('ChannelCount', int),
        ('PeakValue', int),
        ('SampleRate', int),

        ('AlbumArtist', str),
        ('AlbumTitle', str),
        ('ContributingArtist', str),
        ('Composer', str),
        ('Conductor', str),
        ('Lyrics', str),
        ('Mood', str),
        ('TrackNumber', str),
        ('TrackCount', str),
        ('CoverArtUrlLarge', str),
        ('CoverArtUrlSmall', str)
    )

    def valid_model(self, value):
        return True

    def valid_brand(self, value):
        return False



class MusicDataBase(object):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('songs', dict),
        ('albums', dict),
        ('singers', dict),
        ('playlists', dict),
        ('folders', dict),
        ('test', list),
    )

    __contextName__ = "MusicDataBase"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        testuris = [
        os.sep.join([os.path.dirname(os.getcwd()), 'music', '1.mp3']),
        '/usr/share/deepin-sample-music/邓入比_我们的情歌.mp3',
        '/usr/share/deepin-sample-music/郭一凡_说走就走的旅行.mp3',
        '/usr/share/deepin-sample-music/胡彦斌_依然是你.mp3'
        ]

        for uri in testuris:
            song = Song()
            song.Uri = uri
            song.Title = os.path.basename(uri).split('.')[0]
            # self.songs.update({uri: song})
            self.test.append(song)
        # print self.test
