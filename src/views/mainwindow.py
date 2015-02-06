#!/usr/bin/python
# -*- coding: utf-8 -*-

from dwidgets import DQuickView
from PyQt5.QtCore import Qt, pyqtProperty

class MainWindow(DQuickView):

    def __init__(self):
        super(MainWindow, self).__init__()
        self.setFlags(Qt.FramelessWindowHint)
        self.resize(960, 600)
        self.moveCenter()
        self.rootContext().setContextProperty('windowView', self)
