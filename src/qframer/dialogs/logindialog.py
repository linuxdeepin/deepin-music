#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
from qframer.qt.QtCore import *
from qframer.qt.QtGui import *

if __name__ == '__main__':
    from basedialog import BaseDialog
else:
    from .basedialog import BaseDialog


class LoginDialog(BaseDialog):
    def __init__(self, styleoptions, parent=None):
        super(LoginDialog, self).__init__(styleoptions, parent)

        self.login_np = QWidget()
        login_np_mainlayout = QGridLayout()
        login_nameLabel = QLabel(u'用户名')
        self.login_name = QLineEdit(self)
        self.login_name.setPlaceholderText(u'用户名')

        login_passwordLabel = QLabel(u'密码')
        self.login_password = QLineEdit(self)
        self.login_password.setEchoMode(QLineEdit.Password)
        self.login_password.setPlaceholderText(u'密码')

        login_np_mainlayout.addWidget(login_nameLabel, 0, 0)
        login_np_mainlayout.addWidget(self.login_name, 0, 1)
        login_np_mainlayout.addWidget(login_passwordLabel, 1, 0)
        login_np_mainlayout.addWidget(self.login_password, 1, 1)
        self.login_np.setLayout(login_np_mainlayout)

        #确认按钮布局
        self.enterwidget = QWidget()
        self.pbLogin = QPushButton(u'登录', self)
        self.pbCancel = QPushButton(u'取消', self)
        self.pbLogin.setFixedHeight(30)
        self.pbCancel.setFixedHeight(30)

        self.pbLogin.clicked.connect(self.login)
        self.pbCancel.clicked.connect(self.reject)

        enterwidget_mainlayout = QGridLayout()
        enterwidget_mainlayout.addWidget(self.pbLogin, 0, 0)
        enterwidget_mainlayout.addWidget(self.pbCancel, 0, 1)
        self.enterwidget.setLayout(enterwidget_mainlayout)

        self.layout().addWidget(self.login_np)
        self.layout().addWidget(self.enterwidget)
        self.resize(self.width(), self.height())

    def login(self):
        self.accept()  # 关闭对话框并返回1


def login(loginoptions):
    """返回True或False"""
    dialog = LoginDialog(loginoptions)
    if dialog.exec_():
        return True, (dialog.login_name.text(), dialog.login_password.text())
    else:
        return False, (u'', u'')


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    styleoptions = {
        'title': u'登录',
        'windowicon': "../skin/images/ov-orange-green.png",
        'minsize': (400, 300),
        'size': (400, 300),
        'logo_title': u'智能光纤云终端管理平台',
        'logo_img_url': "../skin/images/ov-orange-green.png",
    }
    print(login(styleoptions))
    sys.exit(app.exec_())
