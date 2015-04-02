#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
from qframer.qt.QtCore import *
from qframer.qt.QtGui import *

if __name__ == '__main__':
    from basedialog import BaseDialog
else:
    from .basedialog import BaseDialog


class UrlinputDialog(BaseDialog):
    def __init__(self, styleoptions, parent=None):
        super(UrlinputDialog, self).__init__(styleoptions, parent)

        # url内容输入
        self.urlwidget = QWidget()
        url_mainlayout = QGridLayout()
        self.urlLabel = QLabel(u'请输入需要访问的url:')
        self.urlLineEdit = QLineEdit(u'http://192.168.10.135:8000/webs/protection_areas/list')
        url_mainlayout.addWidget(self.urlLabel, 0, 0)
        url_mainlayout.addWidget(self.urlLineEdit, 1, 0)
        self.urlwidget.setLayout(url_mainlayout)

        #确认按钮布局
        self.enterwidget = QWidget()
        self.pbEnter = QPushButton(u'确定', self)
        self.pbCancel = QPushButton(u'取消', self)
        self.pbEnter.clicked.connect(self.enter)
        self.pbCancel.clicked.connect(self.reject)
        enterwidget_mainlayout = QGridLayout()
        enterwidget_mainlayout.addWidget(self.pbEnter, 0, 0)
        enterwidget_mainlayout.addWidget(self.pbCancel, 0, 1)
        self.enterwidget.setLayout(enterwidget_mainlayout)

        self.layout().addWidget(self.urlwidget)
        self.layout().addWidget(self.enterwidget)
        self.resize(self.width(), self.height())

    def enter(self):
        self.accept()  # 关闭对话框并返回1


def urlinput(options):
    dialog = UrlinputDialog(options)
    if dialog.exec_():
        return True, unicode(dialog.urlLineEdit.text())
    else:
        return False, unicode(dialog.urlLineEdit.text())


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
    print(urlinput(styleoptions))
    sys.exit(app.exec_())
