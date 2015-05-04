
"""
Provides QtWebKit classes and functions.
"""
import os
from . import QT_API
from . import PYQT5_API
from . import PYQT4_API
from . import PYSIDE_API

if os.environ[QT_API] == PYQT5_API:
    from PyQt5.QtWebKit import *
    from PyQt5.QtWebKitWidgets import *
elif os.environ[QT_API] == PYQT4_API:
    from PyQt4.QtWebKit import *
elif os.environ[QT_API] == PYSIDE_API:
    from PySide.QtWebKit import *
