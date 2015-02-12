#!/usr/bin/python
# -*- coding: utf-8 -*-

import functools

contexts = {}

def registerContext(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        self = args[0]
        if hasattr(self, '__contextName__'):
            contexts.update({self.__contextName__: self})
        func(*args, **kwargs)
    return wrapper
