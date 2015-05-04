#!/usr/bin/python
# -*- coding: utf-8 -*-

import logging
import os
import sys
from logging.handlers import RotatingFileHandler
from logging import RootLogger
import traceback


class ALERootLogger(RootLogger):

    def __init__(self, level):
        super(ALERootLogger, self).__init__(level)

    def error(self, msg, *args, **kwargs):
        _t = traceback.format_exc()
        if _t == "None\n":
            msg = msg
        else:
            msg = _t[:-1]
        super(ALERootLogger, self).error(msg, *args, **kwargs)

def handle_exception(exc_type, exc_value, exc_traceback):
    info = ''.join(traceback.format_exception(exc_type, exc_value, exc_traceback))
    ALERootLogger.senderror(repr("Uncaught exception %s" % info))


logging.root = ALERootLogger(logging.WARNING)
logging.root.setLevel(logging.INFO)
logging.root.propagate = 0
#log write in file
logpath = os.sep.join([os.getcwd(), 'log', 'main.log'])
fh = RotatingFileHandler(logpath, maxBytes=10 * 1024 * 1024, backupCount=100)
fh.setLevel(logging.INFO)
#log write in console
ch = logging.StreamHandler()
ch.setLevel(logging.INFO)
#log formatter
formatter = logging.Formatter('%(asctime)s %(levelname)8s [%(filename)s%(lineno)06s] %(message)s')
fh.setFormatter(formatter)
ch.setFormatter(formatter)
logging.root.addHandler(fh)
# logging.root.addHandler(ch)

logger = logging.root
logger.propagate = 0
