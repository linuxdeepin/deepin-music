#!/usr/bin/env python
# -*- coding: utf-8 -*-

from widget.lyrics import ScrollLyricsWindow
import gtk

a = ScrollLyricsWindow()
a.lyrics_win.show_all()
a.set_whole_lyrics("桍夺奇趣深度在桍枯奇趣极")
a.set_text("深度播放器dsf dsf sdf dsf sdf ")

gtk.main()

