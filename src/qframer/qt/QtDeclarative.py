"""
Provides QtDeclarative classes and functions.
"""
import os
from . import QT_API
from . import PYQT5_API
from . import PYQT4_API
from . import PYSIDE_API

if os.environ[QT_API] == PYQT5_API:
    raise Exception("PyQt5 don't support QtDeclarative")
elif os.environ[QT_API] == PYQT4_API:
    from PyQt4.QtDeclarative import *
elif os.environ[QT_API] == PYSIDE_API:
    from PySide.QtDeclarative import *
