#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import re
import requests
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl)
from PyQt5.QtGui import QCursor, QDesktopServices
from .utils import registerContext
from config.constants import LRCPath

from deepin_utils.net import is_network_connected
from dwidgets.coverlrc.lrc_download import TTPlayer, DUOMI, SOSO, TTPod
from dwidgets.coverlrc.cover_query import poster
from dwidgets.dthreadutil import dthread


class LrcWorker(QObject):

    __contextName__ = 'LrcWorker'

    lrcFileExisted = pyqtSignal('QString')
    lrcDownloaded = pyqtSignal('QString', 'QString')

    @registerContext
    def __init__(self, parent=None):
        super(LrcWorker, self).__init__(parent)
        self._lrcDir = LRCPath

        self.initConnect()

    def initConnect(self):
        self.lrcDownloaded.connect(self.getLrc)

    def isLrcExisted(self , artist, title):
        path = self.getLrcPath(artist, title)
        if path:
            if os.path.exists(path):
                return True
            else:
                return False
        else:
            return False

    def getLrcPath(self, artist, title):
        ret = "%s-%s.lrc" % (artist, title)
        return os.path.join(self._lrcDir, ret)

    def vaild_lrc(self, filepath):
        try:
            fp = open(filepath, "r")
            lrc_content = fp.read()
            fp.close()
        except:    
            return False
        else:
            partial="".join( (i for i in lrc_content if (ord(i) < 128 and ord(i) != 0) ) )
            return bool(re.search('\[\d{1,}:\d{1,}.*?\]',partial))

    @dthread
    def getLrc(self, artist, title):
        if title:
            lrc_path = self.getLrcPath(artist, title)
            if lrc_path and os.path.exists(lrc_path):
                self.lrcFileExisted.emit(lrc_path)
            else:
                artist = artist.encode('utf-8')
                title = title.encode('utf-8')
                lrc_path = self.multiple_engine(lrc_path, artist, title)
                if lrc_path:
                    self.lrcFileExisted.emit(lrc_path)
                else:
                    self.lrcFileExisted.emit('')
        else:
            self.lrcFileExisted.emit('')

    def multiple_engine(self, lrc_path, artist, title):
        try:
            ret = False

            ting_result = poster.query_lrc_info(artist, title)
            if ting_result:
                urls = [item[2] for item in ting_result]
                for url in urls:
                    ret = self.downloadLRC(url, lrc_path)
                    if ret:
                        return lrc_path
            
            result = TTPlayer().request(artist, title)

            if result:
                urls = [item[2] for item in result]                
                for url in urls:
                    ret = self.downloadLRC(url, lrc_path)
                    if ret and self.vaild_lrc(lrc_path):
                        return lrc_path
                    
            ttpod_result = TTPod().request_data(artist, title)        
            if ttpod_result:
                with open(lrc_path, 'wb') as fp:
                    fp.write(ttpod_result)
                    return lrc_path
                        
            duomi_result = DUOMI().request(artist, title)
            if duomi_result:
                urls = [item[2] for item in duomi_result]                
                for url in urls:
                    ret = self.downloadLRC(url, lrc_path)
                    if ret and self.vaild_lrc(lrc_path):
                        return lrc_path
                        
            soso_result =  SOSO().request(artist, title)
            if soso_result:
                urls = [item[2] for item in soso_result]                
                for url in urls:
                    ret = self.downloadLRC(url, lrc_path)
                    if ret and self.vaild_lrc(lrc_path):
                        return lrc_path
            try:
                os.unlink(lrc_path)
            except:
                pass
                
            return None
                    
        except Exception, e:
            raise
            return None

    def downloadLRC(self, url, localUrl):
        try:
            r = requests.get(url)
            with open(localUrl, "wb") as f:
                f.write(r.content)
            return True
        except:
            return False
