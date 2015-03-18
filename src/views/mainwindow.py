#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect, QUrl,
    pyqtProperty, QObject,
    pyqtSlot, pyqtSignal,
    QThread)
from .basewindow import BaseWindow
from controllers import registerContext, contexts


class MainWindow(BaseWindow):

    __contextName__ = 'MainWindow'

    @registerContext
    def __init__(self):
        super(MainWindow, self).__init__()

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
                    event.accept()
            elif flag == "Simple":
                x = self.quickItems['simpletitleBar'].x()
                y = self.quickItems['simpletitleBar'].y()
                width = self.quickItems['simpletitleBar'].width()
                height = self.quickItems['simpletitleBar'].height()
                rect = QRect(x, y, width, height)
                if rect.contains(event.pos()):
                    self.dragPosition = event.globalPos() - \
                        self.frameGeometry().topLeft()
                    event.accept()

            super(MainWindow, self).mousePressEvent(event)
        elif event.button() == Qt.RightButton:
            flag = contexts['WindowManageWorker'].windowMode
            if flag == "Full":
                if self.quickItems['webEngineViewPage'].isVisible():
                    event.ignore()
                else:
                    event.accept()
                    super(MainWindow, self).mousePressEvent(event)
