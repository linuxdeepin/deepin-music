#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from PyQt5.QtCore import (QObject, pyqtSignal,
                pyqtSlot, pyqtProperty, QVariant)
from PyQt5.QtQml import QJSValue, qmlRegisterType

# class ListModel(QAbstractListModel):

    # def __init__(self, fields, parent=None):
    #     super(ListModel, self).__init__(parent)
    #     self._roles = {}
    #     for i in fields:
    #         index = fields.index(i)
    #         role = '%sRole' % i[0]
    #         setattr(self, role, Qt.UserRole + index + 1)
    #         self._roles[getattr(self, role)] = i[0]
    #     self._items = []

    # def addItem(self, item):
    #     self.beginInsertRows(QModelIndex(), self.rowCount(), self.rowCount())
    #     self._items.append(item)
    #     self.endInsertRows()

    # def rowCount(self, parent=QModelIndex()):
    #     return len(self._items)

    # def data(self, index, role=Qt.DisplayRole):
    #     try:
    #         item = self._items[index.row()]
    #     except IndexError:
    #         return QVariant()

    #     for key, value in self._roles.items():
    #         if role == key:
    #             return getattr(item, value)

    #     return QVariant()

    # def roleNames(self):
    #     # return self._roles


class DListModel(QObject):

    countChanged = pyqtSignal(int)
    dataChanged = pyqtSignal('QVariant')

    #qml2py
    qml2Py_appendSignal = pyqtSignal('QVariant')
    qml2py_clearSignal = pyqtSignal()
    qml2py_insertSignal = pyqtSignal(int, 'QVariant')
    qml2py_moveSignal = pyqtSignal(int, int, int)
    qml2py_removeSignal = pyqtSignal(int)
    qml2py_setSignal = pyqtSignal(int, 'QVariant')
    qml2py_setPropertySignal = pyqtSignal(int, 'QString', 'QVariant')

    #py2qml
    py2qml_appendSignal = pyqtSignal('QVariant')
    py2qml_clearSignal = pyqtSignal()
    py2qml_insertSignal = pyqtSignal(int, 'QVariant')
    py2qml_moveSignal = pyqtSignal(int, int, int)
    py2qml_removeSignal = pyqtSignal(int)
    py2qml_setSignal = pyqtSignal(int, 'QVariant')
    py2qml_setPropertySignal = pyqtSignal(int, 'QString', 'QVariant')
    

    def __init__(self, dataTye):
        super(DListModel, self).__init__()
        self.dataTye = dataTye
        self._data = []
        self.initConnect()

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

    @pyqtProperty(int, notify=countChanged)
    def count(self):
        return len(self._data)

    def append(self, obj):
        if isinstance(obj, QJSValue):
            _obj = obj.toVariant()
            obj = self.dataTye(**_obj)
        self._data.append(obj)
        self.py2qml_appendSignal.emit(obj)
        self.countChanged.emit(len(self._data))

    def clear(self):
        del self._data[:]
        self.py2qml_clearSignal.emit()
        self.countChanged.emit(len(self._data))

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
        self.countChanged.emit(len(self._data))

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

    def remove(self, index):
        self._data.pop(index)
        self.py2qml_removeSignal.emit(index)
        self.countChanged.emit(len(self._data))

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
