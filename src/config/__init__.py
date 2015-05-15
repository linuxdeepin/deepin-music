#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import json
from PyQt5.QtCore import qVersion

applicationName = 'Deepin Music'
applicationVersion = '3.0.0'
organizationDomain = 'linuxdeepin.com'
organizationName = "Deepin"
windowIcon = os.sep.join(['skin', 'images', 'deepin-music.png'])
windowTitle = u'Deepin Music'

if qVersion().startswith('5.3'):
    isWebengineUsed = False
else:
    isWebengineUsed = True
