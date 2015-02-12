#!/usr/bin/python
# -*- coding: utf-8 -*-

import threading

def dthread(func):
    import functools
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        t = threading.Thread(target=func, args=args, kwargs=kwargs)
        t.setDaemon(True)
        t.start()
    return wrapper
