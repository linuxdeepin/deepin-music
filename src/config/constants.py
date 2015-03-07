#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

parentDir = os.path.dirname

Home = os.path.expanduser('~')
ConfigPath = os.path.join(Home, '.config')
ProjectName = "DeepinMusic3"
ProjectConfigPath = os.path.join(ConfigPath, ProjectName)

PlaylistPath = os.path.join(ProjectConfigPath, 'playlist')

MusicDBFile = os.path.join(parentDir(os.getcwd()), 'music', 'music.db')


if not os.path.exists(ProjectConfigPath): 
	os.makedirs(ProjectConfigPath)

if not os.path.exists(PlaylistPath): 
	os.makedirs(PlaylistPath)
