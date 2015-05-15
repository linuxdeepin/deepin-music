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


class FMoveableWidget(QLabel):

    def __init__(self, locked, parent=None):
        super(FMoveableWidget, self).__init__(parent)
        if locked:
            self.setAttribute(Qt.WA_TransparentForMouseEvents, True)
        else:
            self.setAttribute(Qt.WA_TransparentForMouseEvents, False)
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setAttribute(Qt.WA_Hover, True)
        self.setWindowFlags(Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)

    def moveCenter(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def mouseMoveEvent(self, event):
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                self.move(event.globalPos() - self.dragPosition)
                event.accept()
                self.setAttribute(Qt.WA_TranslucentBackground, False)

    def mousePressEvent(self, event):
        self.setFocus()
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - \
                self.frameGeometry().topLeft()
            event.accept()

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
            self.isSideClicked = False
            self.setCursor(Qt.ArrowCursor)

    def mouseMoveEvent(self, event):
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                self.move(
                    event.globalPos() - self.dragPosition)
                event.accept()


class DLrcWindow(FMoveableWidget):

    def __init__(self, locked=False, parent=None):
        super(DLrcWindow, self).__init__(locked, parent)
        self.installEventFilter(self)
        self.isBackgroundVisible = False
        self.resize(1000, 100)

        self.setMinimumSize(100, 80)

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

        self.button = QPushButton('l', self)
        self.button.move(100, 0)
        self.button.resize(40, 40)

        self.toolBarHeight = 60

        self.font = QFont()
        self.font.setPixelSize(30)
        self.setFont(self.font)

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
            self.startY = self.toolBarHeight + (self.height() - self.toolBarHeight - self.textHeight) / 2

            painter.setPen(QColor(0, 0, 0, 200))
            painter.drawText(self.startX + 1, self.startY + 1, self.textWidth, self.textHeight, Qt.AlignLeft, self.text)

            painter.setPen(QPen(self.linear_gradient, 0))
            painter.drawText(self.startX, self.startY, self.textWidth, self.textHeight, Qt.AlignLeft, self.text)

            painter.setPen(QPen(self.mask_linear_gradient, 0))
            painter.drawText(self.startX, self.startY, self.textWidth * self.percentage , self.textHeight, Qt.AlignLeft, self.text)

        if self.isBackgroundVisible:
            color = QColor('lightgray')
            color.setAlpha(30)
            painter.fillRect(0, self.startY, self.width(), self.textHeight, color)
            self.button.show()
        else:
            self.button.hide()

    def eventFilter(self, obj, event):
        if event.type() == QEvent.HoverEnter:
            self.isBackgroundVisible = True
        elif event.type() == QEvent.HoverLeave:
            self.isBackgroundVisible = False

        return super(FMoveableWidget, self).eventFilter(obj, event)
        


class LockWindow(QFrame):

    style = '''
    QPushButton#LockedButton{
        border-image: url(./skin/svg/lrc.desktop.lock.svg);
        border: none
    }

    QPushButton#LockedButton:pressed{
        border-image: url(./skin/svg/lrc.desktop.unlock.svg);
        border: none
    }

    '''

    def __init__(self):
        super(LockWindow, self).__init__()
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setAttribute(Qt.WA_Hover, True)
        self.setWindowFlags(Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setMouseTracking(True)
        self.installEventFilter(self)
        
        self.button = QPushButton(self)
        self.button.setObjectName('LockedButton')
        self.button.resize(32, 32)
        self.setStyleSheet(self.style)

    def eventFilter(self, obj, event):
        if event.type() == QEvent.HoverEnter:
            self.button.show()
        elif event.type() == QEvent.HoverLeave:
            self.button.hide()

        return super(LockWindow, self).eventFilter(obj, event)


class LrcWindowManager(QObject):

    def __init__(self):
        super(LrcWindowManager, self).__init__()
        self.unLockWindow = DLrcWindow(False)
        self.lockedWindow = DLrcWindow(True)
        self.lockButtonWindow = LockWindow()

        self.unLockWindow.button.clicked.connect(self.showLocked)
        self.lockButtonWindow.button.clicked.connect(self.showNoraml)

        self.text = ''
        self.percentage = 0
        self.state = 'Normal'

        screenHeight = QDesktopWidget().availableGeometry().height()
        self.lastPosition = QPoint(200, screenHeight - 100)
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
        pos = self.lockedWindow.pos()
        if pos == QPoint(0, 0):
            pos = self.lastPosition
        self.unLockWindow.move(pos)
        self.lockButtonWindow.move(pos)
        self.lockedWindow.hide()
        self.lockButtonWindow.hide()
        self.unLockWindow.show()
        self.state = 'Normal'

    def showLocked(self):
        pos = self.unLockWindow.pos()
        self.lockedWindow.move(pos)
        self.lockButtonWindow.move(pos)
        self.unLockWindow.hide()
        self.lockedWindow.show()

        self.lockButtonWindow.resize(self.lockedWindow.width(), 40)
        centerX = (self.lockedWindow.width() - self.lockButtonWindow.button.width()) / 2
        self.lockButtonWindow.button.move(QPoint(centerX, 8))
        self.lockButtonWindow.show()

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
        self.lockButtonWindow.hide()

        self.lastPosition = self.unLockWindow.pos()

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
