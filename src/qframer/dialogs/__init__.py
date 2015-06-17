#!/usr/bin/python
# -*- coding: utf-8 -*-

from .logindialog import login
from .exitdialog import exit
from .msgdialog import MessageDialog
from .msgdialog import msg
from .ipaddressdialog import ipaddressinput
from .urlinputdialog import urlinput
from .numinputdialog import numinput
from .confirmdialog import confirm
from .confirmdialog import ConfirmDialog
from .basedialog import DynamicTextWidget
# from .weblogindialog import weblogin
from .settingsdialog import settingsinput

__version__ = '0.1.0'

__all__ = ['DynamicTextWidget', 'ConfirmDialog', 'MessageDialog', 'login', 'exit', 'msg', 'ipaddressinput', 'urlinput', 'numinput', 'confirm']

__author__ = 'dragondjf(dragondjf@gmail.com)'
