#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *

from .ftitlebar import FTitleBar
from .fstatusbar import FStatusBar


class FMainWindow(QMainWindow):

    sizeChanged = Signal()

    def __init__(self):
        super(FMainWindow, self).__init__()
        self._initFlags()
        self._initWindowFlags()
        self._initMainWindow()

        self._initTitleBar()
        self._initTitlebarConnect()

        self._initToolbars()
        self._initStatusbar()

        self._initSystemTray()

        self.setAttribute(Qt.WA_Hover)
        self.installEventFilter(self)

        self.isSideClicked = False
        self.isCusorLeftSide = False
        self.isCusorRightSide = False
        self.isCusorDownSide = False

    def _initFlags(self):
        self._framelessflag = True  # 无系统边框标志
        self._customTitlebarFlag = True  # 自定义标题栏标志
        self._menubarFlag = False
        self._maximizedflag = False  # 初始化时窗口最大化标志
        self._lockflag = False  # 锁定标志
        self._pinflag = False  # 置顶标志
        self._modeflag = False  # 手机(True)或桌面模式(False)

        desktopWidth = QDesktopWidget().availableGeometry().width()
        desktopHeight = QDesktopWidget().availableGeometry().height()
        self.default_size = QSize(desktopWidth * 0.8, desktopHeight * 0.8)
        self.phone_size = QSize(400, desktopHeight * 0.8)
        self.oldPosition = QPoint(0, 0)

    def _initWindowFlags(self, flag=True):
        framelessflag = flag
        if framelessflag:
            # 无边框， 带系统菜单， 可以最小化
            self.setWindowFlags(
                Qt.FramelessWindowHint)
        self._framelessflag = framelessflag

    def _initMainWindow(self):
        self.showNormal()
        self.moveCenter()
        self.oldPosition = self.pos()

        self.setAttribute(Qt.WA_DeleteOnClose)
        self.layout().setContentsMargins(0, 0, 0, 0)

    def _initSystemTray(self):
        self.systemTray = QSystemTrayIcon(self)
        self.systemTray.activated.connect(self.onSystemTrayIconClicked)

    def onSystemTrayIconClicked(self, reason):
        if reason == QSystemTrayIcon.Unknown:
            pass
        elif reason == QSystemTrayIcon.Context:
            pass
        elif reason == QSystemTrayIcon.DoubleClick:
            pass
        elif reason == QSystemTrayIcon.Trigger:
            self.setVisible(not self.isVisible())
        elif reason == QSystemTrayIcon.MiddleClick:
            pass
        else:
            pass

    def setSystemTrayMenu(self, menu):
        if isinstance(menu, QMenu) and \
                hasattr(self, 'systemTray') and self.systemTray:
            self.systemTray.setContextMenu(menu)

    def setCentralWidget(self, widget):
        centralWidget = QFrame(self)
        mainlayout = QVBoxLayout()
        mainlayout.addWidget(self.titleBar())
        mainlayout.addWidget(widget)
        mainlayout.setContentsMargins(0, 0, 0, 0)
        mainlayout.setSpacing(0)
        centralWidget.setLayout(mainlayout)
        super(FMainWindow, self).setCentralWidget(centralWidget)

    def _initTitleBar(self, flag=True):
        self._customTitlebarFlag = flag
        if self._customTitlebarFlag:
            self._titleBar = FTitleBar()

    def _initTitlebarConnect(self):
        if self.isFtitleBarExisted():
            self._titleBar.locked.connect(self.setLocked)
            self._titleBar.modeed.connect(self.setDesktopMode)
            self._titleBar.pined.connect(self.setPined)
            self._titleBar.minimized.connect(self.showMinimized)
            self._titleBar.maximized.connect(self.swithMaxNormal)

    def isFtitleBarExisted(self):
        if self._customTitlebarFlag:
            if isinstance(self._titleBar, FTitleBar):
                return True
        return False

    def titleBar(self):
        if self.isFtitleBarExisted():
            return self._titleBar

    def _initToolbars(self):
        pass

    def _initStatusbar(self):
        statusbar = FStatusBar(self)
        self.setStatusBar(statusbar)

    def setWindowIcon(self, icon):
        if not isinstance(icon, QIcon):
            qicon = QIcon(icon)
        else:
            qicon = icon
        super(FMainWindow, self).setWindowIcon(qicon)
        if self.isFtitleBarExisted():
            self._titleBar.setLogo(qicon)

        self.systemTray.setIcon(qicon)
        self.systemTray.show()

    def setWindowTitle(self, title):
        super(FMainWindow, self).setWindowTitle(title)
        if self.isFtitleBarExisted():
            self._titleBar.setTitle(title)

    def setDesktopMode(self, flag):
        self._modeflag = flag
        if flag:
            self.resize(self.phone_size.width(), self.height())
        else:
            self.resize(self.default_size)

    def isDesktopMode(self):
        return self._modeflag

    def setPined(self, flag):
        self._pinflag = flag
        if flag:
            self.setWindowFlags(self.windowFlags() | Qt.WindowStaysOnTopHint)
        else:
            self.setWindowFlags(
                self.windowFlags() & (~Qt.WindowStaysOnTopHint))
        self.show()

    def isPined(self):
        return self._pinflag

    def setLocked(self, flag):
        self._lockflag = flag

    def isLocked(self):
        return self._lockflag

    def moveCenter(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def swithMaxNormal(self, flag):
        self._maximizedflag = flag
        if self._maximizedflag:
            self.showMaximized()
        else:
            self.showNormal()

    def showMaximized(self):
        self.resize(QDesktopWidget().availableGeometry().size())
        self.moveCenter()

    def showNormal(self):
        self.resize(self.default_size)
        self.move(self.oldPosition)

    def isMaximized(self):
        return self._maximizedflag

    def resizeEvent(self, event):
        super(FMainWindow, self).resizeEvent(event)
        self.default_size = event.oldSize()
        self.sizeChanged.emit()

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
        # elif self.isSideClicked and self.isCusorLeftSide:
        #     x = event.x() + self.xPos - self.rdragx
        #     w = max(self.minimumWidth(),
        #             self.xPos + self.currentWidth - x)
        #     self.move(x , self.yPos)
        #     self.resize(w, self.currentHeight)
        else:
            # 鼠标移动事件
            if self.isMaximized():
                event.ignore()
            else:
                if not self.isLocked():
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
            # elif event.pos().x() < 5:
            #     self.setCursor(Qt.SizeHorCursor)
            #     self.isCusorLeftSide = True
            elif not self.isSideClicked:
                self.setCursor(Qt.ArrowCursor)
                self.isCusorLeftSide = False
                self.isCusorRightSide = False
                self.isCusorDownSide = False
            return True
        return super(FMainWindow, self).eventFilter(obj, event)
