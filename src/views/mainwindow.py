#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect,QUrl,
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
                x = self.quickItems['leftSideBar'].width()
                y = self.quickItems['mainTitleBar'].height()
                width = x + self.quickItems['webEngineViewPage'].width()
                height = y + self.quickItems['webEngineViewPage'].height()
                rect = QRect(x, y , width, height)
                if not rect.contains(event.pos()):
                    self.dragPosition = event.globalPos() - self.frameGeometry().topLeft()
                    event.accept()
            elif flag == "Simple":
                x = self.quickItems['simpleCenterWindow'].x()
                y = self.quickItems['simpleCenterWindow'].y()
                width = self.quickItems['simpleCenterWindow'].width()
                height = self.quickItems['simpleCenterWindow'].height()
                rect = QRect(x, y , width, height)
                if not rect.contains(event.pos()):
                    self.dragPosition = event.globalPos() - self.frameGeometry().topLeft()
                    event.accept()

        super(MainWindow, self).mousePressEvent(event)
