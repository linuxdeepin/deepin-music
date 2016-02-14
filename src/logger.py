#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

import logging
import re

levelno = logging.INFO
classfilter = []

def setLevelNo(n):
    global levelno
    levelno = ( 100 - (n * 10) )
    
def setFilter(filter_list):    
    global classfilter
    classfilter = filter_list

class MyFilter(logging.Filter):
    def __init__(self, name=""): pass
    def filter(self, record):
        if record.levelno >= levelno: return True
        for filter in classfilter:
            if record.name.startswith(filter): return True
        return False


logger = logging.getLogger("")
logger.setLevel(logging.DEBUG)
logging.addLevelName(100,"DEPRECATED")

# formatter = logging.Formatter('%(levelname)-8s %(name)-30s %(message)s')
formatter = logging.Formatter('%(levelname)-8s %(message)s')

handler = logging.StreamHandler()
handler.setFormatter(formatter)
handler.addFilter(MyFilter())
logger.addHandler(handler)

def objaddr(obj):
    string = object.__repr__(obj)
    m = re.search("at (0x\w+)",string)
    if m: return  m.group(1)[2:]
    return "       "

class Logger(object):

    def set_logname(self, name):
        self.__logname = name

    def get_logname(self):
        if hasattr(self,"__logname") and self.__logname :
            return self.__logname
        else:
            return "%s.%s"%(self.__module__,self.__class__.__name__)

    def logdebug(self, msg, *args, **kwargs): 
        # msg = "%s %s"%(objaddr(self),msg)
        mylogger = logging.getLogger(self.get_logname())
        mylogger.debug(msg, *args, **kwargs)

    def loginfo(self, msg, *args, **kwargs): 
        # msg = "%s  %s"%(objaddr(self),msg)
        mylogger = logging.getLogger(self.get_logname())
        mylogger.info(msg, *args, **kwargs)

    def logwarn(self, msg, *args, **kwargs): 
        # msg = "%s  %s"%(objaddr(self),msg)
        mylogger = logging.getLogger(self.get_logname())
        mylogger.warn(msg, *args, **kwargs)

    def logerror(self, msg, *args, **kwargs): 
        # msg = "%s  %s"%(objaddr(self),msg)
        mylogger = logging.getLogger(self.get_logname())
        mylogger.error(msg, *args, **kwargs)

    def logcritical(self, msg, *args, **kwargs): 
        # msg = "%s  %s"%(objaddr(self),msg)
        mylogger = logging.getLogger(self.get_logname())
        mylogger.critical(msg, *args, **kwargs)

    def logexception(self, msg, *args, **kwargs):
        # msg = "%s  %s"%(objaddr(self),msg)
        mylogger = logging.getLogger(self.get_logname())
        mylogger.exception(msg, *args, **kwargs)

    def logdeprecated(self, msg, *args, **kwargs):
        # msg = "%s  %s"%(objaddr(self),msg)
        mylogger = logging.getLogger(self.get_logname())
        mylogger.log(100,msg, *args, **kwargs)

def newLogger(name):
    l = Logger()
    l.set_logname(name)
    return l
    
