#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext
from deepin_menu.menu import *

from log import logger


SettingMenuItems = [
    ('AddMusic', 'Add music'),
    None,
    ('EasyMode', 'Easy mode'),
    ('MiniMode', 'Mini mode'),
    None,
    ('CheckUpdate', 'Check update'),
    ('Setting', 'Setting'),
    None,
    ('Exit', 'Exit')
]


class MenuWorker(QObject):

    __contextName__ = 'MenuWorker'

    miniTrigger = pyqtSignal()

    @registerContext
    def __init__(self):
        super(MenuWorker, self).__init__()

    def _menu_item_invoked(self, _id, _checked):
        logger.info(_id, _checked, '+++++++++++++')
        if _id == "MiniMode":
            self.miniTrigger.emit()

    @pyqtSlot()
    def showSettingMenu(self):
        self.menu = Menu(SettingMenuItems)
        self.menu.itemClicked.connect(self._menu_item_invoked)
        self.menu.showRectMenu(QCursor.pos().x(), QCursor.pos().y())


if __name__ == "__main__":
    import sys
    from PyQt5.QtCore import QCoreApplication

    app = QCoreApplication([])

    @pyqtSlot(str, bool)
    def invoked(s, c):
        logger.info("id: ", s, ", checked: ", c)
        # menu.setItemText("id_nonactive", "hello")
        # menu.setItemActivity("id_nonactive", True)

    @pyqtSlot()
    def dismissed():
        app.quit()

    # 1)
    # driver = MenuItem("id_driver", "Driver", "/usr/share/icons/Deepin/apps/16/preferences-driver.png")
    # display = MenuItem("id_display", "Display", "/usr/share/icons/Deepin/apps/16/preferences-display.png")
    # show = Menu()
    # show.addMenuItem(MenuItem("id_sub_display", "Display", "/usr/share/icons/Deepin/apps/16/preferences-display.png"))
    # display.setSubMenu(show)
    # menu = Menu(is_root=True)
    # menu.addMenuItems([driver, display])
    # menu.showMenu(200, 200)

    # 2)
    menu = Menu([("id_driver", "Driver", ("/usr/share/icons/Deepin/apps/16/preferences-display.png",)),
                 None,
                 ("id_display", "_Display", (), [
                  ("display_sub1", "Display One"), ("display_sub2", "Display Two"), ]),
                 ("id_radio", "RadioButtonMenu"),
                 ("id_checkbox", "_CheckBoxMenu"),
                 ("id_extra", "_ExtraTest", (), [], "Ctrl-X"),
                 MenuSeparator(),
                 CheckableMenuItem("radio_group_2:radio:radio2_sub1", "One"),
                 CheckableMenuItem("radio_group_2:radio:radio2_sub2", "Two"),
                 None,
                 CheckableMenuItem(
                     "checkbox_group_2:checkbox:checkbox2_sub1", "One"),
                 CheckableMenuItem(
                     "checkbox_group_2:checkbox:checkbox2_sub2", "Two"),
                 None,
                 MenuItem("id_nonactive", "NotActive", isActive=False),
                 CheckableMenuItem("id_check", "CheckMe", True)], is_root=True,)
    radio_sub = RadioButtonMenu(
        "radio_group_1", [("id_radio1", "Radio One"), ("id_radio2", "Radio Two"), ])
    checkbox_sub = CheckboxMenu("checkbox_group_1", [
                                ("id_checkbox1", "Checkbox One"), ("id_checkbox2", "Checkbox Two")])
    menu.getItemById("id_radio").setSubMenu(radio_sub)
    menu.getItemById("id_checkbox").setSubMenu(checkbox_sub)
    # menu.getItemById("id_radio2").showCheckmark = False
    menu.itemClicked.connect(invoked)
    menu.menuDismissed.connect(dismissed)
    menu.showRectMenu(1400, 300)
    #menu.showDockMenu(1366, 768)

    sys.exit(app.exec_())
