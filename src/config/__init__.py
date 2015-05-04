#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import json
from PyQt5.QtCore import qVersion

applicationName = 'Deepin Music'
applicationVersion = '3.0.0'
organizationDomain = 'linuxdeepin.com'
organizationName = "Deepin"
windowIcon = os.sep.join(['skin', 'images', 'PFramer.png'])
windowTitle = u'Deepin Music'

if '5.3' in qVersion():
    isWebengineUsed = False
else:
    isWebengineUsed = True
