"""
Provides QtDesigner classes and functions.
"""
import os
from . import QT_API
from . import PYQT5_API
from . import PYQT4_API


if os.environ[QT_API] == PYQT5_API:
    from PyQt5.QtDesigner import *
elif os.environ[QT_API] == PYQT4_API:
    from PyQt4.QtDesigner import *
