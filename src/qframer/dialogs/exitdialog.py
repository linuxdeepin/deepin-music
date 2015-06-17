#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
from qframer.qt.QtCore import *
from qframer.qt.QtGui import *

if __name__ == '__main__':
    from basedialog import BaseDialog
else:
    from .basedialog import BaseDialog


class ExitDialog(BaseDialog):
    def __init__(self, styleoptions, parent=None):
        super(ExitDialog, self).__init__(styleoptions, parent)

        # 退出设置
        self.exitoptwidget = QWidget()
        exit_mainlayout = QGridLayout()

        self.exitradiogroup = QButtonGroup(self.exitoptwidget)
        self.minRadio = QRadioButton(u'最小化')
        self.exitRadio = QRadioButton(u'退出')
        self.exitsaveRadio = QRadioButton(u'退出并保存配置')
        self.exitradiogroup.addButton(self.minRadio)
        self.exitradiogroup.addButton(self.exitRadio)
        self.exitradiogroup.addButton(self.exitsaveRadio)

        exit_mainlayout.addWidget(self.minRadio, 0, 0)
        exit_mainlayout.addWidget(self.exitRadio, 1, 0)
        exit_mainlayout.addWidget(self.exitsaveRadio, 2, 0)
        self.exitoptwidget.setLayout(exit_mainlayout)
        self.exitsaveRadio.setChecked(True)

        #确认按钮布局
        self.enterwidget = QWidget()
        self.pbEnter = QPushButton(u'确定', self)
        self.pbCancel = QPushButton(u'取消', self)
        self.pbEnter.setFixedHeight(30)
        self.pbCancel.setFixedHeight(30)

        self.pbEnter.clicked.connect(self.exit)
        self.pbCancel.clicked.connect(self.close)

        enterwidget_mainlayout = QGridLayout()
        enterwidget_mainlayout.addWidget(self.pbEnter, 0, 0)
        enterwidget_mainlayout.addWidget(self.pbCancel, 0, 1)
        self.enterwidget.setLayout(enterwidget_mainlayout)

        self.layout().addStretch()
        self.layout().addWidget(self.exitoptwidget)
        self.layout().addStretch()
        self.layout().addWidget(self.enterwidget)
        self.resize(self.width(), self.height())

        self.exitflag = {}

    def exit(self):
        for radio in ['minRadio', 'exitRadio', 'exitsaveRadio']:
            if getattr(self, radio) is self.exitradiogroup.checkedButton():
                self.exitflag.update({radio: True})
            else:
                self.exitflag.update({radio: False})
        self.accept()


def exit(options):
    """返回True或False"""
    dialog = ExitDialog(options)
    if dialog.exec_():
        return True, dialog.exitflag
    else:
        return False, dialog.exitflag

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    styleoptions = {
        'title': u'退出设置',
        'windowicon': "../skin/images/ov-orange-green.png",
        'minsize': (400, 300),
        'size': (400, 300),
        'logo_title': u'智能光纤云终端管理平台',
        'logo_img_url': "../skin/images/ov-orange-green.png"
    }
    print(exit(styleoptions))
    sys.exit(app.exec_())
