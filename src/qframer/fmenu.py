#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *


class FMenu(QMenu):

    '''
        self.menuItems 定义menu
        self.actions QAction集合
    '''

    def __init__(self, parent=None):
        super(FMenu, self).__init__(parent)
        self.parent = parent
        self.qactions = {}
        self.menuItems = []
        self.creatMenus(self.menuItems)

    def creatMenus(self, menusettings):
        for menuaction in menusettings:
            if 'type' in menuaction and menuaction['type'] == "submenu":
                self.createSubAction(menuaction['trigger'], menuaction)
            else:
                self.creatAction(self, menuaction)

    def createSubAction(self, pmenu_name, menu):
        submenu = self.addMenu(u'%s' % menu['name'])
        setattr(
            self,
            '%smenu' % menu['trigger'],
            submenu)
        for menuaction in menu['actions']:
            self.creatAction(submenu, menuaction)

    def creatAction(self, submenu, menuaction):
        if 'checkable' in menuaction:
            setattr(
                self,
                '%sAction' % menuaction['trigger'],
                QAction(
                    QIcon(QPixmap(menuaction['icon'])),
                    u'%s' % menuaction['name'],
                    self,
                    checkable=menuaction['checkable']
                )
            )
        else:
            setattr(
                self,
                '%sAction' % menuaction['trigger'],
                QAction(
                    QIcon(QPixmap(menuaction['icon'])),
                    u'%s' % menuaction['name'],
                    self,
                )
            )

        action = getattr(self, '%sAction' % menuaction['trigger'])
        action.setShortcut(QKeySequence(menuaction['shortcut']))
        submenu.addAction(action)
        self.qactions.update({menuaction['trigger']: action})

    def getActionByName(self, name):
        if hasattr(self, '%sAction' % name):
            return getattr(self, '%sAction' % name)
        else:
            return None
