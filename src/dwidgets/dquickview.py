#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
from PyQt5 import QtCore
from PyQt5 import QtGui
from PyQt5 import QtQuick
from PyQt5 import QtQml


class DQuickView(QtQuick.QQuickView):

    """The DQuickView init webengine in PyQt5"""

    def __init__(self):
        super(DQuickView, self).__init__()
        self.setResizeMode(QtQuick.QQuickView.SizeRootObjectToView)
        self.engine().addImportPath(os.sep.join([os.getcwd(), 'qml']))
        self.initWebengine()
        self.initConnect()

    def initWebengine(self):
        component = QtQml.QQmlComponent(self.engine())
        component.setData('''
            import QtQuick 2.4
            import WebEngineWrapper 1.0
            WebEngineWrapper {
                Component.onCompleted: {
                    initialize()
                }
            }
        ''', QtCore.QUrl(''));
        item = component.create()
        item.setParentItem(self.rootObject())

    def initConnect(self):
        self.statusChanged.connect(self.trackStatus)
        self.engine().quit.connect(QtGui.QGuiApplication.instance().quit)

    def trackStatus(self, status):
        if status == QtQuick.QQuickView.Null:
            print('This QQuickView has no source set.')
        elif status == QtQuick.QQuickView.Ready:
            print('This QQuickView has loaded %s and created the QML component.' % self.source())
        elif status == QtQuick.QQuickView.Loading:
            print('This QQuickView is loading network data.')
        elif status == QtQuick.QQuickView.Error:
            print('One or more errors has occurred. Call errors() to retrieve a list of errors.')
            print(self.errors())

    def mousePressEvent(self, event):
        # 鼠标点击事件
        if event.button() == QtCore.Qt.LeftButton:
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
            if event.buttons() == QtCore.Qt.LeftButton:
                self.setPosition(event.globalPos() - self.dragPosition)
                event.accept()
        super(DQuickView, self).mouseMoveEvent(event)

    def moveCenter(self):
        qr = self.frameGeometry()
        cp = self.screen().availableGeometry().center()
        qr.moveCenter(cp)
        self.setPosition(qr.topLeft())


if __name__ == '__main__':
    import platform
    app = QtGui.QGuiApplication(sys.argv)
    main = DQuickView()
    main.setSource(QtCore.QUrl('mainrect.qml'))
    main.show()
    sys.exit(app.exec_())
