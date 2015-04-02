#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *


class FSuspensionWidget(QFrame):

    '''
        Suspension window
    '''

    def __init__(self, filename=None, parent=None):
        super(FSuspensionWidget, self).__init__(parent)
        self.parent = parent
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setWindowFlags(Qt.ToolTip)
        self.initData()

        if filename:
            self.setBg(filename)

    def initData(self):
        self._bgPixmap = None
        self._contextMenu = None
        self._currentPos = QPoint(0, 0)

    def contextMenu(self):
        return self._contextMenu

    def setBg(self, filename):
        self._bgPixmap = QPixmap(filename)
        self.resize(self._bgPixmap.size())
        self.setMask(self._bgPixmap.mask())

    def setContextMenu(self, menu):
        self._contextMenu = menu

    def mousePressEvent(self, event):
        '''
        press left button in mouse to move window
        press right button in mouse to close contextMenu
        '''
        if event.button() == Qt.LeftButton:
            self._currentPos = event.globalPos() - \
                self.frameGeometry().topLeft()
            event.accept()
        elif event.button() == Qt.RightButton:
            if self._contextMenu:
                self._contextMenu.exec_(QCursor.pos())

    def mouseDoubleClickEvent(self, event):
        if self.parent:
            self.parent.setVisible(not self.parent.isVisible())

    def mouseMoveEvent(self, event):
        if event.buttons() & Qt.LeftButton:
            self.move(event.globalPos() - self._currentPos)
            event.accept()

    def paintEvent(self, event):
        if self._bgPixmap:
            _painter = QPainter(self)
            _painter.drawPixmap(0, 0, self._bgPixmap)

    def leaveEvent(self, event):
        self.setCursor(Qt.ArrowCursor)

    def enterEvent(self, event):
        self.setCursor(Qt.PointingHandCursor)
