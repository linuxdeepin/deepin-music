#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

parentDir = os.path.dirname

Home = os.path.expanduser('~')
ConfigPath = os.path.join(Home, '.config')
ProjectName = "DeepinMusic3"
ProjectConfigPath = os.path.join(ConfigPath, ProjectName)

PlaylistPath = os.path.join(ProjectConfigPath, 'playlist')


if not os.path.exists(ProjectConfigPath): 
	os.makedirs(ProjectConfigPath)

if not os.path.exists(PlaylistPath): 
	os.makedirs(PlaylistPath)
