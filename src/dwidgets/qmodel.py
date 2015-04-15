#!/usr/bin/python
# -*- coding: utf-8 -*-

import types
import functools
from PyQt5 import QtCore
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
                          pyqtProperty)
import copy


class FieldExpection(Exception):
    pass


class ValidFormatExpection(Exception):
    pass


class SetJsonExpection(Exception):
    pass


class SetJsonKeyExpection(Exception):
    pass


class ModelMetaclass(type):

    def __new__(cls, clsname, clsbases, clsdict):
        if '__Fields__' in clsdict:
            Fields = clsdict['__Fields__']
        else:
            Fields = ()

        formatFields = []

        for field in Fields:
            if len(field) < 2:
                raise FieldExpection(
                    "Each Field tuple length must be large than 1")
            elif len(field) == 2:
                if field[1] == 'QString':
                    newfield = (field[0], field[1], '')
                else:
                    newfield = (field[0], field[1], field[1]())
            elif len(field) == 3:
                newfield = (field[0], field[1], field[2])
            else:
                raise FieldExpection(
                    "Each Field tuple length must be smaller than 4")
            formatFields.append(newfield)

        Fields = tuple(formatFields)

        Signals = {}

        functions = {}

        for k in copy.deepcopy(clsdict):
            if isinstance(clsdict[k], pyqtSignal):
                Signals.update({k:clsdict[k]})
                clsdict.pop(k)

        class DObject(QtCore.QObject):

            if '__Fields__' in clsdict:
                __Fields__ = clsdict['__Fields__']

            def __init__(self, *args, **kwargs):
                super(DObject, self).__init__()
                self.__class__.__name__ = clsname
                for key, Type, default in Fields:
                    self.__dict__['_' + key] = kwargs.get(key, default)

                self.valid_message = {}

                self.setJsoning = False

                for k in clsdict:
                    setattr(self.__class__, k, clsdict[k])

                if 'initialize' in clsdict:
                    self.initialize(*args, **kwargs)

            @pyqtProperty(str)
            def string(self):
                return str(self)

            def __repr__(self):
                values = ('%s=%r' % (key, self.__dict__['_' + key])
                          for key, value, default in Fields)
                return '<%s (%s)>' % (clsname, ', '.join(values))

            def __getitem__(self, key):
                print key, '+++'
                return self.__dict__['_' + key]

            @pyqtSlot(dict)
            def setDict(self, obj):
                self.setJsoning = True
                self.valid_message = {}
                if isinstance(obj, dict):
                    for key, value, default in Fields:
                        if key in obj:
                            setattr(self, key, obj[key])
                    self.setJsoning = False
                    return self.valid_message
                else:
                    self.setJsoning = False
                    raise SetJsonExpection("obj's type must be dict")

            def getDict(self):
                ret = {}
                for key, value, default in Fields:
                    ret[key] = self.__dict__['_' + key]
                return ret

            locals().update(Signals)

            for key, Type, default in Fields:
                if Type in [dict, list]:
                    Type = 'QVariant'
                if Type is str:
                    Type = 'QString'
                locals()[key + "Changed"] = QtCore.pyqtSignal(Type)

                def _get(key):
                    def f(self):
                        return self.__dict__['_' + key]
                    return f

                def _set(key):
                    def f(self, value):
                        if not self.setJsoning:
                            self.valid_message = {}
                        validmethod = 'valid_' + key
                        if validmethod in clsdict:
                            method = clsdict[validmethod]
                        else:
                            def valid_defaut(self, v):
                                return True
                            method = valid_defaut
                        valid_return = method(self, value)

                        if valid_return is True:
                            error, validFlag = (
                                "set %s=%s valid ok" % (key, value), True)
                        elif valid_return is False:
                            error, validFlag = (
                                "set %s=%s valid error" % (key, value), False)
                        else:
                            if(len(valid_return) == 2):
                                validFlag, error = valid_return
                                if isinstance(validFlag, bool) and \
                                        isinstance(error, str):
                                    pass
                                else:
                                    error, validFlag = (
                                        "def valid_%s function error." % key,
                                        False)
                                    raise ValidFormatExpection(
                                        "def valid_%s function error." % key)
                            else:
                                error, validFlag = (
                                    "def valid_%s function error." % key,
                                    False)
                                raise ValidFormatExpection(
                                    "def valid_%s function error." % key)

                        if validFlag:
                            self.__dict__['_' + key] = value
                            getattr(self, key + "Changed").emit(value)

                        else:
                            self.valid_message[key] = error
                    return f

                set = _set(key)
                get = _get(key)

                locals()[key] = pyqtProperty(Type, get, set, notify=locals()[key + "Changed"])

        return DObject


class Object_Dict(dict):

    '''
        Makes a dictionary behave like an object.
    '''

    def __init__(self, *args, **kw):
        dict.__init__(self, *args, **kw)
        self.__dict__ = self


# class Car(object):

#     __metaclass__ = ModelMetaclass

#     __Fields__ = (
#         ('model', str, "123"),
#         ('brand', str, "456"),
#         ('year', int),
#         ('inStock', bool),
#         ('d', dict, {'a': 1111})
#     )

#     def valid_model(self, value):
#         return True

#     def valid_brand(self, value):
#         return False


# def slot(n):
#     print("get data from signal:", n)


# def test(func):
#     import functools

#     @functools.wraps(func)
#     def wrapper(*args, **kwagrs):
#         print("============ %s start =============" % func.func_name)
#         func(*args, **kwagrs)
#         print("============ %s end   =============\n" % func.func_name)
#     return wrapper


# @test
# def test_Json():
#     car = Car(model="1111")
#     print Car.model, car.model
#     obj = {
#         'model': "8888",
#         'brand': "+++"
#     }
#     car.setJson(obj)
#     print(car.valid_message)
#     print(car.getJson())
#     print(car)


# @test
# def test_attr():
#     car = Car(model="1111")
#     car.model = "333"
#     print(car.valid_message)
#     print(car)


# @test
# def test_slot():
#     car = Car(model="1111")
#     car.modelChanged.connect(slot)
#     car.model = "111"
#     print(car)


# if __name__ == '__main__':
#     car = Car()
#     print car.__class__
