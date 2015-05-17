#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot, 
    pyqtProperty, QUrl)
from .utils import registerContext, contexts


class SignalManager(QObject):

    __contextName__ = 'SignalManager'

    addtoFavorite = pyqtSignal('QString')
    removeFromFavorite = pyqtSignal('QString')

    @registerContext
    def __init__(self, parent=None):
        super(SignalManager, self).__init__(parent)


signalManager = SignalManager()
