#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QCoreApplication, Qt, QObject, pyqtSignal, pyqtSlot, QUrl, QThread

if os.name == 'posix':
    QCoreApplication.setAttribute(Qt.AA_X11InitThreads, True)

from dwidgets import QSingleApplication
from views import MainWindow
import config


class PyUtil(QObject):

    playUrl = pyqtSignal(unicode)

    def __init__(self, quickItems, parent=None):
        super(PyUtil, self).__init__(parent)

        self.quickItems = quickItems
        self.quickItems['webMusic360Page'].playMusicByID.connect(self.getMusicURLByID)
        self.playUrl.connect(self.quickItems['MainMusic'].playMusic)

    @pyqtSlot(int, result=str)
    def md5(self, musicId):
        import hashlib
        s = 'id=%d_projectName=linuxdeepin' % (musicId)
        md5Value = hashlib.md5(s)
        return md5Value.hexdigest()

    @pyqtSlot(int)
    def getMusicURLByID(self, musicId):
        import hashlib
        import requests
        sign = self.md5(musicId)
        params = {
            'id': musicId,
            'src': 'linuxdeepin',
            'sign': sign
        }
        ret = requests.get("http://s.music.haosou.com/player/songForPartner", params=params)
        print(ret.json())
        jsonRet = ret.json()

        self.playUrl.emit(jsonRet['playlinkUrl'])


if __name__ == '__main__':
    app = QSingleApplication(config.applicationName, sys.argv)
    if app.isRunning():
        sys.exit(0)
    mainWindow = MainWindow()
    mainWindow.setSource(QUrl('views/Main.qml'))
    app.setActivationWindow(mainWindow)
    mainWindow.show()

    workThread = QThread()
    worker = PyUtil(mainWindow.quickItems)
    worker.moveToThread(workThread)
    workThread.start()

    exitCode = app.exec_()
    
    sys.exit(exitCode)
