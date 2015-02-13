#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect,
    pyqtProperty, QObject, 
    pyqtSlot, pyqtSignal, 
    QThread)
from PyQt5.QtGui import QGuiApplication, QIcon
from PyQt5.QtQuick import QQuickView
from dwidgets import DQuickView
from controllers import registerContext, contexts
import config


class MainWindow(DQuickView):

    quickItems = {}

    __contextName__ = 'MainWindow'

    @registerContext
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setFlags(Qt.FramelessWindowHint)
        self.setIcon(QIcon(config.windowIcon))
        self.initConnect()

    def setContext(self, name, obj):
        rootContext = self.rootContext()
        if rootContext.contextProperty(name) is None:
            rootContext.setContextProperty(name, obj)

    def setContexts(self, cons=None):
        assert isinstance(cons, dict)
        for key in cons:
            self.setContext(key, cons[key])

    def initConnect(self):
        self.statusChanged.connect(self.trackStatus)
        self.engine().quit.connect(QGuiApplication.instance().quit)

    def intQMLConnect(self):
        self.getAllItems(self.rootObject())

    def trackStatus(self, status):
        if status == QQuickView.Null:
            print('This QQuickView has no source set.')
        elif status == QQuickView.Ready:
            print('This QQuickView has loaded %s and created the QML component.' % self.source())
            self.moveCenter()
            self.intQMLConnect()
            self.quickItems['MainMusic'].initConnect()

        elif status == QQuickView.Loading:
            print('This QQuickView is loading network data.')
        elif status == QQuickView.Error:
            print('One or more errors has occurred. Call errors() to retrieve a list of errors.')
            print(self.errors())

    def getAllItems(self, obj):
        for item in obj.childItems():
            if item.objectName():
                self.quickItems.update({item.objectName(): item})
            self.getAllItems(item)

    def mousePressEvent(self, event):
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            x = self.quickItems['leftSideBar'].width()
            y = self.quickItems['titleBar'].height()
            width = x + self.quickItems['webEngineViewPage'].width()
            height = y + self.quickItems['webEngineViewPage'].height()
            rect = QRect(x, y , width, height)
            if not rect.contains(event.pos()):
                self.dragPosition = event.globalPos() - self.frameGeometry().topLeft()
                event.accept()
        super(DQuickView, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
        super(DQuickView, self).mouseReleaseEvent(event)

    def mouseMoveEvent(self, event):
        # 鼠标移动事件
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                self.setPosition(event.globalPos() - self.dragPosition)
                event.accept()
        super(DQuickView, self).mouseMoveEvent(event)
