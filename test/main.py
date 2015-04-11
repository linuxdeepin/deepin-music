#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
sys.path.insert(0, '../src/dwidgets/')
from PyQt5.QtCore import (QObject, pyqtSignal,
                pyqtSlot, pyqtProperty, QDir, 
                QDirIterator, QTimer, QThread,
                QThreadPool, QAbstractListModel, Qt, QUrl, QModelIndex, QVariant)
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView
from PyQt5.QtQml import QJSValue


from qmodel import ModelMetaclass



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

class ListModel(QObject):

    dataChanged = pyqtSignal('QVariant')

    #qml2py
    qml2Py_appendSignal = pyqtSignal('QVariant')
    qml2py_clearSignal = pyqtSignal()
    qml2py_insertSignal = pyqtSignal(int, 'QVariant')
    qml2py_moveSignal = pyqtSignal(int, int, int)
    qml2py_removeSignal = pyqtSignal(int)
    qml2py_setSignal = pyqtSignal(int, 'QVariant')
    qml2py_setPropertySignal = pyqtSignal(int, 'QString', 'QString')

    #py2qml
    py2qml_appendSignal = pyqtSignal('QVariant')
    py2qml_clearSignal = pyqtSignal()
    py2qml_insertSignal = pyqtSignal(int, 'QVariant')
    py2qml_moveSignal = pyqtSignal(int, int, int)
    py2qml_removeSignal = pyqtSignal(int)
    py2qml_setSignal = pyqtSignal(int, 'QVariant')
    py2qml_setPropertySignal = pyqtSignal(int, 'QString', 'QString')
    

    def __init__(self, dataTye):
        super(ListModel, self).__init__()
        self.dataTye = QmlArtistObject
        self._data = []
        self.initConnect()
        self.initData()

        self.timer = QTimer()
        self.timer.timeout.connect(self.appendData)
        self.timer.timeout.connect(self.updateData)
        # self.timer.start(1000)

        self.appendData()

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

    def appendData(self):
        i = len(self._data)
        objdict = {
            'name': 'qwe%s' % i,
            'count': i,
            'cover': '/home/djf/jjk/',
            'songs': {}
        }
        obj = self.dataTye(**objdict)
        self.append(obj)

    def updateData(self):
        import random
        self.setProperty(0, 'count', random.randrange(0,10000))

    def initConnect(self):
        self.qml2Py_appendSignal.connect(self.append)
        self.qml2py_clearSignal.connect(self.clear)
        self.qml2py_insertSignal.connect(self.insert)
        self.qml2py_moveSignal.connect(self.move)
        self.qml2py_removeSignal.connect(self.remove)
        self.qml2py_setSignal.connect(self.set)
        self.qml2py_setPropertySignal.connect(self.setProperty)        

    @pyqtProperty('QVariant', notify=dataChanged)
    def data(self):
        return self._data

    @data.setter
    def data(self, value):
        self._data = value
        self.dataChanged.emit(value)

    def append(self, obj):
        if isinstance(obj, QJSValue):
            _obj = obj.toVariant()
            obj = self.dataTye(**_obj)
        self._data.append(obj)
        self.py2qml_appendSignal.emit(obj)

    def clear(self):
        del self._data[:]
        self.py2qml_clearSignal.emit()

    def get(self, index):
        if index < len(self._data):
            return self._data[index]
        else:
            return None

    def insert(self, index, obj):
        if isinstance(obj, QJSValue):
            _obj = obj.toVariant()
            obj = self.dataTye(**_obj)
        self._data.insert(index, obj)
        self.py2qml_insertSignal.emit(index, obj)

    def move(self, _from, _to, _n):
        dataMoved = []
        for i in range(_from, _from + _n):
            dataMoved.append(self._data.pop(_from))
        for item in dataMoved:
            index = dataMoved.index(item)
            self._data.insert(_to + index, item)
        self.py2qml_moveSignal.emit(_from, _to, _n)

        a = []
        for obj in self._data:
            a.append(obj.count)
        print a

    def remove(self, index):
        self._data.pop(index)
        self.py2qml_removeSignal.emit(index)

    def set(self, index, obj):
        if index < len(self._data):
            if isinstance(obj, QJSValue):
                _obj = obj.toVariant()
                obj = self.dataTye(**_obj)
            self._data[index] = obj
            self.py2qml_setSignal.emit(index, obj)

    def setProperty(self, index, key, value):
        if index < len(self._data):
            obj = self._data[index]
            if hasattr(obj, key):
                setattr(obj, key, value)
                self.py2qml_setPropertySignal.emit(index, key, value)

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
