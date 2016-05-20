#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

from dtk.ui.theme import Theme, ui_theme
from dtk.ui.skin_config import skin_config
import os
from deepin_utils.file import get_parent_dir
from constant import FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT, PROGRAM_VERSION

# Init skin config.
skin_config.init_skin(
    "sky",
    os.path.join(get_parent_dir(__file__, 3), "skin"),
    os.path.expanduser("~/.config/deepin-music-player/skin"),
    os.path.expanduser("~/.config/deepin-music-player/skin_config.ini"),
    "dmusic",
    PROGRAM_VERSION
    )

# Create application theme.
app_theme = Theme(
    os.path.join(get_parent_dir(__file__, 3), "app_theme"),
    os.path.expanduser("~/.config/deepin-music-player/theme")
    )

# Set theme.
skin_config.load_themes(ui_theme, app_theme)
skin_config.set_application_window_size(FULL_DEFAULT_WIDTH, FULL_DEFAULT_HEIGHT)

