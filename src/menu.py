#!/usr/bin/python
# -*- coding: utf-8 -*-

from dwidgets.dmenu import DMenu
from PyQt5.QtGui import QPainter, QPainterPath, QRegion, QBitmap
from PyQt5.QtCore import Qt, QRectF

class SettingsMenu(DMenu):

    """docstring for SettingsMenu"""

    style = '''
        QMenu {
            background-color: transparent; /* sets background of the menu */
            border: 4px;
        }

        QMenu::item {
            /* sets background of menu item. set this to something non-transparent
                if you want menu color and menu item color to be different */
            background-color: white;
        }

        QMenu::item:selected { /* when user selects item using mouse or keyboard */
            background-color: #654321;
        }
    '''

    def __init__(self, parent=None):
        super(SettingsMenu, self).__init__(parent)
        self.parent = parent
        self.menuItems = [
            {
                'name': self.tr('Login'),
                'icon': u'',
                'shortcut': u'',
                'trigger': 'Login',
            },
            {
                'name': self.tr('Show s'),
                'icon': u'',
                'shortcut': u'',
                'trigger': 'Suspension',
            },
            {
                'name': self.tr('Show f'),
                'icon': u'',
                'shortcut': u'',
                'trigger': 'Float',
            },
            {
                'name': self.tr('Show D'),
                'icon': u'',
                'shortcut': u'',
                'trigger': 'Dock',
            },
            {
                'name': self.tr('Language'),
                'trigger': 'Language',
                'type': 'submenu',
                'actions': [
                    {
                        'name': 'English',
                        'icon': u'',
                        'shortcut': u'',
                        'trigger': 'English',
                        "checkable": True
                    },
                    {
                        'name': 'Chinese',
                        'icon': u'',
                        'shortcut': u'',
                        'trigger': 'Chinese',
                        "checkable": True
                    },
                ]
            },
            {
                'name': self.tr('Document'),
                'trigger': 'Document',
                'type': 'submenu',
                'actions': [
                    {
                        'name': 'Android developer guide',
                        'icon': u'',
                        'shortcut': u'',
                        'trigger': 'AndroidDeveloper',
                        "checkable": False
                    },
                    {
                        'name': 'iOS developer guide',
                        'icon': u'',
                        'shortcut': u'',
                        'trigger': 'IOSDeveloper',
                        "checkable": False
                    },
                    {
                        'name': 'Ford developer center',
                        'icon': u'',
                        'shortcut': u'',
                        'trigger': 'FordDeveloper',
                        "checkable": False
                    },
                ]
            },
            {
                'name': self.tr('ObjectView'),
                'icon': u'',
                'shortcut': u'',
                'trigger': 'ObjectView',
            },
            {
                'name': self.tr('About'),
                'icon': u'',
                'shortcut': u'Qt.Key_F12',
                'trigger': 'About',
            },
            {
                'name': self.tr('Exit'),
                'icon': u'',
                'shortcut': u'',
                'trigger': 'Exit',
            },
        ]
        self.creatMenus(self.menuItems)
        self.initConnect()

        self.setStyleSheet(self.style)

        

        getattr(self, '%sAction' % 'English').setChecked(True)

    def initConnect(self):
        for item in ['English', 'Chinese']:
            getattr(self, '%sAction' % item).triggered.connect(self.updateChecked)

    def updateChecked(self):
        for item in ['English', 'Chinese']:
            action = getattr(self, '%sAction' % item)
            if self.sender() is action:
                action.setChecked(True)
            else:
                action.setChecked(False)

    def paintEvent(self, event):
        # objBitmap = QBitmap(self.size())
        # self.painter = QPainter(objBitmap)
        # self.painter.setRenderHints(QPainter.Antialiasing | QPainter.HighQualityAntialiasing, True)
        # self.painter.fillRect(self.rect(), Qt.white);
        # self.painter.drawRoundedRect(self.rect(), 4, 4) #圆角5像素
        # self.setMask(objBitmap)
        super(SettingsMenu, self).paintEvent(event)
        


    #     painter.setRenderHints(QPainter::Antialiasing
    #                            | QPainter::HighQualityAntialiasing, true);  // 圆角平滑
    #     painter.fillRect(rect(), Qt::white);            // 填充位图矩形框(用白色填充)
    #     painter.setBrush(QColor(0, 0, 0));
    #     painter.drawRoundedRect(this->rect(), 5, 5);    // 在位图上画圆角矩形(用黑色填充)
    #     setMask(objBitmap);                             // 使用setmask过滤即可，被圆角切除部分用白色填充
