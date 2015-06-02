#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect, QUrl,
    pyqtProperty, QObject,
    pyqtSlot, pyqtSignal,
    QThread, QPointF)
from PyQt5.QtQuick import QQuickView
from .basewindow import BaseWindow
from controllers import registerContext, contexts, registerObj
from lrcwindow import LrcWindowManager

class MainWindow(BaseWindow):

    __contextName__ = 'MainWindow'

    windowFocusChanged = pyqtSignal('QWindow*', arguments=['window'])
    mousePressed = pyqtSignal('QPointF', arguments=['point'])
    wheel = pyqtSignal('QPointF', arguments=['point'])

    @registerContext
    def __init__(self, engine=None, parent=None):
        super(MainWindow, self).__init__(engine, parent)
        self._initConnect()

    def _initConnect(self):
        from PyQt5.QtWidgets import qApp
        qApp.focusWindowChanged.connect(self.changeFocusWindow) 

    def mousePressEvent(self, event):
        self.mousePressed.emit(event.pos())
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            flag = contexts['WindowManageWorker'].windowMode
            if flag == "Full":
                x = self.quickItems['mainTitleBar'].x()
                y = self.quickItems['mainTitleBar'].y()
                width = self.quickItems['mainTitleBar'].width()
                height = self.quickItems['mainTitleBar'].height()
                rect = QRect(x, y, width, height)
                if rect.contains(event.pos()):
                    self.dragPosition = event.globalPos() - \
                        self.frameGeometry().topLeft()
            elif flag == "Simple":
                x = self.rootObject().x()
                y = self.rootObject().y()
                width = self.rootObject().width()
                height = 40
                rect = QRect(x, y, width, height)
                if rect.contains(event.pos()):
                    self.dragPosition = event.globalPos() - \
                        self.frameGeometry().topLeft()
        super(MainWindow, self).mousePressEvent(event)

    def changeFocusWindow(self, window):
        self.windowFocusChanged.emit(window)

    def wheelEvent(self, event):
        self.wheel.emit(QPointF(event.x(), event.y()))
        super(MainWindow, self).wheelEvent(event)
