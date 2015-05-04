#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import functools

from .qt.QtCore import *
from .qt.QtGui import *

views = {}


def collectView(func):

    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        self = args[0]
        if hasattr(self, 'viewID'):
            views.update({self.viewID: self})
        func(*args, **kwargs)
    return wrapper


def setSkinForApp(qssfile):
    if os.path.exists(qssfile):
        fd = open(qssfile, "r")
        style = fd.read()
        fd.close()
        QApplication.instance().setStyleSheet(style)
    else:
        QApplication.instance().setStyleSheet("")
