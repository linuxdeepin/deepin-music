#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
from PyQt5.QtCore import (Qt, QPoint, QSize,
    pyqtSignal, pyqtProperty)
from PyQt5.QtWidgets import QFrame


class FMoveableWidget(QFrame):

    qSizeChanged = pyqtSignal('QSize')
    qPositionChanged = pyqtSignal('QPoint')

    def __init__(self, parent=None):
        super(FMoveableWidget, self).__init__(parent)
        self.setWindowFlags(Qt.FramelessWindowHint)

    @pyqtProperty('QPoint', notify=qPositionChanged)
    def qPosition(self):
        return self.pos()

    @qPosition.setter
    def qPosition(self, pos):
        self.move(pos)
        self.qPositionChanged.emit(pos)

    def setPosition(self, pos):
        self.qPosition = pos

    @pyqtProperty('QSize', notify=qSizeChanged)
    def qSize(self):
        return self.size()

    @qSize.setter
    def qSize(self, size):
        self.setFixedSize(size)
        self.qSizeChanged.emit(size)

    def moveCenter(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def mouseMoveEvent(self, event):
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                pos = event.globalPos() - self.dragPosition
                self.qPosition = pos
                self.setAttribute(Qt.WA_TranslucentBackground, False)

    def mousePressEvent(self, event):
        self.setFocus()
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - \
                self.frameGeometry().topLeft()
        super(FMoveableWidget, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
            self.isSideClicked = False
            self.setCursor(Qt.ArrowCursor)
        super(FMoveableWidget, self).mousePressEvent(event)

    def resizeEvent(self, event):
        self.qSizeChanged.emit(event.size())
        super(FMoveableWidget, self).resizeEvent(event)



if __name__ == '__main__':
    from PyQt5.QtWidgets import QApplication
    app = QApplication(sys.argv)
    
    lrcManager = FMoveableWidget()
    lrcManager.show()

    exitCode = app.exec_()

    sys.exit(exitCode)
