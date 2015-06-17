#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
from qframer.qt.QtCore import *
from qframer.qt.QtGui import *

if __name__ == '__main__':
    from basedialog import BaseDialog
else:
    from .basedialog import BaseDialog


class MessageDialog(BaseDialog):
    def __init__(self, text, styleoptions, parent=None):
        super(MessageDialog, self).__init__(styleoptions, parent)
        # message内容提示
        self.msglabel = QLabel(text)
        self.msglabel.setAlignment(Qt.AlignCenter)
        #确认按钮布局
        self.enterwidget = QWidget()
        self.pbEnter = QPushButton(u'确定', self)
        self.pbEnter.clicked.connect(self.enter)
        self.enter_mainlayout = QGridLayout()
        self.enter_mainlayout.addWidget(self.pbEnter, 0, 0)
        self.enterwidget.setLayout(self.enter_mainlayout)

        self.layout().addWidget(self.msglabel)
        self.layout().addWidget(self.enterwidget)
        self.resize(self.width(), self.height())

    def enter(self):
        self.accept()  # 关闭对话框并返回1


def msg(text, styleoptions):
    """返回True或False"""
    dialog = MessageDialog(text, styleoptions)
    if dialog.exec_():
        return True
    else:
        return False


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    styleoptions = {
        'title': u'消息提示',
        'windowicon': "../skin/images/ov-orange-green.png",
        'minsize': (400, 300),
        'size': (400, 300),
        'logo_title': u'智能光纤云终端管理平台',
        'logo_img_url': "../skin/images/ov-orange-green.png"
    }
    print(msg('sddsdsdsdssddsds', styleoptions))
    sys.exit(app.exec_())
