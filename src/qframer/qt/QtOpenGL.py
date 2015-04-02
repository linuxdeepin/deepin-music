"""
Provides QtOpenGL classes and functions.
"""
import os
from . import QT_API
from . import PYQT5_API
from . import PYQT4_API
from . import PYSIDE_API

if os.environ[QT_API] == PYQT5_API:
    from PyQt5.QtOpenGL import *
    raise Exception("python3.4 don't support OpenGL")
elif os.environ[QT_API] == PYQT4_API:
    from PyQt4.QtOpenGL import *
elif os.environ[QT_API] == PYSIDE_API:
    from PySide.QtOpenGL import *
