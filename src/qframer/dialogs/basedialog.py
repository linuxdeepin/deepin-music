#!/usr/bin/python
# -*- coding: utf-8 -*-
import os

from qframer.qt.QtCore import *
from qframer.qt.QtGui import *

def set_skin(widget, qssfile):
    if os.path.exists(qssfile):
        fd = open(qssfile, "r")
        style = fd.read()
        fd.close()
        widget.setStyleSheet(style)

class BaseDialog(QDialog):

    def __init__(self, styleoptions, parent=None):
        super(BaseDialog, self).__init__(parent)
        title = styleoptions['title']
        windowicon = styleoptions['windowicon']
        minsize = styleoptions['minsize']
        size = styleoptions['size']
        logo_title = styleoptions['logo_title']
        logo_img_url = styleoptions['logo_img_url']

        self.setWindowTitle(title)
        self.setWindowIcon(QIcon(windowicon))  # 设置程序图标
        self.setMinimumSize(minsize[0], minsize[1])
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowSystemMenuHint | Qt.WindowMinimizeButtonHint)  # 无边框， 带系统菜单， 可以最小化

        self.logowidget = DynamicTextWidget(logo_title, logo_img_url)

        # 主布局
        mainlayout = QVBoxLayout()
        mainlayout.addWidget(self.logowidget)
        mainlayout.setContentsMargins(5, 5, 5, 5)
        mainlayout.setSpacing(0)
        self.setLayout(mainlayout)

        self.resize(size[0], size[1])

    def mousePressEvent(self, event):
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - self.frameGeometry().topLeft()
            event.accept()

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition

    def mouseMoveEvent(self, event):
        # 鼠标移动事件
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                self.move(event.globalPos() - self.dragPosition)
                event.accept()


class DynamicTextWidget(QWidget):
    def __init__(self, text, bg, parent=None):
        super(DynamicTextWidget, self).__init__(parent)
        self.setFixedSize(400, 132)
        self.bg = bg
        self.text = text

        newFont = self.font()
        newFont.setPointSize(newFont.pointSize() + 10)
        self.setFont(newFont)

        self.painter = QPainter()

        self.timer = QBasicTimer()

        self.step = 0
        self.timer.start(60, self)

    def paintEvent(self, event):
        self.painter.begin(self)
        self.drawBackground(self.painter)
        self.drawDynamicText(self.painter)
        self.painter.end()

    def set_painterpencolor(self, painter):
        color = QColor()
        import random
        i = random.randint(1, 15)
        color.setHsv((15 - i) * 16, 255, 191)
        painter.setPen(color)

    def drawBackground(self, painter):
        painter.drawPixmap(0, 0, self.width(), self.height(), QPixmap(self.bg))

    def drawDynamicText(self, painter):
        sineTable = (0, 38, 71, 92, 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38)
        metrics = QFontMetrics(self.font())
        x = (self.width() - metrics.width(self.text)) / 2
        y = (self.height() + metrics.ascent() - metrics.descent()) / 2
        color = QColor()

        for i, ch in enumerate(self.text):
            index = (self.step + i) % 16
            color.setHsv((15 - index) * 16, 255, 191)
            painter.setPen(color)
            painter.drawText(x, y - ((sineTable[index] * metrics.height()) / 400), ch)
            x += metrics.width(ch)

    def setText(self, newText):
        self.text = newText

    def setspreed(self, spreed):
        self.spreed = spreed
        self.timer.stop()
        self.timer.start(self.spreed, self)

    def timerEvent(self, event):
        if self.text:
            if event.timerId() == self.timer.timerId():
                self.step += 1
                self.update()
            else:
                super(DynamicTextWidget, self).timerEvent(event)
        else:
            self.timer.stop()


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    styleoptions = {
        'title': u'退出设置',
        'windowicon': "../skin/images/ov-orange-green.png",
        'minsize': (400, 300),
        'size': (400, 300),
        'logo_title': u'dssssssss',
        'logo_img_url': "../skin/images/ov-orange-green.png"
    }
    dialog = BaseDialog(styleoptions)
    dialog.show()
    sys.exit(app.exec_())
