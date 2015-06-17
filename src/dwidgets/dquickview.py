#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
from PyQt5 import QtCore
from PyQt5 import QtGui
from PyQt5 import QtQuick
from PyQt5 import QtQml
from PyQt5.QtCore import qVersion, pyqtSlot, pyqtSignal, QPointF, Qt
from PyQt5.QtGui import QCursor, QSurfaceFormat, QColor, QPainter, QPixmap,QBitmap, QRegion, QBrush
from deepin_utils.file import get_parent_dir

if '5.3' in qVersion():
    isWebengineUsed = False
else:
    isWebengineUsed = True


class DQuickView(QtQuick.QQuickView):

    """The DQuickView init webengine in PyQt5"""

    def __init__(self, engine=None, parent=None):
        super(DQuickView, self).__init__(engine, parent)
        self.setResizeMode(QtQuick.QQuickView.SizeViewToRootObject)
        self.engine().addImportPath(os.sep.join([get_parent_dir(__file__, 2), 'qml']))
        if isWebengineUsed:
            self.initWebengine()

        format = QSurfaceFormat()
        format.setAlphaBufferSize(8)
        format.setRenderableType(QSurfaceFormat.OpenGL)

        self.setFormat(format)
        self.setColor(QColor(Qt.transparent))
        self.setClearBeforeRendering(True)


    def setRoundMask(self):
        bmp = QPixmap(self.size())
        bmp.fill(Qt.white)
        p = QPainter(bmp)
        p.setRenderHint(QPainter.Antialiasing)
        p.setBrush(QBrush(Qt.white))
        p.setCompositionMode(QPainter.CompositionMode_Clear)
        p.drawRoundedRect(0, 0, self.width(), self.height(), 3, 3)
        p.end()
        self.setMask(QRegion(QBitmap(bmp)))


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

    @pyqtSlot(result='QPoint')
    def getCursorPos(self):
        return QCursor.pos()


if __name__ == '__main__':
    import platform
    app = QtGui.QGuiApplication(sys.argv)
    main = DQuickView()
    main.setSource(QtCore.QUrl('mainrect.qml'))
    main.show()
    sys.exit(app.exec_())
