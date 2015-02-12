#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

from PyQt5.QtCore import Qt, pyqtProperty, QObject, pyqtSlot, pyqtSignal, QThread
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView
from dwidgets import DQuickView
from controllers import registerContext, contexts


class MainWindow(DQuickView):

    quickItems = {}

    __contextName__ = 'MainWindow'

    @registerContext
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setFlags(Qt.FramelessWindowHint)
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

    # def keyPressEvent(self, event):
    #     if(event.key() == Qt.Key_F1):
    #         # self.rootobj.setFocus(True)
    #         pass
    #     elif (event.key() == Qt.Key_F2):
    #         # self.rootobj.setFocus(True)
    #         pass
    #     super(MainWindow, self).keyPressEvent(event)
