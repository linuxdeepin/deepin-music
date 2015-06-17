#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect, QUrl,
    pyqtProperty, QObject,
    pyqtSlot, pyqtSignal,
    QThread, QPointF)
from PyQt5.QtGui import QKeySequence
from PyQt5.QtWidgets import QSystemTrayIcon
from PyQt5.QtQuick import QQuickView
from .basewindow import BaseWindow
from controllers import registerContext, windowManageWorker, signalManager, menuWorker
from lrcwindow import LrcWindowManager


class MainWindow(BaseWindow):

    __contextName__ = 'MainWindow'

    windowFocusChanged = pyqtSignal('QWindow*', arguments=['window'])
    mousePressed = pyqtSignal('QPointF', arguments=['point'])
    wheel = pyqtSignal('QPointF', arguments=['point'])

    @registerContext
    def __init__(self, engine=None, parent=None):
        super(MainWindow, self).__init__(engine, parent)
        self._initSystemTray()
        self._initConnect()

    def _initConnect(self):
        from PyQt5.QtWidgets import qApp
        qApp.focusWindowChanged.connect(self.changeFocusWindow)
        signalManager.hideShowWindowToggle.connect(self.actionhideShow)

    def _initSystemTray(self):
        from PyQt5.QtWidgets import qApp
        self.systemTray = QSystemTrayIcon(self)
        self.systemTray.setIcon(qApp.windowIcon())
        self.systemTray.show()
        self.systemTray.activated.connect(self.onSystemTrayIconClicked)

    def onSystemTrayIconClicked(self, reason):
        if reason == QSystemTrayIcon.Unknown:
            pass
        elif reason == QSystemTrayIcon.Context:
            menuWorker.systemTrayMenuShowed.emit()
        elif reason == QSystemTrayIcon.DoubleClick:
            pass
        elif reason == QSystemTrayIcon.Trigger:
            self.setVisible(not self.isVisible())
        elif reason == QSystemTrayIcon.MiddleClick:
            pass
        else:
            pass

    def mousePressEvent(self, event):
        self.mousePressed.emit(event.pos())
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            flag = windowManageWorker.windowMode
            if flag == "Full":
                x = self.quickItems['mainTitleBar'].x()
                y = self.quickItems['mainTitleBar'].y()
                width = self.quickItems['mainTitleBar'].width()
                height = self.quickItems['mainTitleBar'].height()
                rect = QRect(x, y, width, height)
                if rect.contains(event.pos()):
                    self.dragPosition = event.globalPos() - \
                        self.frameGeometry().topLeft()
            elif flag == "Simple":
                x = self.rootObject().x()
                y = self.rootObject().y()
                width = self.rootObject().width()
                height = 40
                rect = QRect(x, y, width, height)
                if rect.contains(event.pos()):
                    self.dragPosition = event.globalPos() - \
                        self.frameGeometry().topLeft()
        super(MainWindow, self).mousePressEvent(event)

    def changeFocusWindow(self, window):
        self.windowFocusChanged.emit(window)

    def wheelEvent(self, event):
        self.wheel.emit(QPointF(event.x(), event.y()))
        super(MainWindow, self).wheelEvent(event)

    def keyPressEvent(self, event):
        from controllers import configWorker, signalManager, mediaPlayer
        if configWorker.isShortcutEnable:
            modifier = QKeySequence(event.modifiers()).toString()
            keyString = QKeySequence(event.key()).toString()
            shortcut = modifier + keyString
            print shortcut
            if shortcut == configWorker.shortcut_preivous:
                signalManager.previousSong.emit()
            elif shortcut == configWorker.shortcut_next:
                signalManager.nextSong.emit()
            elif shortcut == configWorker.shortcut_volumnIncrease:
                signalManager.volumnIncrease.emit()
            elif shortcut == configWorker.shortcut_volumeDecrease:
                signalManager.volumnDecrease.emit()
            elif shortcut == configWorker.shortcut_playPause:
                signalManager.playToggle.emit(not mediaPlayer.playing)
            elif shortcut == configWorker.shortcut_simpleFullMode:
                signalManager.simpleFullToggle.emit()
            elif shortcut == configWorker.shortcut_miniFullMode:
                signalManager.miniFullToggle.emit()
            elif shortcut == configWorker.shortcut_hideShowWindow:
                signalManager.hideShowWindowToggle.emit()
            elif shortcut == configWorker.shortcut_hideShowDesktopLRC:
                signalManager.hideShowDesktopLrcToggle.emit()

        super(MainWindow, self).keyPressEvent(event)

    def actionhideShow(self):
        if self.windowState() == Qt.WindowActive:
            self.setWindowState(Qt.WindowMinimized)
        elif self.windowState() == Qt.WindowMinimized:
            self.setWindowState(Qt.WindowActive)
