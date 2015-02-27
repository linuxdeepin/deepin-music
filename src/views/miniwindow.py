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


class MiniWindow(BaseWindow):

    __contextName__ = 'MiniWindow'

    @registerContext
    def __init__(self):
        super(MiniWindow, self).__init__()

    def mousePressEvent(self, event):
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - self.frameGeometry().topLeft()
            event.accept()
        super(MiniWindow, self).mousePressEvent(event)