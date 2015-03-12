#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot

from .utils import registerContext
from dwidgets import dthread
import threading

from log import logger


class Web360ApiWorker(QObject):

    addMediaContent = pyqtSignal('QVariant')

    __contextName__ = 'Web360ApiWorker'

    @registerContext
    def __init__(self, parent=None):
        super(Web360ApiWorker, self).__init__(parent)

    @classmethod
    def md5(cls, musicId):
        import hashlib
        s = 'id=%d_projectName=linuxdeepin' % (musicId)
        md5Value = hashlib.md5(s)
        return md5Value.hexdigest()

    @dthread
    @pyqtSlot(int)
    def getMusicURLByID(self, musicId):
        import hashlib
        import requests
        sign = self.md5(musicId)
        params = {
            'id': musicId,
            'src': 'linuxdeepin',
            'sign': sign
        }
        ret = requests.get("http://s.music.haosou.com/player/songForPartner", params=params)

        result = {
            'url': ret.url,
            'ret': ret.json()
        }
        self.addMediaContent.emit(result)

    def emitURL(self, jsonRet):
        if jsonRet and 'playlinkUrl' in jsonRet:
            logger.info(jsonRet)
            self.playUrl.emit(jsonRet)
        else:
            logger.info('Get URL Error' + repr(jsonRet))
