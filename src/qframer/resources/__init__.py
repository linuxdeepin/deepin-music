import sys
if sys.version_info.major == 2:
	from .qrc_icons_py27 import *
else:
	from .qrc_icons_py34 import *
