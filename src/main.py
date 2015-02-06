#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5 import QtCore

if os.name == 'posix':
    QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_X11InitThreads, True)

from dwidgets import QSingleApplication
from views import MainWindow
import config


if __name__ == '__main__':
    app = QSingleApplication(config.applicationName, sys.argv)
    if app.isRunning():
        sys.exit(0)
    mainWindow = MainWindow()
    mainWindow.setSource(QtCore.QUrl('views/main.qml'))
    app.setActivationWindow(mainWindow)
    mainWindow.show()

    exitCode = app.exec_()
    sys.exit(exitCode)
