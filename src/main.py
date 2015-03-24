#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QCoreApplication, Qt

if os.name == 'posix':
    QCoreApplication.setAttribute(Qt.AA_X11InitThreads, True)

from dwidgets import QSingleApplication
from app import DeepinPlayer
import config


if __name__ == '__main__':
    app = QSingleApplication(config.applicationName, sys.argv)
    if app.isRunning():
        sys.exit(0)

    deepinPlayer = DeepinPlayer.instance()
    deepinPlayer.show()

    exitCode = app.exec_()

    sys.exit(exitCode)
