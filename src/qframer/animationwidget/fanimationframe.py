#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from qframer.qt.QtCore import *
from qframer.qt.QtGui import *
from ..fmainwindow import FMainWindow


class FAnimationFrame(QFrame):

    style = '''
        QFrame{
            background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(0, 82, 112, 255), stop:0.5 rgba(0, 211, 197, 255), stop:1 rgba(0, 82, 112, 255));
        }
    '''

    def __init__(self, parent=None):
        super(FAnimationFrame, self).__init__(parent)
        self.parent = parent
        self._initUI()
        self._initConnect()
        self._initAnimationRect()
        self._initShowAnimation()
        self._initHideAnimation()

    def _initUI(self):
        self.setStyleSheet(self.style)

    def _initConnect(self):
        if self.isMainWindow():
            if hasattr(self.parent, 'sizeChanged'):
                self.parent.sizeChanged.connect(self.resizeWithMainWindow)

    def isMainWindow(self):
        if self.parent and isinstance(self.parent, FMainWindow):
            return True
        else:
            return False

    def resizeWithMainWindow(self):
        if self.isMainWindow():
            height = self.parent._titleBar.height()
            rect = QRect(0, height, self.w, self.h)
            self.setGeometry(rect)

    def _initAnimationRect(self):
        if self.isMainWindow():
            height = self.parent._titleBar.height()
            self.startRect = QRect(self.w, height, 0, 0)
            self.endRect = QRect(0, height, self.w, self.h)

    @property
    def h(self):
        if self.isMainWindow():
            height = self.parent.height() - self.parent.titleBar().height() - \
                self.parent.statusBar().height()
            return height
        else:
            return None

    @property
    def w(self):
        if self.isMainWindow():
            width = self.parent.width()
            return width
        else:
            return None

    def _initShowAnimation(self):
        self.showanimation = QPropertyAnimation(self, 'geometry')
        self.showanimation.setStartValue(self.startRect)
        self.showanimation.setEndValue(self.endRect)
        self.showanimation.setDuration(200)
        self.showanimation.setEasingCurve(QEasingCurve.OutCubic)

    def _initHideAnimation(self):
        self.hideanimation = QPropertyAnimation(self, 'geometry')
        self.hideanimation.setStartValue(self.endRect)
        self.hideanimation.setEndValue(self.startRect)
        self.hideanimation.setDuration(200)
        self.hideanimation.setEasingCurve(QEasingCurve.OutCubic)
        self.hideanimation.finished.connect(self.hide)

    def animationShow(self):
        self.show()
        self.showanimation.start()

    def animationHide(self):
        self.hideanimation.start()

    def mouseDoubleClickEvent(self, event):
        self.animationHide()
        super(FAnimationFrame, self).mouseDoubleClickEvent(event)
