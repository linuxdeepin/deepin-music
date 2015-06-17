#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import (
    Qt, QRect,QUrl,
    pyqtProperty, QObject, 
    pyqtSlot, pyqtSignal, 
    QThread)
from PyQt5.QtGui import QGuiApplication, QIcon
from PyQt5.QtQuick import QQuickView
from dwidgets import DQuickView
from controllers import registerContext
import config

from log import logger


class BaseWindow(DQuickView):

    globalObjectChanged = pyqtSignal('QJSValue')
    loadSuccessed = pyqtSignal()

    def __init__(self, engine=None, parent=None):
        super(BaseWindow, self).__init__(engine, parent)
        self.setFlags(Qt.FramelessWindowHint)
        self.setIcon(QIcon(config.windowIcon))
        self.initConnect()
        self.quickItems = {}

    def setContext(self, name, obj):
        rootContext = self.rootContext()
        if rootContext.contextProperty(name) is None:
            rootContext.setContextProperty(name, obj)

    def setContexts(self, cons=None):
        assert isinstance(cons, dict)
        for key in cons:
            self.setContext(key, cons[key])

    @pyqtProperty('QJSValue', notify=globalObjectChanged)
    def globalObject(self):
        return self.engine().globalObject()

    def initConnect(self):
        self.statusChanged.connect(self.trackStatus)
        self.engine().quit.connect(QGuiApplication.instance().quit)

    def initQMLConnect(self):
        self.getAllItems(self.rootObject())

    def trackStatus(self, status):
        if status == QQuickView.Null:
            logger.info('This QQuickView has no source set.')
        elif status == QQuickView.Ready:
            logger.info('This QQuickView %s has loaded %s and created the QML component.' %(str(self), self.source()))
            self.moveCenter()
            self.initQMLConnect()
            self.loadSuccessed.emit()
        elif status == QQuickView.Loading:
            logger.info('This QQuickView is loading network data.')
        elif status == QQuickView.Error:
            logger.info('One or more errors has occurred. Call errors() to retrieve a list of errors.')
            logger.info(self.errors())

    def getAllItems(self, obj):
        for item in obj.childItems():
            if item.objectName():
                self.quickItems.update({item.objectName(): item})
            self.getAllItems(item)

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
        super(BaseWindow, self).mouseReleaseEvent(event)

    def mouseMoveEvent(self, event):
        # 鼠标移动事件
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                self.setPosition(event.globalPos() - self.dragPosition)
                event.accept()
        super(BaseWindow, self).mouseMoveEvent(event)
