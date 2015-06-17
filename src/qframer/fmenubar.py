#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *


class FMenuBar(QMenuBar):

    def __init__(self, parent=None):
        super(FMenuBar, self).__init__(parent)
        self.qactions = {}
        self.menuItems = []
        self.creatMenus(self.menuItems)

    def creatMenus(self, menusettings):
        for menu in menusettings['menus']:
            setattr(
                self,
                '%smenu' % menu['trigger'],
                self.addMenu(u'%s' % menu['name'])
            )
            submenu = getattr(self, '%smenu' % menu['trigger'])
            for menuaction in menu['actions']:
                if 'type' in menuaction and menuaction['type'] == "submenu":
                    self.createSubAction(menu['trigger'], menuaction)
                else:
                    self.creatAction(submenu, menuaction)

    def createSubAction(self, pmenu_name, menu):
        childmenu = getattr(self, '%smenu' % pmenu_name)
        submenu = childmenu.addMenu(u'%s' % menu['name'])
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
