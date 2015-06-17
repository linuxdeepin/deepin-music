#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from PyQt5.QtCore import QStandardPaths


parentDir = os.path.dirname

Home = os.path.expanduser('~')
ConfigPath = os.path.join(Home, '.config')
ProjectName = "deepin-muisc"
ProjectPath = os.path.join(ConfigPath, ProjectName)

MusicManagerPath = os.path.join(ProjectPath, 'music')
PlaylistPath = os.path.join(ProjectPath, 'playlist')
CoverPath = os.path.join(ProjectPath, 'cover')
ArtistCoverPath = os.path.join(CoverPath, 'artist')
AlbumCoverPath = os.path.join(CoverPath, 'album')
SongCoverPath = os.path.join(CoverPath, 'song')
OnlineSongCoverPath = os.path.join(CoverPath, 'onlineSong')
OnlineAlbumCoverPath = os.path.join(CoverPath, 'onlineAlbum')
LevevDBPath = os.path.join(ProjectPath,  'leveldb')
MusicDBFile = os.path.join(ProjectPath,  'music.db')

DownloadSongPath = os.path.join(QStandardPaths.writableLocation(QStandardPaths.MusicLocation), 'download')
LRCPath = os.path.join(QStandardPaths.writableLocation(QStandardPaths.MusicLocation), 'lyrics')
LogCachePath = os.path.join(Home, '.cache', 'deepin-muisc')
LogPath = os.path.join(LogCachePath, 'main.log')

paths = [
	ProjectPath,
	MusicManagerPath,
	PlaylistPath,
	CoverPath,
	ArtistCoverPath,
	AlbumCoverPath,
	SongCoverPath,
	OnlineSongCoverPath,
	OnlineAlbumCoverPath,
	LevevDBPath,
	DownloadSongPath,
	LRCPath,
	LogCachePath
]

for path in paths:
	if not os.path.exists(path):
	    os.makedirs(path)
