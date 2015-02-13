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
        self.setResizeMode(QtQuick.QQuickView.SizeViewToRootObject)
        self.engine().addImportPath(os.sep.join([os.getcwd(), 'qml']))
        self.initWebengine()

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
