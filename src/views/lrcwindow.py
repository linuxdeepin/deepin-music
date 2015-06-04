#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
from PyQt5.QtCore import (
    Qt, QRect,QUrl,
    pyqtProperty, QObject, 
    pyqtSlot, pyqtSignal, 
    QTimer, QEvent, QPoint, QSize)
from PyQt5.QtGui import QColor, QPen, QLinearGradient, QPainter, QFont, QPalette, QRegion
from PyQt5.QtWidgets import (QApplication, QLabel, QFrame, 
    QPushButton, QDesktopWidget, QHBoxLayout,
    QVBoxLayout)
from deepin_utils.file import get_parent_dir
from controllers import registerContext, contexts, registerObj
from controllers import signalManager


class FMoveableWidget(QLabel):

    qSizeChanged = pyqtSignal('QSize')
    qPositionChanged = pyqtSignal('QPoint')

    def __init__(self, locked, parent=None):
        super(FMoveableWidget, self).__init__(parent)
        if locked:
            self.setAttribute(Qt.WA_TransparentForMouseEvents, True)
        else:
            self.setAttribute(Qt.WA_TransparentForMouseEvents, False)
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setAttribute(Qt.WA_Hover, True)
        self.setWindowFlags(Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)

    @pyqtProperty('QPoint', notify=qPositionChanged)
    def qPosition(self):
        return self.pos()

    @qPosition.setter
    def qPosition(self, pos):
        self.move(pos)
        self.qPositionChanged.emit(pos)

    def setPosition(self, pos):
        self.qPosition = pos

    @pyqtProperty('QSize', notify=qSizeChanged)
    def qSize(self):
        return self.size()

    @qSize.setter
    def qSize(self, size):
        self.setFixedSize(size)
        self.qSizeChanged.emit(size)

    def moveCenter(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def mouseMoveEvent(self, event):
        if hasattr(self, "dragPosition"):
            if event.buttons() == Qt.LeftButton:
                pos = event.globalPos() - self.dragPosition
                self.qPosition = pos
                self.setAttribute(Qt.WA_TranslucentBackground, False)

    def mousePressEvent(self, event):
        self.setFocus()
        # 鼠标点击事件
        if event.button() == Qt.LeftButton:
            self.dragPosition = event.globalPos() - \
                self.frameGeometry().topLeft()
        super(FMoveableWidget, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        # 鼠标释放事件
        if hasattr(self, "dragPosition"):
            del self.dragPosition
            self.isSideClicked = False
            self.setCursor(Qt.ArrowCursor)
        super(FMoveableWidget, self).mousePressEvent(event)

    def resizeEvent(self, event):
        self.qSizeChanged.emit(event.size())
        super(FMoveableWidget, self).resizeEvent(event)


class UnLockWindow(FMoveableWidget):

    hoverChanged = pyqtSignal(bool)

    def __init__(self, locked=False, parent=None):
        super(UnLockWindow, self).__init__(locked, parent)

        self.initData()
        self.initUI()
        self.initConnect()
        self.installEventFilter(self)

    def initData(self):
        self._hovered = False

    def initUI(self):
        self.unLockBar = UnLockToolBar()

        barLayout = QHBoxLayout()
        barLayout.setAlignment(Qt.AlignHCenter)
        barLayout.addWidget(self.unLockBar)
        barLayout.setContentsMargins(0, 0, 0, 0)

        self.lrcText = DLrcText()
        lrcLayout = QHBoxLayout()
        lrcLayout.addWidget(self.lrcText)
        lrcLayout.setContentsMargins(50, 0, 50, 0)

        mainLayout = QVBoxLayout()
        mainLayout.addLayout(barLayout)
        mainLayout.addStretch()
        mainLayout.addLayout(lrcLayout)
        mainLayout.setSpacing(0)
        mainLayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(mainLayout)

        self.unLockBar.setVisible(False)
        self.setFixedSize(901, 90)

    def initConnect(self):
        self.hoverChanged.connect(self.lrcText.setHovered)
        self.hoverChanged.connect(self.unLockBar.setVisible)
        self.qSizeChanged.connect(self.lrcText.updateTextSize)

        self.lrcText.qSizeChanged.connect(self.updateSize)

    def updateSize(self, size):
        self.setFixedSize(self.width(), self.unLockBar.height() + self.lrcText.height())

    def updateText(self, text, percentage, lyric_id):
        self.lrcText.updateText(text, percentage, lyric_id)

    @pyqtProperty(bool, notify=hoverChanged)
    def hovered(self):
        return  self._hovered

    @hovered.setter
    def hovered(self, hovered):
        self._hovered = hovered
        self.hoverChanged.emit(hovered)

    @pyqtSlot('QPoint')
    def move(self, pos):
        super(UnLockWindow, self).move(pos)

    def eventFilter(self, obj, event):
        if event.type() == QEvent.HoverEnter:
            self.hovered = True
        elif event.type() == QEvent.HoverLeave:
            self.hovered = False
        return super(FMoveableWidget, self).eventFilter(obj, event)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        if self.hovered:
            color = QColor(0, 0, 0, 90)
            painter.fillRect(0, 0, self.width(), self.height(), color)
        super(UnLockWindow, self).paintEvent(event)


class LockWindow(FMoveableWidget):

    def __init__(self, locked=False, parent=None):
        super(LockWindow, self).__init__(locked, parent)

        self.initData()
        self.initUI()
        self.initConnect()

    def initData(self):
        pass

    def initUI(self):

        self.lrcText = DLrcText()
        lrcLayout = QHBoxLayout()
        lrcLayout.addWidget(self.lrcText)
        lrcLayout.setContentsMargins(50, 0, 50, 0)

        self.setLayout(lrcLayout)
        self.setFixedSize(901, 40)

    def initConnect(self):
        # self.qSizeChanged.connect(self.lrcText.setSize)
        pass

    @pyqtSlot('QPoint')
    def move(self, pos):
        super(LockWindow, self).move(pos)

class DLrcText(QLabel):

    qSizeChanged = pyqtSignal('QSize')
    hoverChanged = pyqtSignal(bool)

    def __init__(self, parent=None):
        super(DLrcText, self).__init__(parent)
        self.initData()
        self.initUI()
        self.initConnect()

    def initData(self):
        self._hovered = False

        self.linear_gradient = QLinearGradient()
        self.linear_gradient.setStart(0, 10)
        self.linear_gradient.setFinalStop(0, 40)
    
        self.linear_gradient.setColorAt(0.1, QColor(14, 179, 255));
        self.linear_gradient.setColorAt(0.5, QColor(114, 232, 255));
        self.linear_gradient.setColorAt(1, QColor(14, 179, 255));

        self.mask_linear_gradient = QLinearGradient()
        self.mask_linear_gradient.setStart(0, 10)
        self.mask_linear_gradient.setFinalStop(0, 40)
        self.mask_linear_gradient.setColorAt(0.1, QColor(0, 100, 40))
        self.mask_linear_gradient.setColorAt(0.5, QColor(0, 72, 16))
        self.mask_linear_gradient.setColorAt(1, QColor(0, 255, 40))

        self.text = ""
        self.percentage = 0
        self.lyric_id = 0

        self.line1_text = ''
        self.line1_percentage = 0

        self.line2_text = ''
        self.line2_percentage = 0

        self.lrcfont = QFont()
        self.lrcfont.setPixelSize(30)
        self.setFont(self.lrcfont)

        self._lineMode = 1

    def initUI(self):
        pass

    def initConnect(self):
        signalManager.singleTextInfoChanged.connect(self.updateSingleText)
        signalManager.douleTextInfoChanged.connect(self.updateDoubleText)

        signalManager.noLrcFound.connect(self.updateNoLrcTip)
        signalManager.fontIncreaseChanged.connect(self.increaseFont)
        signalManager.fontDecreaseChanged.connect(self.decreaseFont)

        signalManager.showLrcSingleLine.connect(self.setSingleLine)
        signalManager.showLrcDoubleLine.connect(self.setDoubleLine)

    def disConnect(self):
        signalManager.singleTextInfoChanged.disconnect(self.updateSingleText)
        signalManager.douleTextInfoChanged.disconnect(self.updateDoubleText)

        signalManager.noLrcFound.disconnect(self.updateNoLrcTip)
        signalManager.fontIncreaseChanged.disconnect(self.increaseFont)
        signalManager.fontDecreaseChanged.disconnect(self.decreaseFont)

        signalManager.showLrcSingleLine.disconnect(self.setSingleLine)
        signalManager.showLrcDoubleLine.disconnect(self.setDoubleLine)

    def increaseFont(self):
        font = self.font()
        size = font.pixelSize()
        font.setPixelSize(size + 1)
        self.setFont(font)
        height = self.height()
        self.qSize = QSize(self.width(), height + 1)

    def decreaseFont(self):
        font = self.font()
        size = font.pixelSize()
        font.setPixelSize(size - 1)
        self.setFont(font)
        self.qSize = QSize(self.width(), self.height() - 1)

    def setSingleLine(self):
        if self._lineMode == 1:
            return
        self._lineMode = 1
        self.qSize = QSize(self.width(), self.height() / 2)
        signalManager.lineModeChanged.emit(self._lineMode)
        

    def setDoubleLine(self):
        if self._lineMode == 2:
            return
        self._lineMode = 2
        self.qSize = QSize(self.width(), self.height() * 2)
        signalManager.lineModeChanged.emit(self._lineMode)
    
    @pyqtProperty('QSize', notify=qSizeChanged)
    def qSize(self):
        return self.size()

    @qSize.setter
    def qSize(self, size):
        self.setFixedSize(size)
        self.qSizeChanged.emit(size)

    def setSize(self, size):
        self.qSize = size

    @pyqtProperty(bool, notify=hoverChanged)
    def hovered(self):
        return  self._hovered

    @hovered.setter
    def hovered(self, hovered):
        self._hovered = hovered
        self.hoverChanged.emit(hovered)

    def setHovered(self, hovered):
        self.hovered = hovered

    def updateNoLrcTip(self):
        self.updateSingleText('No lyric found', 0, 0)

    def updateSingleText(self, text, percentage, lyric_id):
        self.text = text
        self.percentage = percentage
        self.lyric_id = lyric_id
        self.update()

    def updateDoubleText(self, texts):
        self.line1_text = texts[0][0]
        self.line1_percentage = texts[0][1]

        self.line2_text = texts[1][0]
        self.line2_percentage = texts[1][1]
        self.update()

    def updateTextSize(self, size):
        self.setFixedSize(size.width() - 100, self.height())

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setFont(self.font())
        if self._lineMode == 1:
            self.drawText(painter, self.text, self.percentage)
        if self._lineMode == 2:
            self.drawText(painter, self.line1_text, self.line1_percentage, mode="Double", lineMode=1)
            self.drawText(painter, self.line2_text, self.line2_percentage, mode="Double", lineMode=2)

        super(DLrcText, self).paintEvent(event)

    def drawText(self, painter, text, percentage, mode='Single', lineMode=1):
        textWidth = self.fontMetrics().width(text)
        textHeight = self.fontMetrics().height()
        startX = 0
        startY = 0
        if mode == 'Single':
            startX = (self.width() - textWidth) / 2
            startY = (self.height() - textHeight) / 2
        elif mode == 'Double' and lineMode == 1:
            startX = (self.width() - textWidth) / 2  - self.width() / 4
            startY = (self.height() / 2 - textHeight) / 2
        elif mode == 'Double' and lineMode == 2:
            startX = (self.width() - textWidth) / 2 + self.width() / 4
            startY = self.height() / 2 + (self.height() / 2 - textHeight) / 2

        painter.setPen(QColor(0, 0, 0, 200))
        painter.drawText(startX + 1, startY + 1, textWidth, textHeight, Qt.AlignLeft, text)


        painter.setPen(QPen(self.linear_gradient, 0))
        painter.drawText(startX, startY, textWidth, textHeight, Qt.AlignLeft, text)

        self.mask_linear_gradient.setStart(startX, startY)
        self.mask_linear_gradient.setFinalStop(startX, startY + textHeight)

        painter.setPen(QPen(self.mask_linear_gradient, 0))
        painter.drawText(startX, startY, textWidth * percentage , textHeight, Qt.AlignLeft, text)


class DToolButton(QPushButton):
    """docstring for DToolButton"""
    def __init__(self, parent=None):
        super(DToolButton, self).__init__(parent)
        self.setFocusPolicy(Qt.NoFocus)
        self.setFixedSize(16, 16)

    def mousePressEvent(self, event):
        super(DToolButton, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        super(DToolButton, self).mouseReleaseEvent(event)


class UnLockToolBar(QFrame):
    
    style = '''
        QPushButton#PreButton{
            border-image: url(./skin/lrc/lrc.previous.normal.png)

        }
        QPushButton#PreButton:pressed{
            border-image: url(./skin/lrc/lrc.previous.pressed.png)
        }

        QPushButton#PlayButton{
            border-image: url(./skin/lrc/lrc.play.normal.png)
        }
        QPushButton#PlayButton:pressed{
            border-image: url(./skin/lrc/lrc.play.pressed.png)
        }

        QPushButton#PauseButton{
            border-image: url(./skin/lrc/lrc.pause.normal.png)
        }
        QPushButton#PauseButton:pressed{
            border-image: url(./skin/lrc/lrc.pause.pressed.png)
        }

        QPushButton#NextButton{
            border-image: url(./skin/lrc/lrc.next.normal.png)
        }
        QPushButton#NextButton:pressed{
            border-image: url(./skin/lrc/lrc.next.pressed.png)
        }

        QPushButton#FontPlusButton{
            border-image: url(./skin/lrc/lrc.font.increase.normal.png)
        }
        QPushButton#FontPlusButton:pressed{
            border-image: url(./skin/lrc/lrc.font.increase.pressed.png)
        }

        QPushButton#FontMinusButton{
            border-image: url(./skin/lrc/lrc.font.decrease.normal.png)
        }
        QPushButton#FontMinusButton:pressed{
            border-image: url(./skin/lrc/lrc.font.decrease.pressed.png)
        }

        QPushButton#BackButton{
            border-image: url(./skin/lrc/lrc.back.normal.png)
        }
        QPushButton#BackButton:pressed{
            border-image: url(./skin/lrc/lrc.back.pressed.png)
        }

        QPushButton#ForwardButton{
            border-image: url(./skin/lrc/lrc.forward.normal.png)
        }
        QPushButton#ForwardButton:pressed{
            border-image: url(./skin/lrc/lrc.forward.pressed.png)
        }

        QPushButton#ThemeButton{
            border-image: url(./skin/lrc/lrc.theme.normal.png)
        }
        QPushButton#ThemeButton:pressed{
            border-image: url(./skin/lrc/lrc.theme.pressed.png)
        }

        QPushButton#SingleLineButton{
            border-image: url(./skin/lrc/lrc.single.normal.png)
        }
        QPushButton#SingleLineButton:pressed{
            border-image: url(./skin/lrc/lrc.single.pressed.png)
        }

        QPushButton#DoubleLineButton{
            border-image: url(./skin/lrc/lrc.double.normal.png)
        }
        QPushButton#DoubleLineButton:pressed{
            border-image: url(./skin/lrc/lrc.double.pressed.png)
        }

        QPushButton#KalaokButton{
            border-image: url(./skin/lrc/lrc.kalaok.normal.png)
        }
        QPushButton#KalaokButton:pressed{
            border-image: url(./skin/lrc/lrc.kalaok.pressed.png)
        }

        QPushButton#LockButton{
            border-image: url(./skin/lrc/lrc.lock.normal.png)
        }
        QPushButton#LockButton:pressed{
            border-image: url(./skin/lrc/lrc.lock.pressed.png)
        }

        QPushButton#SettingButton{
            border-image: url(./skin/lrc/lrc.setting.normal.png)
        }
        QPushButton#SettingButton:pressed{
            border-image: url(./skin/lrc/lrc.setting.pressed.png)
        }

        QPushButton#SearchButton{
            border-image: url(./skin/lrc/lrc.search.normal.png)
        }
        QPushButton#SearchButton:pressed{
            border-image: url(./skin/lrc/lrc.search.pressed.png)
        }

        QPushButton#CloseButton{
            border-image: url(./skin/lrc/lrc.close.normal.png)
        }
        QPushButton#CloseButton:pressed{
            border-image: url(./skin/lrc/lrc.close.pressed.png)
        }
    '''

    qPositionChanged = pyqtSignal('QPoint')


    def __init__(self, parent=None):
        super(UnLockToolBar, self).__init__(parent)

        self.initData()
        self.initUI()
        self.initConnect()

        self.installEventFilter(self)

    def initData(self):
        pass

    def initUI(self):
        self.preButton =  DToolButton()
        self.preButton.setObjectName('PreButton')
        self.preButton.setStyleSheet(self.style)
        self.playButton = DToolButton()
        self.playButton.setObjectName('PlayButton')
        self.playButton.setFixedSize(24, 24)
        self.nextButton = DToolButton()
        self.nextButton.setObjectName('NextButton')

        self.fontIncreaseButton = DToolButton()
        self.fontIncreaseButton.setObjectName('FontPlusButton')
        self.fontDecreaseButton = DToolButton()
        self.fontDecreaseButton.setObjectName('FontMinusButton')

        self.backButton = DToolButton()
        self.backButton.setObjectName('BackButton')
        self.forwardButton = DToolButton()
        self.forwardButton.setObjectName('ForwardButton')

        self.themeButton = DToolButton()
        self.themeButton.setObjectName('ThemeButton')
        
        self.singleLineButton = DToolButton()
        self.singleLineButton.setObjectName('SingleLineButton')
        self.doubleLineButton = DToolButton()
        self.doubleLineButton.setObjectName('DoubleLineButton')

        self.kalaokButton = DToolButton()
        self.kalaokButton.setObjectName('KalaokButton')

        self.lockButton = DToolButton()
        self.lockButton.setObjectName('LockButton')

        self.settingButton = DToolButton()
        self.settingButton.setObjectName('SettingButton')

        self.searchButton = DToolButton()
        self.searchButton.setObjectName('SearchButton')

        self.closeButton = DToolButton()
        self.closeButton.setObjectName('CloseButton')
        
        playLayout = QHBoxLayout()
        playLayout.addWidget(self.preButton)
        playLayout.addWidget(self.playButton)
        playLayout.addWidget(self.nextButton)
        playLayout.setSpacing(10)
        playLayout.setContentsMargins(0, 0, 0, 0)

        controlLayout = QHBoxLayout()
        controlLayout.addWidget(self.fontIncreaseButton)
        controlLayout.addWidget(self.fontDecreaseButton)
        controlLayout.addWidget(self.backButton)
        controlLayout.addWidget(self.forwardButton)
        controlLayout.addWidget(self.themeButton)
        controlLayout.addWidget(self.singleLineButton)
        controlLayout.addWidget(self.doubleLineButton)
        controlLayout.addWidget(self.kalaokButton)
        controlLayout.addWidget(self.lockButton)
        controlLayout.addWidget(self.settingButton)
        controlLayout.addWidget(self.searchButton)
        controlLayout.addWidget(self.closeButton)
        controlLayout.setSpacing(14)
        controlLayout.setContentsMargins(0, 0, 0, 0)

        mainLayout = QHBoxLayout()
        mainLayout.addLayout(playLayout)
        mainLayout.addLayout(controlLayout)
        mainLayout.setContentsMargins(0, 18, 0, 8)
        mainLayout.setSpacing(14)
        self.setLayout(mainLayout)

        self.setFixedSize(436, 50)
        self.setStyleSheet(self.style)

    def initConnect(self):
        self.preButton.clicked.connect(signalManager.previousSong)
        self.playButton.clicked.connect(self.toggleSong)
        self.nextButton.clicked.connect(signalManager.nextSong)
        self.fontIncreaseButton.clicked.connect(signalManager.fontIncreaseChanged)
        self.fontDecreaseButton.clicked.connect(signalManager.fontDecreaseChanged)

        self.singleLineButton.clicked.connect(signalManager.showLrcSingleLine)
        self.doubleLineButton.clicked.connect(signalManager.showLrcDoubleLine)

        self.lockButton.clicked.connect(signalManager.locked)
        self.settingButton.clicked.connect(signalManager.lrcSetting)
        self.closeButton.clicked.connect(signalManager.lrcClosed)
        signalManager.playingChanged.connect(self.updatePlayButton)

    def toggleSong(self):
        if self.playButton.objectName() == 'PlayButton':
            signalManager.playToggle.emit(False)
            self.playButton.setObjectName('PauseButton')
        elif self.playButton.objectName() == 'PauseButton':
            signalManager.playToggle.emit(True)
            self.playButton.setObjectName('PlayButton')
        self.playButton.setStyleSheet(self.style)

    def updatePlayButton(self, playing):
        if playing:
            self.playButton.setObjectName('PlayButton')
        else:
            self.playButton.setObjectName('PauseButton')
        self.playButton.setStyleSheet(self.style)

    @pyqtProperty('QPoint', notify=qPositionChanged)
    def qPosition(self):
        return self.pos()

    @qPosition.setter
    def qPosition(self, pos):
        self.move(pos)
        self.qPositionChanged.emit(pos)

    def setPosition(self, pos):
        self.qPosition = pos


class LockBar(QFrame):

    style = '''
        QPushButton#unlockButton{
            border-image: url(./skin/lrc/lrc.desktop.lock.normal.png)

        }
        QPushButton#unlockButton:pressed{
            border-image: url(./skin/lrc/lrc.desktop.unlock.normal.png)
        }
    '''

    hoverChanged = pyqtSignal(bool)
    qPositionChanged = pyqtSignal('QPoint')

    def __init__(self, parent=None):
        super(LockBar, self).__init__(parent)
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setAttribute(Qt.WA_Hover, True)
        self.setWindowFlags(Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.initData()
        self.initUI()
        self.initConnect()

        self.installEventFilter(self)

    def initData(self):
        self._hovered = False

    def initUI(self):
        self.unlockButton = DToolButton()
        self.unlockButton.setObjectName('unlockButton')
        self.unlockButton.setFixedSize(32, 32)
        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.unlockButton)
        mainLayout.setSpacing(0)
        mainLayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(mainLayout)

        self.unlockButton.setStyleSheet(self.style)
        self.unlockButton.setVisible(False)
        self.setFixedSize(32, 32)

    def initConnect(self):
        self.unlockButton.clicked.connect(signalManager.unlocked)

    @pyqtProperty('QPoint', notify=qPositionChanged)
    def qPosition(self):
        return self.pos()

    @qPosition.setter
    def qPosition(self, pos):
        self.move(pos)
        self.qPositionChanged.emit(pos)

    def setPosition(self, pos):
        self.qPosition = pos

    @pyqtProperty(bool, notify=hoverChanged)
    def hovered(self):
        return  self._hovered

    @hovered.setter
    def hovered(self, hovered):
        self._hovered = hovered
        self.hoverChanged.emit(hovered)

    def eventFilter(self, obj, event):
        if event.type() == QEvent.HoverEnter:
            self.hovered = True
            self.unlockButton.setVisible(True)
        elif event.type() == QEvent.HoverLeave:
            self.hovered = False
            self.unlockButton.setVisible(False)
        return super(LockBar, self).eventFilter(obj, event)


class LrcWindowManager(QObject):

    def __init__(self, mainWindow):
        super(LrcWindowManager, self).__init__()
        self.mainWindow = mainWindow

        self.initData()
        self.initUI()
        self.initConnect()

    def initData(self):
        self.text = ''
        self.percentage = 0
        self.state = 'Normal'
        self.isVisible = False

    def initUI(self):
        self.unLockWindow = UnLockWindow(False)

    def initConnect(self):
        self.mainWindow.loadSuccessed.connect(self.updateUnLockWindowPosition)
        signalManager.locked.connect(self.showLocked)
        signalManager.unlocked.connect(self.showNoraml)
        signalManager.lrcToggleShow.connect(self.toggleShow)
        signalManager.lrcToggleLock.connect(self.toggleLock)
        signalManager.lrcClosed.connect(self.hide)

    def updateUnLockWindowPosition(self):
        self.unLockWindow.setPosition(self.mainWindow.geometry().bottomLeft() + QPoint(0, 0))

    def showNoraml(self):
        self.unLockWindow.show()
        self.destoryLockWindow()

        self.state = 'Normal'

    def showLocked(self):
        self.lockedWindow = LockWindow(True)
        self.lockBar = LockBar()

        self.updateLockWindow()
        self.lockedWindow.show()
        self.lockBar.show()

        self.unLockWindow.hide()
        self.state = 'Locked'

    def updateLockWindow(self):
        self.lockedWindow.qSize = self.unLockWindow.lrcText.size()
        pos = self.unLockWindow.mapToGlobal(self.unLockWindow.lrcText.geometry().topLeft())
        self.lockedWindow.setPosition(pos)
        x = pos.x() + (self.lockedWindow.width() - self.lockBar.width()) / 2
        y = pos.y() - self.lockBar.height()
        self.lockBar.setPosition(QPoint(x, y))

        self.lockedWindow.lrcText._lineMode = self.unLockWindow.lrcText._lineMode
        self.lockedWindow.lrcText.setFont(self.unLockWindow.lrcText.font())

    def destoryLockWindow(self):
        if hasattr(self, 'lockedWindow'):
            self.lockedWindow.hide()
            self.lockedWindow.lrcText.disConnect()
            del self.lockedWindow
        if hasattr(self, 'lockBar'):
            self.lockBar.hide()
            del self.lockBar

    def show(self):
        self.isVisible = True
        if self.state == 'Normal':
            self.showNoraml()
        elif self.state == 'Locked':
            self.showLocked()

    def hide(self):
        self.isVisible = False
        self.unLockWindow.hide()
        self.destoryLockWindow()

    def toggleShow(self):
        if self.isVisible:
            self.hide()
        else:
            self.show()

    def toggleLock(self):
        if self.state == 'Normal':
            self.showLocked()
        elif self.state == 'Locked':
            self.showNoraml()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    lrcManager = LrcWindowManager()
    lrcManager.toggle()

    exitCode = app.exec_()

    sys.exit(exitCode)
