#!/usr/bin/python
# -*- coding: utf-8 -*-
from .qmodel import ModelMetaclass
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty)
from controllers import registerContext



class TestDB(QObject):

    __metaclass__ = ModelMetaclass
    
    __Fields__ = (
        ('model', str, "123"),
        ('brand', str, "456"),
        ('year', int),
        ('inStock', bool),
        ('d', dict, {'a': 1111}),
        ('l', list, [1, 2 ,3])
    )

    __contextName__ = "TestDB"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        self.valid_model(3)

    def valid_model(self, value):
        return True

    def valid_brand(self, value):
        return False



class TestQObject(QObject):

    __contextName__ = "TestQObject"

    @registerContext
    def __init__(self):
        super(QObject, self).__init__()

    @pyqtProperty('QString')
    def ms(self):
        return '69874'

    @pyqtProperty('QVariant')
    def ml(self):
        return [1, 2 ,3]

    @pyqtProperty('QVariant')
    def md(self):
        return {'a': 1, 'b': 2}

testObject = TestQObject()

'''

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
'''