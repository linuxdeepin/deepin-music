#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from dwidgets import DQuickView
from PyQt5.QtCore import Qt, pyqtProperty, QObject, pyqtSlot, pyqtSignal, QThread
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView

class MainWindow(DQuickView):

    quickItems = {}

    def __init__(self):
        super(MainWindow, self).__init__()
        self.setFlags(Qt.FramelessWindowHint)
        self.initContext()
        self.initConnect()

    def initContext(self):
        self.rootContext = self.rootContext()
        self.rootContext.setContextProperty('MainWindow', self)

        # self.rootContext.setContextProperty('PyUtil', self.worker)

    def initConnect(self):
        self.statusChanged.connect(self.trackStatus)
        self.engine().quit.connect(QGuiApplication.instance().quit)

    def intQMLConnect(self):
        self.rootObj = self.rootObject()
        self.getAllItems(self.rootObj)

    def trackStatus(self, status):
        if status == QQuickView.Null:
            print('This QQuickView has no source set.')
        elif status == QQuickView.Ready:
            print('This QQuickView has loaded %s and created the QML component.' % self.source())
            self.moveCenter()
            self.intQMLConnect()
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
