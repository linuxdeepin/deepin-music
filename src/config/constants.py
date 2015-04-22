#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from PyQt5.QtCore import QDir

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
OnlineSongCoverPath = os.path.join(CoverPath, 'onlineSong')
LevevDBPath = os.path.join(ProjectPath,  'leveldb')
MusicDBFile = os.path.join(ProjectPath,  'music.db')

DownloadSongPath = os.path.join(QDir.homePath(), u'音乐', 'download')


paths = [
	ProjectPath,
	MusicManagerPath,
	PlaylistPath,
	CoverPath,
	ArtistCoverPath,
	AlbumCoverPath,
	SongCoverPath,
	OnlineSongCoverPath,
	LevevDBPath,
	DownloadSongPath
]

for path in paths:
	if not os.path.exists(path):
	    os.makedirs(path)
