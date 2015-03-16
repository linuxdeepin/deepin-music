#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

parentDir = os.path.dirname

Home = os.path.expanduser('~')
ConfigPath = os.path.join(Home, '.config')
ProjectName = "DeepinMusic3"
ProjectPath = os.path.join(ConfigPath, ProjectName)

PlaylistPath = os.path.join(ProjectPath, 'playlist')
CoverPath = os.path.join(ProjectPath, 'cover')

MusicDBFile = os.path.join(ProjectPath,  'music.db')


paths = [
	ProjectPath,
	PlaylistPath,
	CoverPath
]

for path in paths:
	if not os.path.exists(path):
	    os.makedirs(path)
