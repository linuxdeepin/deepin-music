#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
import sys
os.environ['LC_ALL'] = "en_US.UTF-8"
os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])))
from PyQt5.QtCore import QCoreApplication, Qt, QThread
from PyQt5.QtQml import qmlRegisterType
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
    deepinPlayer = DeepinPlayer.instance()
    deepinPlayer.show()
    exitCode = app.exec_()
    sys.exit(exitCode)
