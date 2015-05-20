#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
os.environ['LC_ALL'] = "en_US.UTF-8"
import sys
from PyQt5.QtCore import QCoreApplication, Qt, QThread
from PyQt5.QtQml import qmlRegisterType
# from PyQt5.QtQuick import QQuickWindow
if os.name == 'posix':
    QCoreApplication.setAttribute(Qt.AA_X11InitThreads, True)

from dwidgets import QSingleApplication
from app import DeepinPlayer
import config
import faulthandler
import threading
from config.constants import LogCachePath 

fd = open(os.path.join(LogCachePath, 'crash.log'), 'w')
faulthandler.enable(fd, all_threads=True)


if __name__ == '__main__':
    app = QSingleApplication(config.applicationName, sys.argv)
    if app.isRunning():
        sys.exit(0)
    # qmlRegisterType(QQuickWindow, "QQuickWindow", 1, 0, "DWindow")
    deepinPlayer = DeepinPlayer.instance()
    deepinPlayer.show()
    exitCode = app.exec_()
    sys.exit(exitCode)
