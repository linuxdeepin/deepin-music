#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from PyQt5.QtCore import (QObject, pyqtSignal,
                pyqtSlot, pyqtProperty, QDir, 
                QDirIterator, QTimer, QThread,
                QThreadPool, QAbstractListModel, Qt, QUrl, QModelIndex, QVariant)
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView
from PyQt5.QtQml import QJSValue


from qmodel import ModelMetaclass
from dlistmodel import DListModel


class QmlArtistObject(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('name', 'QString'),
        ('count', int),
        ('cover', 'QString'),
        ('songs', dict),
    )

    def initialize(self, *agrs, **kwargs):
        self.setDict(kwargs)

class ListModel(DListModel):

    def __init__(self, dataTye):
        super(ListModel, self).__init__(dataTye)
        self.initData()

    def initData(self):
        objs = []
        for i in range(10):
            objdict = {
                'name': 'qwe%s' % i,
                'count': i,
                'cover': '/home/djf/jjk/',
                'songs': {'song': '1111'}
            }
            obj = self.dataTye(**objdict)
            objs.append(obj)

        self.data = objs


if __name__ == '__main__':
    import sys

    app = QGuiApplication(sys.argv)

    view = QQuickView()
    view.setResizeMode(QQuickView.SizeRootObjectToView)
    ctxt = view.rootContext()
    myListModel = ListModel(QmlArtistObject)
    ctxt.setContextProperty('myListModel', myListModel)

    view.setSource(QUrl('ListShow.qml'))
    view.show()

    sys.exit(app.exec_())
