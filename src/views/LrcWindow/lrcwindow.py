#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import Qt, QTimer, QEvent
from PyQt5.QtGui import QColor, QPen, QLinearGradient, QPainter, QFont, QPalette
from PyQt5.QtWidgets import QApplication, QLabel, QFrame, QPushButton
from PyQt5.QtQuickWidgets import QQuickWidget


class FMoveableWidget(QLabel):

    def __init__(self, parent=None):
        super(FMoveableWidget, self).__init__(parent)
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setAttribute(Qt.WA_Hover, True)
        self.setWindowFlags(Qt.SubWindow | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)

        self.installEventFilter(self)
        self.boderFlag = False

        self.isSideClicked = False
        self.isCusorLeftSide = False
        self.isCusorRightSide = False
        self.isCusorDownSide = False

    def moveCenter(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

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

        self.xPos = self.x()
        self.yPos = self.y()
        self.rdragx = event.x()
        self.rdragy = event.y()
        self.currentWidth = self.width()
        self.currentHeight = self.height()
        self.isSideClicked = True

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
            self.isSideClicked = False
            self.setCursor(Qt.ArrowCursor)

    def mouseMoveEvent(self, event):
        self.oldPosition = self.pos()
        if self.isSideClicked and self.isCusorRightSide:
            w = max(self.minimumWidth(),
                    self.currentWidth + event.x() - self.rdragx)
            h = self.currentHeight
            self.resize(w, h)
        elif self.isSideClicked and self.isCusorDownSide:
            w = self.currentWidth
            h = max(self.minimumHeight(),
                    self.currentHeight + event.y() - self.rdragy)
            self.resize(w, h)
        elif self.isSideClicked and self.isCusorLeftSide:
            x = event.x() + self.xPos - self.rdragx
            w = max(self.minimumWidth(),
                    self.xPos + self.currentWidth - x)
            self.move(x , self.yPos)
            self.resize(w, self.currentHeight)
        else:
            # 鼠标移动事件
            if self.isMaximized():
                event.ignore()
            else:
                if hasattr(self, "dragPosition"):
                    if event.buttons() == Qt.LeftButton:
                        self.move(
                            event.globalPos() - self.dragPosition)
                        event.accept()

    def eventFilter(self, obj, event):
        if event.type() == QEvent.HoverMove:
            if self.width() - event.pos().x() < 5:
                self.setCursor(Qt.SizeHorCursor)
                self.isCusorRightSide = True
            elif self.height() - event.pos().y() < 5:
                self.setCursor(Qt.SizeVerCursor)
                self.isCusorDownSide = True
            elif event.pos().x() < 5:
                self.setCursor(Qt.SizeHorCursor)
                self.isCusorLeftSide = True
            elif not self.isSideClicked:
                self.setCursor(Qt.ArrowCursor)
                self.isCusorLeftSide = False
                self.isCusorRightSide = False
                self.isCusorDownSide = False
        elif event.type() == QEvent.HoverEnter:
            self.boderFlag = True
        elif event.type() == QEvent.HoverLeave:
            self.boderFlag = False

        return super(FMoveableWidget, self).eventFilter(obj, event)


class DQuickWidget(FMoveableWidget):

    def __init__(self):
        super(DQuickWidget, self).__init__()
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
        self.mask_linear_gradient.setColorAt(0.1, QColor(222, 54, 4))
        self.mask_linear_gradient.setColorAt(0.5, QColor(255, 72, 16))
        self.mask_linear_gradient.setColorAt(0.9, QColor(222, 54, 4))

        self.text = "PyQt 5.2版本最新发布了,Qt 库的Python绑定"

        self.font = QFont()
        self.font.setFamily("Times New Roman")
        self.font.setBold(True)
        self.font.setPixelSize(30)

        self.setFont(self.font)

        self.timer = QTimer(self)
        self.timer.setInterval(100)
        self.timer.timeout.connect(self.update)
        self.timer.start()

        self.textWidth = self.fontMetrics().width(self.text)
        self.textHeight = self.fontMetrics().height()

        self.p = 0.1

        self.button = QPushButton('1111', self)
        self.button.move(100, 0)
        self.button.resize(40, 40)

        self.toolBarHeight = 50


    def paintEvent(self, event):

        self.startX = (self.width() - self.textWidth) / 2
        self.startY = self.toolBarHeight + (self.height() - self.toolBarHeight - self.textHeight) / 2

        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # pixelSize = self.font.pixelSize()
        # pixelSize += 1
        # self.font.setPixelSize(pixelSize)
        # self.setFont(self.font)
        # self.textHeight = self.fontMetrics().height()
        # spacing = (self.height() - self.toolBarHeight - self.textHeight) / 2
        # # if spacing > 0:

        painter.setFont(self.font)

        if self.boderFlag:
            color = QColor('lightgray')
            color.setAlpha(30)
            painter.fillRect(0, self.toolBarHeight, self.width(), self.height() - self.toolBarHeight, color)

        painter.setPen(QColor(0, 0, 0, 200))
        painter.drawText(self.startX + 1, self.startY + 1, self.textWidth, self.textHeight, Qt.AlignLeft, self.text)

        painter.setPen(QPen(self.linear_gradient, 0))
        painter.drawText(self.startX, self.startY, self.textWidth, self.textHeight, Qt.AlignLeft, self.text)

        painter.setPen(QPen(self.mask_linear_gradient, 0))
        painter.drawText(self.startX, self.startY, self.textWidth * self.p , self.textHeight, Qt.AlignLeft, self.text)

        self.p += 0.01

        if self.p >= 1:
            self.p = 0

        self.font.setPixelSize(30)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    main = DQuickWidget()
    main.show()

    exitCode = app.exec_()
    
    sys.exit(exitCode)
