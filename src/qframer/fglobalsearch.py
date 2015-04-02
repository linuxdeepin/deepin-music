#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *
from .fmoveablewidget import FMoveableWidget


class FGlobalSearchWidget(FMoveableWidget):

    style = '''
        QFrame#FGlobalSearchWidget{
            border: 10px solid rgb(40, 47, 63);
            background-color: rgb(40, 47, 63);
        }
        QLineEdit#search{
            border: 2px solid rgb(69, 187, 217);
            font: 30px;
            color: black;
        }
    '''

    def __init__(self, parent=None):
        super(FGlobalSearchWidget, self).__init__(parent)
        self.parent = parent
        self.setObjectName("FGlobalSearchWidget")
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setWindowFlags(Qt.ToolTip)
        self.initData()
        self.initUI()
        self._initShowAnimation()
        self._initHideAnimation()

    def initData(self):
        pass

    def initUI(self):
        self.setFixedSize(600, 60)
        self.searchEdit = QLineEdit(self)
        self.searchEdit.setObjectName("search")
        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.searchEdit)
        mainLayout.setContentsMargins(0, 0, 0, 0)
        mainLayout.setSpacing(0)
        self.setLayout(mainLayout)

        desktopWidth = QDesktopWidget().availableGeometry().width()
        self.move((desktopWidth - self.width()) / 2, 50)

        self.setStyleSheet(self.style)

    def _initShowAnimation(self):
        self.showanimation = QPropertyAnimation(self, 'windowOpacity')
        self.showanimation.setStartValue(0)
        self.showanimation.setEndValue(1)
        self.showanimation.setDuration(1000)
        self.showanimation.setEasingCurve(QEasingCurve.OutCubic)

    def _initHideAnimation(self):
        self.hideanimation = QPropertyAnimation(self, 'windowOpacity')
        self.hideanimation.setStartValue(1)
        self.hideanimation.setEndValue(0)
        self.hideanimation.setDuration(1000)
        self.hideanimation.setEasingCurve(QEasingCurve.OutCubic)
        self.hideanimation.finished.connect(self.close)

    def animationShow(self):
        self.show()
        self.showanimation.start()

    def animationHide(self):
        self.hideanimation.start()

    def leaveEvent(self, event):
        self.setCursor(Qt.ArrowCursor)

    def enterEvent(self, event):
        self.setCursor(Qt.PointingHandCursor)

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            self.animationHide()
