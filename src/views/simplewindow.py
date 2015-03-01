#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect, QUrl,
    pyqtProperty, QObject, 
    pyqtSlot, pyqtSignal, 
    QThread)
from .basewindow import BaseWindow
from controllers import registerContext


class SimpleWindow(BaseWindow):

    __contextName__ = 'SimpleWindow'

    @registerContext
    def __init__(self):
        super(SimpleWindow, self).__init__()
        self.setFlags(Qt.FramelessWindowHint)

    def mousePressEvent(self, event):
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - self.frameGeometry().topLeft()
            event.accept()
        super(SimpleWindow, self).mousePressEvent(event)
