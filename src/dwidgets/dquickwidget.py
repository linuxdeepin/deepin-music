#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
from PyQt5 import QtCore
from PyQt5 import QtGui
from PyQt5.QtQuickWidgets import QQuickWidget
from PyQt5 import QtQml
from PyQt5.QtCore import qVersion
from deepin_utils.file import get_parent_dir

if '5.3' in qVersion():
    isWebengineUsed = False
else:
    isWebengineUsed = True


class DQuickWidget(QQuickWidget):

    """The DQuickWidget ini webengine in PyQt5"""

    def __init__(self, engine=None, parent=None):
        super(DQuickWidget, self).__init__(engine, parent)
        self.setResizeMode(self.SizeViewToRootObject)
        self.engine().addImportPath(os.sep.join([get_parent_dir(__file__, 2), 'qml']))


if __name__ == '__main__':
    import platform
    app = QtGui.QGuiApplication(sys.argv)
    main = DQuickWidget()
    main.setSource(QtCore.QUrl('mainrect.qml'))
    main.show()
    sys.exit(app.exec_())
