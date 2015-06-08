#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect, QUrl,
    pyqtProperty, QObject,
    pyqtSlot, pyqtSignal,
    QThread)
from PyQt5.QtGui import QRegion, QIcon, QSurfaceFormat, QColor
from PyQt5.QtWidgets import QDesktopWidget
from PyQt5.QtQuick import QQuickView
from .basewindow import BaseWindow
from dwidgets import DQuickWidget, DQuickView
from controllers import registerContext, contexts, registerObj
from lrcwindow import LrcWindowManager
from deepin_utils.file import get_parent_dir
from controllers.signalmanager import signalManager
from controllers import musicManageWorker, coverWorker, lrcWorker
import config


class QmlDialog(DQuickView):

    songObjChanged = pyqtSignal('QVariant')
    __contextName__ = 'QmlDialog'

    @registerContext
    def __init__(self, engine=None, parent=None):
        super(QmlDialog, self).__init__(engine, parent)

        self.setFlags(Qt.Tool | Qt.Dialog |Qt.FramelessWindowHint)
        self.setModality(Qt.ApplicationModal)
        self.setIcon(QIcon(config.windowIcon))

        self._songObj = None
        self.moveRect = QRect(0, 0, 300, 150)

        signalManager.newPlaylistDialogShowed.connect(self.showPlaylistDialog)
        signalManager.newMultiPlaylistDialogShowed.connect(self.showMutiPlaylistDialog)
        signalManager.informationShow.connect(self.showInformationDialog)
        signalManager.addMutiPlaylistFlags.connect(self.addMutiPlaylist)
        signalManager.dialogClosed.connect(self.close)

    @pyqtProperty('QVariant', notify=songObjChanged)
    def songObj(self):
        return self._songObj

    @songObj.setter
    def songObj(self, value):
        self._songObj = value
        self.songObjChanged.emit(value) 

    def showPlaylistDialog(self):
        self.setSource(QUrl.fromLocalFile(
            os.path.join(get_parent_dir(__file__, 2), 'views','dialogs' ,'NewPlaylistDialog.qml')))
        self.moveCenter()
        self.show()

    def showMutiPlaylistDialog(self, _id, _type):
        self._id = _id
        self._type = _type
        self.setSource(QUrl.fromLocalFile(
            os.path.join(get_parent_dir(__file__, 2), 'views','dialogs' ,'MutiPlaylistChoseDialog.qml')))
        self.moveCenter()
        self.show()

    def showInformationDialog(self, _url):
        self._url = _url
        self.songObj = musicManageWorker.getSongObjByUrl(self._url).getDict()
        self.songObj['cover'] = coverWorker.getCover(self.songObj['title'], self.songObj['artist'], self.songObj['album'])
        content = lrcWorker.getLrcContent(self.songObj['title'], self.songObj['artist'])
        self.songObj['lyric'] = '\n'.join(content)
        self.setSource(QUrl.fromLocalFile(
            os.path.join(get_parent_dir(__file__, 2), 'views','dialogs' ,'InformationDialog.qml')))
        self.moveCenter()
        self.show()

    def addMutiPlaylist(self, flags):
        signalManager.addSongsToMultiPlaylist.emit(self._id, self._type, flags)

    def mouseMoveEvent(self, event):
        if hasattr(self, "dragPosition"):
            # if event.buttons() == Qt.LeftButton:
                # if self.moveRect.contains(event.pos()):
            self.setPosition(event.globalPos() - self.dragPosition)
        super(QmlDialog, self).mouseMoveEvent(event)

    def mousePressEvent(self, event):
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - \
                self.frameGeometry().topLeft()
        super(QmlDialog, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
        super(QmlDialog, self).mouseReleaseEvent(event)

    def moveCenter(self):
        from PyQt5.QtWidgets import qApp
        qr = self.frameGeometry()
        cp =  qApp.desktop().availableGeometry().center()
        qr.moveCenter(cp)
        self.setPosition(qr.topLeft())
