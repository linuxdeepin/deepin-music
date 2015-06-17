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
from log import logger
from deepin_utils.net import is_network_connected
from dwidgets.coverlrc.lrc_download import TTPlayer, DUOMI, SOSO, TTPod
from dwidgets.coverlrc.cover_query import poster
from dwidgets.coverlrc.lrc_parser import LrcParser
from dwidgets.dthreadutil import dthread
from .signalmanager import signalManager


class LrcWorker(QObject):

    lrcFileExisted = pyqtSignal('QString')

    currentTextChanged = pyqtSignal('QString')

    __contextName__ = 'LrcWorker'

    @registerContext
    def __init__(self, parent=None):
        super(LrcWorker, self).__init__(parent)
        self._lrcDir = LRCPath
        self.lrcParser = LrcParser()
        self._currentText = ''
        self._lineMode = 1
        self.initConnect()

    def initConnect(self):
        self.lrcFileExisted.connect(self.parserLrc)

        signalManager.downloadLrc.connect(self.getLrc)
        signalManager.lineModeChanged.connect(self.setLrcLineMode)
        signalManager.lrcPositionChanged.connect(self.getLrcText)

    def parserLrc(self, filepath):
        self.lrcParser.set_filename(filepath)
        if self.lrcParser.scroll_lyrics:
            signalManager.updateLrc.emit()

    def setLrcLineMode(self, lineMode):
        self._lineMode = lineMode

    def getLrcText(self, pos, duration):
        ret = self.lrcParser.get_lyric_by_time(pos, duration)
        if ret:
            text, percentage, lyric_id = ret
            if self._lineMode == 1:
                signalManager.singleTextInfoChanged.emit(text, percentage, lyric_id)
            elif self._lineMode == 2:
                texts = []
                if lyric_id % 2 == 0:
                    texts.append((text, percentage, lyric_id))
                    text = self.getLyricTextById(lyric_id + 1)
                    texts.append((text, 0, lyric_id + 1))
                else:
                    texts.append((text, percentage, lyric_id))
                    text = self.getLyricTextById(lyric_id - 1)
                    texts.insert(0, (text, 1, lyric_id - 1))
                signalManager.douleTextInfoChanged.emit(texts)
            self.currentText = text

    @pyqtProperty('QString', notify=currentTextChanged)
    def currentText(self):
        return self._currentText

    @currentText.setter
    def currentText(self, text):
        self._currentText = text
        self.currentTextChanged.emit(text)

    @pyqtSlot(int, result='QString')
    def getLyricTextById(self, lyric_id):
        return self.lrcParser.get_item_lyric(lyric_id)

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
            return self.validLrcContent(lrc_content)

    def validLrcContent(self, content):
        partial="".join( (i for i in content if (ord(i) < 128 and ord(i) != 0) ) )
        return bool(re.search('\[\d{1,}:\d{1,}.*?\]',partial))

    @pyqtSlot('QString', 'QString', result=list)
    def getLrcContent(self, artist, title):
        lrcParser = LrcParser()
        if title:
            lrc_path = self.getLrcPath(artist, title)
            if lrc_path and os.path.exists(lrc_path):
                lrcParser.set_filename(lrc_path)
            else:
                artist = artist.encode('utf-8')
                title = title.encode('utf-8')
                lrc_path = self.multiple_engine(lrc_path, artist, title)
                if lrc_path:
                    lrcParser.set_filename(lrc_path)
        if hasattr(lrcParser, 'scroll_lyrics'):
            return lrcParser.scroll_lyrics
        else:
            return []

    @dthread
    def getLrc(self, artist, title):
        try:
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
                        signalManager.noLrcFound.emit()
        except Exception, e:
            logger.info(e)
        

    def multiple_engine(self, lrc_path, artist, title):
        try:
            engines = [self.ttPlayerEngine, self.ttPodEngine, self.tingEngine, self.sosoEngine, self.duomiEngine]
            for engine in engines:
                lrcPath = engine(lrc_path, artist, title)
                if lrcPath:
                    return lrcPath
            # signalManager.noLrcFound.emit()  
            try:
                os.unlink(lrc_path)
            except:
                pass
            return None
        except Exception, e:
            logger.info(e)
            return None

    def tingEngine(self, lrc_path, artist, title):
        ting_result = poster.query_lrc_info(artist, title)
        if ting_result:
            urls = [item[2] for item in ting_result]
            for url in urls:
                ret = self.downloadLRC(url, lrc_path)
                if ret:
                    return lrc_path
                else:
                    return None
        else:
            return None

    def ttPlayerEngine(self, lrc_path, artist, title):
        result = TTPlayer().request(artist, title)
        if result:
            urls = [item[2] for item in result]                
            for url in urls:
                ret = self.downloadLRC(url, lrc_path)
                if ret:
                    return lrc_path
                else:
                    return None
        else:
            return None

    def ttPodEngine(self, lrc_path, artist, title):
        ttpod_result = TTPod().request_data(artist, title)        
        if ttpod_result:
            if self.validLrcContent(ttpod_result):
                with open(lrc_path, 'wb') as fp:
                    fp.write(ttpod_result.encode('utf-8'))
                    return lrc_path
            else:
                return None

    def duomiEngine(self, lrc_path, artist, title):
        duomi_result = DUOMI().request(artist, title)
        if duomi_result:
            urls = [item[2] for item in duomi_result]                
            for url in urls:
                ret = self.downloadLRC(url, lrc_path)
                if ret:
                    return lrc_path
                else:
                    return None
        else:
            return None

    def sosoEngine(self, lrc_path, artist, title):
        soso_result =  SOSO().request(artist, title)
        if soso_result:
            urls = [item[2] for item in soso_result]
            for url in urls:
                ret = self.downloadLRC(url, lrc_path)
                if ret:
                    return lrc_path
                else:
                    return None
        else:
            return None

    def downloadLRC(self, url, localUrl):
        try:
            r = requests.get(url)
            lrcContent = r.content
            if self.validLrcContent(lrcContent):
                with open(localUrl, "wb") as f:
                    f.write(lrcContent)
                return True
            else:
                return False
        except:
            return False


lrcWorker = LrcWorker()
