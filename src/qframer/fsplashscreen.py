#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
from .qt.QtCore import *
from .qt.QtGui import *


class FSplashScreen(QSplashScreen):

    def __init__(self, t, splash_image):
        if not isinstance(splash_image, QPixmap):
            image = QPixmap(splash_image)
        else:
            image = splash_image
        super(FSplashScreen, self).__init__(image)    # 启动程序的图片
        self.setWindowModality(Qt.ApplicationModal)
        self.fadeTicker(t)
        QApplication.instance().processEvents()

    def fadeTicker(self, keep_t):
        self.setWindowOpacity(0)
        t = 0
        while t <= 50:
            newOpacity = self.windowOpacity() + 0.02   # 设置淡入
            if newOpacity > 1:
                break
            self.setWindowOpacity(newOpacity)
            self.show()
            t -= 1
            time.sleep(0.04)
        self.show()
        time.sleep(keep_t)
        t = 0
        while t <= 50:
            newOpacity = self.windowOpacity() - 0.02   # 设置淡出
            if newOpacity < 0:
                self.close()
                break
            self.setWindowOpacity(newOpacity)
            self.show()
            t += 1
            time.sleep(0.04)
