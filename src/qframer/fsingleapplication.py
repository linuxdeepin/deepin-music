#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from .qt.QtCore import *
from .qt.QtGui import *
from .qt.QtNetwork import *


class QSingleApplication(QApplication):

    messageReceived = Signal(str)

    def __init__(self, id, *argv):

        super(QSingleApplication, self).__init__(*argv)
        self._id = id
        self._activationWindow = None
        self._activateOnMessage = False
        self._server = None

        # Is there another instance running?
        self._outSocket = QLocalSocket()
        self._outSocket.connectToServer(self._id)
        self._outSocket.error.connect(self.handleError)
        self._isRunning = self._outSocket.waitForConnected()

        if self._outSocket.state() == QLocalSocket.ConnectedState:
            # Yes, there is.
            self._outStream = QTextStream(self._outSocket)
            self._outStream.setCodec('UTF-8')
        else:
            # No, there isn't.
            self._outSocket = None
            self._outStream = None
            self._inSocket = None
            self._inStream = None
            self._server = QLocalServer()
            self._server.listen(self._id)
            self._server.newConnection.connect(self._onNewConnection)
            self.aboutToQuit.connect(self.removeServer)

    def handleError(self, msg):
        print(msg)

    def server(self):
        return self._server

    def isRunning(self):
        return self._isRunning

    def id(self):
        return self._id

    def activationWindow(self):
        return self._activationWindow

    def setActivationWindow(self, activationWindow, activateOnMessage=True):
        self._activationWindow = activationWindow
        self._activateOnMessage = activateOnMessage

    def activateWindow(self):
        if not self._activationWindow:
            print("No registered ActivationWindow")
            return
        # Unfortunately this *doesn't* do much of any use, as it won't
        # bring the window to the foreground under KDE... sigh.
        self._activationWindow.setWindowState(
            self._activationWindow.windowState() & ~Qt.WindowMinimized)
        self._activationWindow.raise_()
        self._activationWindow.activateWindow()

    def sendMessage(self, msg,  msecs=5000):
        if not self._outStream:
            return False
        self._outStream << msg << '\n'
        if not self._outSocket.waitForBytesWritten(msecs):
            raise RuntimeError(
                "Bytes not written within %ss" % (msecs / 1000.))

    def _onNewConnection(self):
        if self._inSocket:
            self._inSocket.readyRead.disconnect(self._onReadyRead)
        self._inSocket = self._server.nextPendingConnection()
        if not self._inSocket:
            return
        self._inStream = QTextStream(self._inSocket)
        self._inStream.setCodec('UTF-8')
        self._inSocket.readyRead.connect(self._onReadyRead)
        if self._activateOnMessage:
            self.activateWindow()

    def _onReadyRead(self):
        while True:
            msg = self._inStream.readLine()
            if not msg:
                break
            print("Message received")
            self.messageReceived.emit(msg)

    def removeServer(self):
        self._server.close()
        self._server.removeServer(self._id)
