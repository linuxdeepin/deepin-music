#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from dwidgets import DQuickView
from PyQt5.QtCore import Qt, pyqtProperty
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView

class MainWindow(DQuickView):

    def __init__(self):
        super(MainWindow, self).__init__()
        self.setFlags(Qt.FramelessWindowHint)
        self.rootContext().setContextProperty('MainWindow', self)
        self.initConnect()

    def initConnect(self):
        self.statusChanged.connect(self.trackStatus)
        self.engine().quit.connect(QGuiApplication.instance().quit)

    def trackStatus(self, status):
        if status == QQuickView.Null:
            print('This QQuickView has no source set.')
        elif status == QQuickView.Ready:
            print('This QQuickView has loaded %s and created the QML component.' % self.source())
            self.moveCenter()
            self.rootobj = self.rootObject()
            self.quickItems = {}
            self.getAllItems(self.rootobj)
        elif status == QQuickView.Loading:
            print('This QQuickView is loading network data.')
        elif status == QQuickView.Error:
            print('One or more errors has occurred. Call errors() to retrieve a list of errors.')
            print(self.errors())

    def getAllItems(self, obj):
        for item in obj.childItems():
            if item and item.objectName():
                self.quickItems.update({item.objectName(): item})
                if item.childItems():
                    self.getAllItems(item)

    # def keyPressEvent(self, event):
    #     if(event.key() == Qt.Key_F1):
    #         # self.rootobj.setFocus(True)
    #         pass
    #     elif (event.key() == Qt.Key_F2):
    #         # self.rootobj.setFocus(True)
    #         pass
    #     super(MainWindow, self).keyPressEvent(event)
