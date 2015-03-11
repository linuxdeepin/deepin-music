#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty, QUrl, QDate)
from PyQt5.QtGui import QCursor
from .utils import registerContext, contexts
from .utils import duration_to_string
from models import *
from log import logger

class DBWorker(QObject):

    def __init__(self):
        super(DBWorker, self).__init__()
        db.connect()
        db.create_tables([Song, Singer, Album, Folder, Playlist, SongPlaylist], safe=True)
