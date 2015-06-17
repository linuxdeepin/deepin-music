#!/usr/bin/python
# -*- coding: utf-8 -*-

import functools
from PyQt5.QtCore import QUrl
from PyQt5.QtGui import QDesktopServices

contexts = {}

def registerContext(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        self = args[0]
        if hasattr(self, '__contextName__'):
            contexts.update({self.__contextName__: self})
        func(*args, **kwargs)
    return wrapper

def registerObj(name, obj):
    contexts.update({name: obj})

def duration_to_string(value, default="00:00", i=1000):
    ''' convert duration to string. '''
    if not value: return default
    if (value / i) < 1: i = 1
    if value < 1000:
        return default
    else:
        duration = "%02d:%02d" % (value/(60*i), (value/i) % 60)
        if value/(60*i) / 60 >= 2:
            duration = "%03d:%02d" % (value/(60*i), (value/i) % 60)
        return duration


def openLocalUrl(url):
    QDesktopServices.openUrl(QUrl.fromLocalFile(url))
