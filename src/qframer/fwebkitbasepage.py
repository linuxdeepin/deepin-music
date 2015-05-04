#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *
from .qt.QtWebKit import *
from .qt.QtNetwork import *


class FWebkitBasePage(QFrame):

    def __init__(self, parent=None):
        super(FWebkitBasePage, self).__init__(parent)
        self.parent = parent
        QWebSettings.globalSettings().setAttribute(
            QWebSettings.PluginsEnabled, True)

        QNetworkProxyFactory.setUseSystemConfiguration(True)

        self.view = QWebView(self)
        self.view.setFocus()

        self.setupInspector()
        self.splitter = QSplitter(self)
        self.splitter.setOrientation(Qt.Vertical)

        self.splitter.addWidget(self.view)
        self.splitter.addWidget(self.webInspector)

        mainlayout = QVBoxLayout(self)
        mainlayout.addWidget(self.splitter)
        mainlayout.setContentsMargins(0, 0, 0, 0)
        mainlayout.setSpacing(0)
        self.setLayout(mainlayout)

    def setupInspector(self):
        page = self.view.page()
        page.settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)
        self.webInspector = QWebInspector(self)
        self.webInspector.setPage(page)

        shortcut = QShortcut(self)
        shortcut.setKey('F12')
        shortcut.activated.connect(self.toggleInspector)
        self.webInspector.setVisible(False)

    def toggleInspector(self):
        self.webInspector.setVisible(not self.webInspector.isVisible())
