#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext
from dwidgets import ModelMetaclass


class ConfigWorker(object):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('playbackMode', int, 4),
        ('volume', int, 10)
    )

    __contextName__ = "ConfigWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        pass
