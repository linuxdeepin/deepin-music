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


if not os.path.exists(ProjectPath):
    os.makedirs(ProjectPath)

if not os.path.exists(PlaylistPath):
    os.makedirs(PlaylistPath)

if not os.path.exists(CoverPath):
	os.makedirs(CoverPath)
