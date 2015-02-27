#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty
from PyQt5.QtGui import QCursor
from .utils import registerContext

from PyQt5.QtMultimedia import QMediaPlayer


class MediaPlayer(QMediaPlayer):

	__contextName__ = "MediaPlayer"

	@registerContext
	def __init__(self):
		super(MediaPlayer, self).__init__()


# player = new QMediaPlayer;
# connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
# player->setMedia(QUrl::fromLocalFile("/Users/me/Music/coolsong.mp3"));
# player->setVolume(50);
# player->play();
