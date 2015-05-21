#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect, QUrl,
    pyqtProperty, QObject,
    pyqtSlot, pyqtSignal,
    QThread)
from PyQt5.QtQuick import QQuickView
from .basewindow import BaseWindow
from controllers import registerContext, contexts, registerObj
from lrcwindow import LrcWindowManager

class MainWindow(BaseWindow):

    __contextName__ = 'MainWindow'

    @registerContext
    def __init__(self, engine=None, parent=None):
        super(MainWindow, self).__init__(engine, parent)

    def mousePressEvent(self, event):
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
