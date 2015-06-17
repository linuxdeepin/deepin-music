#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *
from .fmoveablewidget import FMoveableWidget
from .ftitlebar import BaseToolButton, baseHeight


class FTitleBar(QFrame):

    settingMenuShowed = Signal()
    skinMenuShowed = Signal()
    modeed = Signal(bool)
    locked = Signal(bool)
    pined = Signal(bool)
    closed = Signal()

    closestyle = '''
        QToolButton#close{
            background-color: transparent;
            color: white;
        }

        QToolButton#close:hover{
            background-color: red;
            border: 1px;
        }
    '''

    logostyle = '''
        QToolButton#logo{
            background-color: transparent;
        }
        QToolButton#logo:hover{
            background-color: transparent;
        }
    '''

    def __init__(self, parent=None):
        super(FTitleBar, self).__init__(parent)
        self.initData()
        self.initUI()

    def initData(self):
        self.settingDownIcon = QIcon(":/icons/dark/appbar.control.down.png")
        self.clothesIcon = QIcon(":/icons/dark/appbar.clothes.shirt.png")
        self.lockIcon = QIcon(":/icons/dark/appbar.lock.png")
        self.unlockIcon = QIcon(":/icons/dark/appbar.unlock.keyhole.png")
        self.pinIcon = QIcon(":/icons/dark/appbar.pin.png")
        self.unPinIcon = QIcon(":/icons/dark/appbar.pin.remove.png")
        self.closeIcon = QIcon(":/icons/dark/appbar.close.png")

        self.max_flag = False
        self.lock_flag = False
        self.pin_flag = False

    def initUI(self):
        self.setFixedHeight(baseHeight)

        self.lockButton = BaseToolButton()
        self.lockButton.setIcon(self.unlockIcon)

        self.pinButton = BaseToolButton()
        self.pinButton.setIcon(self.unPinIcon)

        self.settingDownButton = BaseToolButton()
        self.settingDownButton.setIcon(self.settingDownIcon)

        self.closeButton = BaseToolButton()
        self.closeButton.setObjectName("close")
        self.closeButton.setStyleSheet(self.closestyle)
        self.closeButton.setIcon(self.closeIcon)

        mainLayout = QHBoxLayout()
        mainLayout.addStretch()
        mainLayout.addWidget(self.settingDownButton)
        mainLayout.addWidget(self.pinButton)
        mainLayout.addWidget(self.lockButton)
        mainLayout.addWidget(self.closeButton)
        mainLayout.setContentsMargins(0, 0, 5, 0)
        mainLayout.setSpacing(0)
        self.setLayout(mainLayout)

        self.settingDownButton.clicked.connect(self.settingMenuShowed)
        self.lockButton.clicked.connect(self.swithLockIcon)
        self.pinButton.clicked.connect(self.swithPinIcon)
        self.closeButton.clicked.connect(self.closed)

    def swithLockIcon(self):
        if self.lock_flag:
            self.lockButton.setIcon(self.unlockIcon)
        else:
            self.lockButton.setIcon(self.lockIcon)
        self.lock_flag = not self.lock_flag
        self.locked.emit(self.lock_flag)

    def swithPinIcon(self):
        if self.pin_flag:
            self.pinButton.setIcon(self.unPinIcon)
        else:
            self.pinButton.setIcon(self.pinIcon)
        self.pin_flag = not self.pin_flag
        self.pined.emit(self.pin_flag)

    def isPined(self):
        return self.pin_flag

    def isLocked(self):
        return self.lock_flag

    def isMax(self):
        return self.max_flag


class FFloatWidget(FMoveableWidget):

    default_width = 300

    def __init__(self, parent=None):
        super(FFloatWidget, self).__init__()
        self.parent = parent
        self.setWindowFlags(
            Qt.WindowType_Mask | Qt.SubWindow | Qt.FramelessWindowHint)

        self._initShowAnimation()
        self._initHideAnimation()
        self._initUI()
        self._initConnect()

    def _initUI(self):
        self.setFixedWidth(self.default_width)
        self.titleBar = FTitleBar(self)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.titleBar)
        mainLayout.addStretch()
        mainLayout.setSpacing(0)
        mainLayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(mainLayout)
        self.setGeometry(self.endRect)

    def _initConnect(self):
        self.titleBar.closed.connect(self.animationHide)
        self.titleBar.pined.connect(self.setFlags)

    @property
    def startRect(self):
        mainwindow = self.parent
        startRect = QRect(mainwindow.x() + mainwindow.width(),
                          mainwindow.y() + mainwindow.titleBar().height(),
                          self.w, mainwindow.height() -
                          mainwindow.titleBar().height())
        return startRect

    @property
    def endRect(self):
        mainwindow = self.parent
        endRect = QRect(mainwindow.x() + mainwindow.width(),
                        mainwindow.y() + mainwindow.titleBar().height(),
                        self.w, mainwindow.height() -
                        mainwindow.titleBar().height())
        return endRect

    @property
    def h(self):
        return self.height()

    @property
    def w(self):
        return self.width()

    def _initShowAnimation(self):
        self.showanimation = QPropertyAnimation(self, 'windowOpacity')
        self.showanimation.setStartValue(0)
        self.showanimation.setEndValue(1)
        self.showanimation.setDuration(1000)
        self.showanimation.setEasingCurve(QEasingCurve.OutCubic)

    def _initHideAnimation(self):
        self.hideanimation = QPropertyAnimation(self, 'windowOpacity')
        self.hideanimation.setStartValue(1)
        self.hideanimation.setEndValue(0)
        self.hideanimation.setDuration(1000)
        self.hideanimation.setEasingCurve(QEasingCurve.OutCubic)
        self.hideanimation.finished.connect(self.hide)

    def animationShow(self):
        self.show()
        self.showanimation.start()

    def animationHide(self):
        self.hideanimation.start()

    def setFlags(self, flag):
        if flag:
            self.setWindowFlags(
                Qt.WindowType_Mask | Qt.SubWindow |
                Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint)
            self.show()
        else:
            self.setWindowFlags(
                Qt.WindowType_Mask | Qt.SubWindow | Qt.FramelessWindowHint)
            self.show()

    def mouseMoveEvent(self, event):
        if self.isLocked():
            pass
        else:
            super(FFloatWidget, self).mouseMoveEvent(event)

    def isLocked(self):
        return self.titleBar.isLocked()
