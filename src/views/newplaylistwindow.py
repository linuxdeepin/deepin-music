#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect, QUrl,
    pyqtProperty, QObject,
    pyqtSlot, pyqtSignal,
    QThread)
from PyQt5.QtGui import QRegion, QIcon
from PyQt5.QtWidgets import QDesktopWidget
from PyQt5.QtQuick import QQuickView
from .basewindow import BaseWindow
from dwidgets import DQuickWidget
from controllers import registerContext, contexts, registerObj
from lrcwindow import LrcWindowManager
from deepin_utils.file import get_parent_dir
from controllers.signalmanager import signalManager
import config

# class NewPlaylistWindow(BaseWindow):

#     __contextName__ = 'NewPlaylistWindow'

#     @registerContext
#     def __init__(self, engine=None, parent=None):
#         super(NewPlaylistWindow, self).__init__(engine, parent)
#         self.setFlags(Qt.Popup)
#         self.setSource(QUrl.fromLocalFile(
#             os.path.join(get_parent_dir(__file__, 2), 'views','dialogs' ,'NewPlaylist.qml')))
#         signalManager.newPlaylistWindowShowed.connect(self.show)
#         self.loadSuccessed.connect(self.moveCenter)

#     # def moveCenter(self):
#     #     # qr = self.frameGeometry()
#     #     # print qr
#     #     # cp = self.parent().frameGeometry().center()
#     #     # qr.moveCenter(cp)
#     #     # print qr, 'moved'
#     #     qr = QRect(-110, 0, 300, 100)
#     #     self.setPosition(qr.topLeft())

#     def mouseMoveEvent(self, event):
#         if hasattr(self, "dragPosition"):
#             if event.buttons() == Qt.LeftButton:
#                 self.setPosition(event.globalPos() - self.dragPosition)
#         super(NewPlaylistWindow, self).mouseMoveEvent(event)

#     def mousePressEvent(self, event):
#         # 鼠标点击事件
#         if event.button() == Qt.LeftButton:
#             self.dragPosition = event.globalPos() - \
#                 self.frameGeometry().topLeft()
#         super(NewPlaylistWindow, self).mousePressEvent(event)

#     def mouseReleaseEvent(self, event):
#         # 鼠标释放事件
#         if hasattr(self, "dragPosition"):
#             del self.dragPosition
#         super(NewPlaylistWindow, self).mousePressEvent(event)


class NewPlaylistWindow(DQuickWidget):

    __contextName__ = 'NewPlaylistWindow'

    @registerContext
    def __init__(self, engine=None, parent=None):
        super(NewPlaylistWindow, self).__init__(engine)
        self.setWindowFlags(Qt.Window | Qt.FramelessWindowHint)
        self.setWindowModality(Qt.ApplicationModal)
        self.setWindowIcon(QIcon(config.windowIcon))

        signalManager.newPlaylistDialogShowed.connect(self.showPlaylistDialog)
        signalManager.newMultiPlaylistDialogShowed.connect(self.showMutiPlaylistDialog)
        signalManager.addMutiPlaylistFlags.connect(self.addMutiPlaylist)
        signalManager.dialogClosed.connect(self.close)

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

    def addMutiPlaylist(self, flags):
        signalManager.addSongsToMultiPlaylist.emit(self._id, self._type, flags)

    def mouseMoveEvent(self, event):
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                # rect = QRect(0, 0, self.width(), 25)
                # if rect.contains(event.pos()):
                self.move(event.globalPos() - self.dragPosition)
        super(NewPlaylistWindow, self).mouseMoveEvent(event)

    def mousePressEvent(self, event):
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - \
                self.frameGeometry().topLeft()
        super(NewPlaylistWindow, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
        super(NewPlaylistWindow, self).mouseReleaseEvent(event)

    def moveCenter(self):
        from PyQt5.QtWidgets import qApp
        qr = self.frameGeometry()
        cp =  qApp.desktop().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())
