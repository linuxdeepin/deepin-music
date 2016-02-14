#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

from widget.playlist import playlist_ui
from widget.local_browser import local_browser
from widget.tab_box import ListTab
from nls import _

local_search_tab =  ListTab(_("Library"), playlist_ui, local_browser)
