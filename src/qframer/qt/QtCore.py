"""
Provides QtCore classes and functions.
"""
import os
from . import QT_API
from . import PYQT5_API
from . import PYQT4_API
from . import PYSIDE_API


if os.environ[QT_API] == PYQT5_API:
    from PyQt5.QtCore import *
    # compatibility with pyside
    from PyQt5.QtCore import pyqtSignal as Signal
    from PyQt5.QtCore import pyqtSlot as Slot
    from PyQt5.QtCore import pyqtProperty as Property
    # use a common __version__
    from PyQt5.QtCore import QT_VERSION_STR as __version__
elif os.environ[QT_API] == PYQT4_API:
    from PyQt4.QtCore import *
    # compatibility with pyside
    from PyQt4.QtCore import pyqtSignal as Signal
    from PyQt4.QtCore import pyqtSlot as Slot
    from PyQt4.QtCore import pyqtProperty as Property
    from PyQt4.QtGui import QSortFilterProxyModel
    # use a common __version__
    from PyQt4.QtCore import QT_VERSION_STR as __version__
elif os.environ[QT_API] == PYSIDE_API:
    from PySide.QtCore import *
    from PySide.QtGui import QSortFilterProxyModel
    # use a common __version__
    import PySide.QtCore
    __version__ = PySide.QtCore.__version__
