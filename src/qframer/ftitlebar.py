#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *
from .resources import *


baseHeight = 25


class BaseToolButton(QToolButton):

    def __init__(self, parent=None):
        super(BaseToolButton, self).__init__(parent)
        self.setFocusPolicy(Qt.NoFocus)
        iconBaseSize = QSize(baseHeight, baseHeight)
        self.setIconSize(iconBaseSize)
        self.setFixedSize(30, baseHeight)

    def setMenu(self, menu):
        super(BaseToolButton, self).setMenu(menu)
        menu.aboutToHide.connect(self.recover)

    def recover(self):
        import sys
        if sys.platform == "linux2":
            self.setAttribute(Qt.WA_UnderMouse, self.rect().contains(self.mapFromGlobal(QCursor.pos())))
            self.update()


class FTitleBar(QFrame):

    settingMenuShowed = Signal()
    skinMenuShowed = Signal()
    modeed = Signal(bool)
    locked = Signal(bool)
    pined = Signal(bool)
    minimized = Signal()
    maximized = Signal(bool)
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
        self.phoneIcon = QIcon(":/icons/dark/appbar.windowsphone.png")
        self.desktopIcon = QIcon(":/icons/dark/appbar.monitor.png")
        self.lockIcon = QIcon(":/icons/dark/appbar.lock.png")
        self.unlockIcon = QIcon(":/icons/dark/appbar.unlock.keyhole.png")
        self.pinIcon = QIcon(":/icons/dark/appbar.pin.png")
        self.unPinIcon = QIcon(":/icons/dark/appbar.pin.remove.png")
        self.minIcon = QIcon(":/icons/dark/appbar.minus.png")
        self.maxIcon = QIcon(":/icons/dark/appbar.fullscreen.box.png")
        self.normalIcon = QIcon(":/icons/dark/appbar.app.png")
        self.closeIcon = QIcon(":/icons/dark/appbar.close.png")

        self.max_flag = False
        self.mode_flag = False
        self.lock_flag = False
        self.pin_flag = False

    def initUI(self):
        self.setFixedHeight(baseHeight)

        self.logoButton = BaseToolButton()
        self.logoButton.setObjectName("logo")
        self.logoButton.setStyleSheet(self.logostyle)

        self.titleLabel = QLabel()

        self.skinButton = BaseToolButton()
        self.skinButton.setIcon(self.clothesIcon)

        self.modeButton = BaseToolButton()
        self.modeButton.setIcon(self.phoneIcon)

        self.lockButton = BaseToolButton()
        self.lockButton.setIcon(self.unlockIcon)

        self.pinButton = BaseToolButton()
        self.pinButton.setIcon(self.unPinIcon)

        self.settingDownButton = BaseToolButton()
        self.settingDownButton.setIcon(self.settingDownIcon)

        self.minButton = BaseToolButton()
        self.minButton.setIcon(self.minIcon)

        self.maxButton = BaseToolButton()
        self.maxButton.setIcon(self.normalIcon)

        self.closeButton = BaseToolButton()
        self.closeButton.setObjectName("close")
        self.closeButton.setStyleSheet(self.closestyle)
        self.closeButton.setIcon(self.closeIcon)

        mainLayout = QHBoxLayout()
        mainLayout.addWidget(self.logoButton)
        mainLayout.addSpacing(5)
        mainLayout.addWidget(self.titleLabel)
        mainLayout.addStretch()
        mainLayout.addWidget(self.settingDownButton)
        mainLayout.addWidget(self.skinButton)
        mainLayout.addWidget(self.modeButton)
        mainLayout.addWidget(self.pinButton)
        mainLayout.addWidget(self.lockButton)
        mainLayout.addWidget(self.minButton)
        mainLayout.addWidget(self.maxButton)
        mainLayout.addWidget(self.closeButton)
        mainLayout.setContentsMargins(0, 0, 5, 0)
        mainLayout.setSpacing(0)
        self.setLayout(mainLayout)

        self.settingDownButton.clicked.connect(self.settingMenuShowed)
        self.skinButton.clicked.connect(self.skinMenuShowed)
        self.modeButton.clicked.connect(self.swithModeIcon)
        self.lockButton.clicked.connect(self.swithLockIcon)
        self.pinButton.clicked.connect(self.swithPinIcon)
        self.minButton.clicked.connect(self.minimized)
        self.maxButton.clicked.connect(self.swicthMaxIcon)
        self.closeButton.clicked.connect(self.closed)

    def swithModeIcon(self):
        if self.mode_flag:
            self.modeButton.setIcon(self.phoneIcon)
        else:
            self.modeButton.setIcon(self.desktopIcon)
        self.mode_flag = not self.mode_flag
        self.modeed.emit(self.mode_flag)

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

    def swicthMaxIcon(self):
        if self.max_flag:
            self.maxButton.setIcon(self.normalIcon)
        else:
            self.maxButton.setIcon(self.maxIcon)
        self.max_flag = not self.max_flag
        self.maximized.emit(self.max_flag)

    def mouseDoubleClickEvent(self, event):
        self.maxButton.click()

    def setLogo(self, icon):
        if isinstance(icon, QIcon):
            logoIcon = icon
        else:
            logoIcon = QIcon(icon)
        self.logoButton.setIcon(logoIcon)

    def setTitle(self, text):
        self.titleLabel.setText(text)

    def getTitle(self):
        return self.titleLabel.text()

    def isLocked(self):
        return self.lock_flag

    def isMax(self):
        return self.max_flag
