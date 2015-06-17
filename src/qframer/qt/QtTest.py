"""
Provides QtTest and functions

.. warning:: PySide is not supported here, that's why there is not unit tests
    running with PySide.

"""
import os
from . import QT_API
from . import PYQT5_API
from . import PYQT4_API
from . import PYSIDE_API

if os.environ[QT_API] == PYQT5_API:
    from PyQt5.QtTest import QTest
elif os.environ[QT_API] == PYQT4_API:
    from PyQt4.QtTest import QTest as OldQTest

    class QTest(OldQTest):
        @staticmethod
        def qWaitForWindowActive(QWidget):
            OldQTest.qWaitForWindowShown(QWidget)
elif os.environ[QT_API] == PYSIDE_API:
    raise ImportError('QtTest support is incomplete for PySide')
