#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from dwidgets import DQuickView
from PyQt5.QtCore import Qt, pyqtProperty

class MainWindow(DQuickView):

    def __init__(self):
        super(MainWindow, self).__init__()
        self.setFlags(Qt.FramelessWindowHint)
        self.rootContext().setContextProperty('MainWindow', self)
