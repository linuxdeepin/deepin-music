#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
from PyQt5.QtCore import (
    Qt, QRect,QUrl,
    pyqtProperty, QObject, 
    pyqtSlot, pyqtSignal, 
    QTimer, QEvent, QPoint)
from PyQt5.QtGui import QColor, QPen, QLinearGradient, QPainter, QFont, QPalette, QRegion
from PyQt5.QtWidgets import QApplication, QLabel, QFrame, QPushButton, QDesktopWidget
from controllers import registerContext, contexts, registerObj
from controllers import signalManager

class FMoveableWidget(QLabel):

    qSizeChanged = pyqtSignal('QSize')
    qPositionChanged = pyqtSignal('QPoint')

    def __init__(self, locked, parent=None):
        super(FMoveableWidget, self).__init__(parent)
        if locked:
            self.setAttribute(Qt.WA_TransparentForMouseEvents, True)
        else:
            self.setAttribute(Qt.WA_TransparentForMouseEvents, False)
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setAttribute(Qt.WA_Hover, True)
        self.setWindowFlags(Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)

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
        self.resize(size)
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


class DLrcWindow(FMoveableWidget):

    hoverChanged = pyqtSignal(bool)

    def __init__(self, locked=False, parent=None):
        super(DLrcWindow, self).__init__(locked, parent)
        self.installEventFilter(self)
        self._hovered = False
        self.resize(901, 40)

        self.linear_gradient = QLinearGradient()
        self.linear_gradient.setStart(0, 10)
        self.linear_gradient.setFinalStop(0, 40)
    
        self.linear_gradient.setColorAt(0.1, QColor(14, 179, 255));
        self.linear_gradient.setColorAt(0.5, QColor(114, 232, 255));
        self.linear_gradient.setColorAt(0.9, QColor(14, 179, 255));

        self.mask_linear_gradient = QLinearGradient()
        self.mask_linear_gradient.setStart(0, 10)
        self.mask_linear_gradient.setFinalStop(0, 40)
        self.mask_linear_gradient.setColorAt(0.1, QColor(0, 0, 40))
        self.mask_linear_gradient.setColorAt(0.5, QColor(255, 72, 16))
        self.mask_linear_gradient.setColorAt(0.9, QColor(0, 255, 40))

        self.text = ""
        self.percentage = 0

        self.font = QFont()
        self.font.setPixelSize(30)
        self.setFont(self.font)

    @pyqtProperty(bool, notify=hoverChanged)
    def hovered(self):
        return  self._hovered

    @hovered.setter
    def hovered(self, hovered):
        self._hovered = hovered
        self.hoverChanged.emit(hovered)

    @pyqtSlot('QPoint')
    def move(self, pos):
        super(DLrcWindow, self).move(pos)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setFont(self.font)
        if self.text:
            self.textWidth = self.fontMetrics().width(self.text)
            self.textHeight = self.fontMetrics().height()
            self.startX = (self.width() - self.textWidth) / 2
            self.startY = (self.height() - self.textHeight) / 2

            painter.setPen(QColor(0, 0, 0, 200))
            painter.drawText(self.startX + 1, self.startY + 1, self.textWidth, self.textHeight, Qt.AlignLeft, self.text)

            painter.setPen(QPen(self.linear_gradient, 0))
            painter.drawText(self.startX, self.startY, self.textWidth, self.textHeight, Qt.AlignLeft, self.text)

            painter.setPen(QPen(self.mask_linear_gradient, 0))
            painter.drawText(self.startX, self.startY, self.textWidth * self.percentage , self.textHeight, Qt.AlignLeft, self.text)

            if self.hovered:
                color = QColor('lightgray')
                color.setAlpha(30)
                painter.fillRect(0, 0, self.width(), self.height(), color)
            else:
                pass

    def eventFilter(self, obj, event):
        if event.type() == QEvent.HoverEnter:
            self.hovered = True
        elif event.type() == QEvent.HoverLeave:
            self.hovered = False
        return super(FMoveableWidget, self).eventFilter(obj, event)


class LrcWindowManager(QObject):

    def __init__(self):
        super(LrcWindowManager, self).__init__()
        self.unLockWindow = DLrcWindow(False)
        self.lockedWindow = DLrcWindow(True)

        self.unLockWindow.qPositionChanged.connect(self.lockedWindow.setPosition)
        signalManager.locked.connect(self.showLocked)
        signalManager.unLocked.connect(self.showNoraml)

        self.text = ''
        self.percentage = 0
        self.state = 'Normal'

        screenHeight = QDesktopWidget().availableGeometry().height()
        self.isVisible = False

    def updateTextInfo(self, text, percentage, lyric_id):
        self.text = text
        self.percentage = percentage
        self.lyric_id = lyric_id

        self.unLockWindow.percentage = self.percentage
        self.lockedWindow.percentage = self.percentage
        self.unLockWindow.text = self.text
        self.lockedWindow.text = self.text
        self.unLockWindow.update()
        self.lockedWindow.update()

    def showNoraml(self):
        self.lockedWindow.hide()
        self.unLockWindow.show()
        self.state = 'Normal'

    def showLocked(self):
        self.unLockWindow.hide()
        self.lockedWindow.show()
        self.state = 'Locked'

    def show(self):
        self.isVisible = True
        if self.state == 'Normal':
            self.showNoraml()
        elif self.state == 'Locked':
            self.showLocked()

    def hide(self):
        self.isVisible = False
        self.unLockWindow.hide()
        self.lockedWindow.hide()

    def toggle(self):
        if self.isVisible:
            self.hide()
        else:
            self.show()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    lrcManager = LrcWindowManager()
    lrcManager.toggle()

    exitCode = app.exec_()

    sys.exit(exitCode)
