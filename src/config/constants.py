#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

parentDir = os.path.dirname

Home = os.path.expanduser('~')
ConfigPath = os.path.join(Home, '.config')
ProjectName = "DeepinMusic3"
ProjectPath = os.path.join(ConfigPath, ProjectName)

MusicManagerPath = os.path.join(ProjectPath, 'music')
PlaylistPath = os.path.join(ProjectPath, 'playlist')
CoverPath = os.path.join(ProjectPath, 'cover')
ArtistCoverPath = os.path.join(CoverPath, 'artist')
AlbumCoverPath = os.path.join(CoverPath, 'album')
SongCoverPath = os.path.join(CoverPath, 'song')

MusicDBFile = os.path.join(ProjectPath,  'music.db')


paths = [
	ProjectPath,
	MusicManagerPath,
	PlaylistPath,
	CoverPath,
	ArtistCoverPath,
	AlbumCoverPath,
	SongCoverPath
]

for path in paths:
	if not os.path.exists(path):
	    os.makedirs(path)
