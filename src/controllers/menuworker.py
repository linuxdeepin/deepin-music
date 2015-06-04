#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, pyqtProperty, QPoint, QUrl
from PyQt5.QtGui import QCursor, QDesktopServices
from .utils import registerContext, openLocalUrl
from deepin_menu.menu import *
from dwidgets import ModelMetaclass
from log import logger
from .playlistworker import PlaylistWorker
from .signalmanager import signalManager


class MenuI18nWorker(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('addMusic', 'QString', u'添加歌曲'),
        ('simpleMode', 'QString', u'简洁模式'),
        ('fullMode', 'QString', u'完整模式'),
        ('file', 'QString', u'文件'),
        ('folder', 'QString', u'文件夹'),
        ('miniMode', 'QString', u'迷你模式'),
        ('checkUpdate', 'QString', u'查看新特性'),
        ('setting', 'QString', u'设置'),
        ('exit', 'QString', u'退出'),
        ('play', 'QString', u'播放'),
        ('pause', 'QString', u'暂停'),
        ('download', 'QString', u'下载'),
        ('temporary', 'QString', u'试听歌单'),
        ('favorite', 'QString', u'我的收藏'),
        ('addToSinglePlaylist', 'QString', u'添加到歌单'),
        ('addToMutiPlaylist', 'QString', u'添加到多个歌单'),
        ('newPlaylist', 'QString', u'新建歌单'),
        ('removeFromDatabase', 'QString', u'从歌库中移除'),
        ('removeFromDriver', 'QString', u'从硬盘中移除'),
        ('removeFromPlaylist', 'QString', u'从列表中移除'),
        ('changeCover', 'QString', u'更换封面'),
        ('order', 'QString', u'排序'),
        ('orderBySongName', 'QString', u'按歌曲名'),
        ('orderByArtist', 'QString', u'按歌手'),
        ('orderByAlbum', 'QString', u'按专辑'),
        ('orderByDuration', 'QString', u'曲长'),
        ('orderByPlayCount', 'QString', u'按播放次数'),
        ('orderByAddTime', 'QString', u'按添加时间'),
        ('orderByFileSize', 'QString', u'按文件大小'),
        ('openFolder', 'QString', u'打开目录'),
        ('information', 'QString', u'信息'),
        ('playAll', 'QString', u'播放全部'),
        ('rename', 'QString', u'重命名'),
        ('downloadAll', 'QString', u'下载全部'),
        ('exportPlaylist', 'QString', u'导出歌单'),
        ('importPlaylist', 'QString', u'导入歌单'),
        ('deletePlaylist', 'QString', u'删除歌单'),
        ('startDownload', 'QString', u'开始下载'),
        ('pauseDownload', 'QString', u'暂停下载'),
        ('deleteDownload', 'QString', u'删除下载'),
        ('preivousSong', 'QString', u'上一首'),
        ('nextSong', 'QString', u'下一首'),
        ('playbackMode', 'QString', u'播放模式'),
        ('random', 'QString', u'随机播放'),
        ('sequential', 'QString', u'顺序播放'),
        ('currentItemInLoop', 'QString', u'单曲循环'),
        ('windowMode', 'QString', u'窗口模式'),
        ('showDesktopLrc', 'QString', u'打开桌面歌词'),
        ('hideDesktopLrc', 'QString', u'关闭桌面歌词'),
        ('lockDesktopLrc', 'QString', u'锁定桌面歌词'),
        ('unlockDesktopLrc', 'QString', u'解锁桌面歌词'),
    )

    __contextName__ = "MenuI18nWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        pass

menuI18nWorker = MenuI18nWorker()


SettingMenuItems = [
    ('AddMusic', menuI18nWorker.addMusic, (), [("File", menuI18nWorker.file), ("Folder", menuI18nWorker.folder)]),
    None,
    ('WindowMode', menuI18nWorker.simpleMode),
    ('MiniMode', menuI18nWorker.miniMode),
    None,
    ('CheckUpdate', menuI18nWorker.checkUpdate),
    ('Setting', menuI18nWorker.setting),
    None,
    ('Exit', menuI18nWorker.exit)
]


ArtistMenuItems = [
    ('Play', menuI18nWorker.play),
    None,
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist, (), []),
    ('AddToMutiPlaylist', menuI18nWorker.addToMutiPlaylist),
    ('NewPlaylist', menuI18nWorker.newPlaylist),
    None,
    ('RemoveFromDatabase', menuI18nWorker.removeFromDatabase),
    ('RemoveFromDriver', menuI18nWorker.removeFromDriver)
]


AlbumMenuItems = [
    ('Play', menuI18nWorker.play),
    None,
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist, (), []),
    ('AddToMutiPlaylist', menuI18nWorker.addToMutiPlaylist),
    ('NewPlaylist', menuI18nWorker.newPlaylist),
    None,
    ('RemoveFromDatabase', menuI18nWorker.removeFromDatabase),
    ('RemoveFromDriver', menuI18nWorker.removeFromDriver)
]


SongMenuItems = [
    ('Play', menuI18nWorker.play),
    None,
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist, (), []),
    ('AddToMutiPlaylist', menuI18nWorker.addToMutiPlaylist),
    ('NewPlaylist', menuI18nWorker.newPlaylist),
    None,
    ('Order', menuI18nWorker.order, (), [
        CheckableMenuItem('Order_group:radio:OrderBySongName', menuI18nWorker.orderBySongName),
        CheckableMenuItem('Order_group:radio:OrderByArtist', menuI18nWorker.orderByArtist),
        CheckableMenuItem('Order_group:radio:OrderByAlbum', menuI18nWorker.orderByAlbum),
        CheckableMenuItem('Order_group:radio:OrderByDuration', menuI18nWorker.orderByDuration),
        CheckableMenuItem('Order_group:radio:OrderByPlayCount', menuI18nWorker.orderByPlayCount),
        CheckableMenuItem('Order_group:radio:OrderByAddTime', menuI18nWorker.orderByAddTime, True),
        CheckableMenuItem('Order_group:radio:OrderByFileSize', menuI18nWorker.orderByFileSize),
        ]),
    None,
    ('RemoveFromDatabase', menuI18nWorker.removeFromDatabase),
    ('RemoveFromDriver', menuI18nWorker.removeFromDriver),
    None,
    ('OpenFolder', menuI18nWorker.openFolder),
    ('Information', menuI18nWorker.information)
]


FolderMenuItems = [
    ('Play', menuI18nWorker.play),
    None,
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist, (), []),
    ('AddToMutiPlaylist', menuI18nWorker.addToMutiPlaylist),
    ('NewPlaylist', menuI18nWorker.newPlaylist),
    None,
    ('RemoveFromDatabase', menuI18nWorker.removeFromDatabase),
    ('RemoveFromDriver', menuI18nWorker.removeFromDriver),
    None,
    ('OpenFolder', menuI18nWorker.openFolder)
]


PlaylistLocalSongMenuItems = [
    ('Play', menuI18nWorker.play),
    None,
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist, (), []),
    ('AddToMutiPlaylist', menuI18nWorker.addToMutiPlaylist),
    ('NewPlaylist', menuI18nWorker.newPlaylist),
    None,
    ('RemoveFromPlaylist', menuI18nWorker.removeFromPlaylist),
    None,
    ('OpenFolder', menuI18nWorker.openFolder),
    ('Information', menuI18nWorker.information)
]


PlaylistOnlineSongMenuItems = [
    ('Play', menuI18nWorker.play),
    ('Download', menuI18nWorker.download),
    None,
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist, (), []),
    ('AddToMutiPlaylist', menuI18nWorker.addToMutiPlaylist),
    ('NewPlaylist', menuI18nWorker.newPlaylist),
    None,
    ('RemoveFromPlaylist', menuI18nWorker.removeFromPlaylist),
    None,
    ('Information', menuI18nWorker.information)
]


PlaylistMenuItems = [
    ('PlayAll', menuI18nWorker.playAll),
    None,
    ('Rename', menuI18nWorker.rename),
    ('DownloadAll', menuI18nWorker.downloadAll),
    None,
    ('ExportPlaylist', menuI18nWorker.exportPlaylist),
    ('ImportPlaylist', menuI18nWorker.importPlaylist),
    None,
    ('DeletePlaylist', menuI18nWorker.deletePlaylist),
]

FTPlaylistMenuItems = [
    ('PlayAll', menuI18nWorker.playAll),
    ('DownloadAll', menuI18nWorker.downloadAll),
]


TemporaryMenuItems = [
    ('Play', menuI18nWorker.play),
    ('RemoveFromPlaylist', menuI18nWorker.removeFromPlaylist),
]

DownloadMenuItems = [
    ('StartDownload', menuI18nWorker.startDownload),
    ('Play', menuI18nWorker.play),
    None,
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist),
    None,
    ('DeleteDownload', menuI18nWorker.deleteDownload)
]

SearchLocalMenuItems = [
    ('Play', menuI18nWorker.play),
    ('AddToSinglePlaylist', menuI18nWorker.addToSinglePlaylist, (), []),
    ('Information', menuI18nWorker.information)
]

SystemTrayMenuItems = [
    ('Play', menuI18nWorker.play),
    ('Previous', menuI18nWorker.preivousSong),
    ('Next', menuI18nWorker.nextSong),
    ('PlaybackMode', menuI18nWorker.playbackMode, (), [
        CheckableMenuItem('PlaybackMode_group:radio:Random', menuI18nWorker.random),
        CheckableMenuItem('PlaybackMode_group:radio:Loop', menuI18nWorker.sequential),
        CheckableMenuItem('PlaybackMode_group:radio:CurrentItemInLoop', menuI18nWorker.currentItemInLoop),
    ]),
    ('WindowMode', menuI18nWorker.windowMode,(), [
        CheckableMenuItem('WindowMode_group:radio:FullMode', menuI18nWorker.fullMode),
        CheckableMenuItem('WindowMode_group:radio:SimpleMode', menuI18nWorker.simpleMode),
        CheckableMenuItem('WindowMode_group:radio:MiniMode', menuI18nWorker.miniMode),
    ]),
    ('DesktopLrcVisible', menuI18nWorker.showDesktopLrc),
    ('DesktopLrcLockStatus', menuI18nWorker.lockDesktopLrc),
    None,
    ('Exit', menuI18nWorker.exit)
]


class DMenu(Menu):
    """docstring for DMenu"""
    def __init__(self, items):
        super(DMenu, self).__init__(items)

    def show(self, style='normal'):
        if style == 'normal':
            self.showRectMenu(QCursor.pos().x(), QCursor.pos().y())
        elif style == 'dock':
            self.showDockMenu(QCursor.pos().x(), QCursor.pos().y())


class MenuWorker(QObject):

    __contextName__ = 'MenuWorker'

    settingMenuShow = pyqtSignal('QString')
    artistMenuShow = pyqtSignal('QString')
    albumMenuShow = pyqtSignal('QString')
    songMenuShow = pyqtSignal('QString', 'QString')
    folderMenuShow = pyqtSignal('QString')
    playlistLocalSongMenuShow = pyqtSignal('QString', 'QString')
    playlistOnlineSongMenuShow = pyqtSignal('QString', 'QString', int)
    playlistNavigationMenuShow = pyqtSignal('QString', int)
    ftPlaylistNavigationMenuShow = pyqtSignal('QString')
    temporaryMenuShowed = pyqtSignal('QString', 'QString')
    downloadMenuShowed = pyqtSignal(int, bool)

    #setting Menu
    simpleTrigger = pyqtSignal()
    fullTrigger = pyqtSignal()
    miniTrigger = pyqtSignal()
    addSongFile = pyqtSignal()
    addSongFolder = pyqtSignal()
    settingTrigger = pyqtSignal()
    exitTrigger = pyqtSignal()

    #Artist Menu
    playArtist = pyqtSignal('QString')
    removeFromDatabaseByArtistName = pyqtSignal('QString')
    removeFromDriverByArtistName = pyqtSignal('QString')

    #Album Menu
    playAlbum = pyqtSignal('QString')
    removeFromDatabaseByAlbumName = pyqtSignal('QString')
    removeFromDriverByAlbumName = pyqtSignal('QString')

    #Song Menu
    orderByKey = pyqtSignal('QString', 'QString')
    openSongFolder = pyqtSignal('QString')
    removeFromDatabaseByUrl = pyqtSignal('QString')
    removeFromDriveByUrl = pyqtSignal('QString')

    #Folder Menu
    playFolder = pyqtSignal('QString')
    removeFromDatabaseByFolderName = pyqtSignal('QString')
    removeFromDriverByFolderName = pyqtSignal('QString')

    #playlist menu:
    playMusicByUrl = pyqtSignal('QString')
    removeFromPlaylist = pyqtSignal('QString', 'QString')

    #playlist navigation menu
    playFTAllSongs = pyqtSignal('QString')
    playNavigationAllSongs = pyqtSignal('QString', int)
    deletePlaylist = pyqtSignal('QString')

    # temporary menu
    playMusicInTemporary = pyqtSignal('QString')

    #download menu
    switchDownloadedStatus = pyqtSignal(int, bool)
    playMusicByIdSignal = pyqtSignal(int)
    removeFromDownloadList = pyqtSignal(int)

    #search menu
    searchLocalSongShowed = pyqtSignal('QString')

    #systemTray:
    systemTrayMenuShowed = pyqtSignal()

    #public menu:
    addSongToPlaylist = pyqtSignal('QString', 'QString')
    addSongsToPlaylist = pyqtSignal('QString', 'QString', 'QString')

    @registerContext
    def __init__(self):
        super(MenuWorker, self).__init__()
        self._artist = ''
        self._album = ''
        self._url = ''
        self._folder = ''
        self._modelType = ''
        self._windowMode = ''
        self._playlist = ''
        self._playlistNavigationIndex = 0
        self._songId = 0
        self._downloaded = False
        self._isDownload = False
        self.createSettingMenu()
        self.createArtistMenu()
        self.createAlbumMenu()
        self.createSongMenu()
        self.createFolderMenu()
        self.createPlaylistSongMenu()
        self.createPlaylistNavgationMenu()
        self.createFTPlaylistNavgationMenu()
        self.createTemporaryMenu()
        self.createDownloadMenu()
        self.createSearchLocalMenu()
        self.createSystemTrayMenu()

    def createSettingMenu(self):
        self.settingMenu = DMenu(SettingMenuItems)
        self.settingMenu.itemClicked.connect(self.settingMenuConnection)
        self.settingMenuShow.connect(self.showSettingMenu)

    def showSettingMenu(self, windowMode):
        self._windowMode = windowMode
        if windowMode == 'MainWindow':
            self.settingMenu.setItemText('WindowMode', menuI18nWorker.simpleMode)
        elif windowMode == 'SimpleWindow':
            self.settingMenu.setItemText('WindowMode', menuI18nWorker.fullMode)
        self.settingMenu.show()

    def createArtistMenu(self):
        self.artistMenu = DMenu(ArtistMenuItems)
        self.artistMenu.itemClicked.connect(self.artistMenuConnection)
        self.artistMenuShow.connect(self.showArtistMenu)

    def showArtistMenu(self, artist):
        self._artist = artist
        self.addPlaylistMenuItemsForMusicManager(self.artistMenu)
        self.artistMenu.show()

    def createAlbumMenu(self):
        self.albumMenu = DMenu(AlbumMenuItems)
        self.albumMenu.itemClicked.connect(self.albumMenuConnection)
        self.albumMenuShow.connect(self.showAlbumMenu)

    def showAlbumMenu(self, album):
        self._album = album
        self.addPlaylistMenuItemsForMusicManager(self.albumMenu)
        self.albumMenu.show()

    def createSongMenu(self):
        self.songMenu = DMenu(SongMenuItems)
        self.songMenu.itemClicked.connect(self.songMenuConnection)
        self.songMenuShow.connect(self.showSongMenu)

    def showSongMenu(self, modelType, url):
        self._modelType = modelType
        self._url = url
        self.addPlaylistMenuItemsForMusicManager(self.songMenu)
        self.songMenu.show()

    def createFolderMenu(self):
        self.folderMenu = DMenu(FolderMenuItems)
        self.folderMenu.itemClicked.connect(self.folderMenuConnection)
        self.folderMenuShow.connect(self.showFolderMenu)

    def showFolderMenu(self, folder):
        self._folder = folder
        self.addPlaylistMenuItemsForMusicManager(self.folderMenu)
        self.folderMenu.show()

    def createPlaylistSongMenu(self):
        self.playlistLocalSongMenu = DMenu(PlaylistLocalSongMenuItems)
        self.playlistOnlineSongMenu = DMenu(PlaylistOnlineSongMenuItems)
        self.playlistLocalSongMenu.itemClicked.connect(self.playlistLocalSongMenuConnection)
        self.playlistOnlineSongMenu.itemClicked.connect(self.playlistOnlineSongMenuConnection)
        

        self.playlistLocalSongMenuShow.connect(self.showPlaylistLocalSongMenu)
        self.playlistOnlineSongMenuShow.connect(self.showPlaylistOnlineSongMenu)

    def showPlaylistLocalSongMenu(self, playlist, url):
        self._playlist = playlist
        self._url = url
        self.addPlaylistMenuItemsForPlaylist(self.playlistLocalSongMenu)
        self.playlistLocalSongMenu.show()

    def showPlaylistOnlineSongMenu(self, playlist, url, songId):
        self._playlist = playlist
        self._url = url
        self._songId = songId            
        self.addPlaylistMenuItemsForPlaylist(self.playlistOnlineSongMenu)
        self.playlistOnlineSongMenu.show()

    def createPlaylistNavgationMenu(self):
        self.playlistNavigationMenu = DMenu(PlaylistMenuItems)
        self.playlistNavigationMenu.itemClicked.connect(self.playlistNavigationMenuConnection)
        self.playlistNavigationMenuShow.connect(self.showPlaylistNavigationMenu)

    def showPlaylistNavigationMenu(self, playlistName, index):
        self._playlist = playlistName
        self._playlistNavigationIndex = index
        self.playlistNavigationMenu.show()

    def createFTPlaylistNavgationMenu(self):
        self.ftPlaylistNavigationMenu = DMenu(FTPlaylistMenuItems)
        self.ftPlaylistNavigationMenu.itemClicked.connect(self.ftPlaylistNavigationMenuConnection)
        self.ftPlaylistNavigationMenuShow.connect(self.showFTPlaylistNavgationMenu)

    def showFTPlaylistNavgationMenu(self, playlistName):
        self._playlist = playlistName
        self.ftPlaylistNavigationMenu.show()

    def createTemporaryMenu(self):
        self.temporaryMenu = DMenu(TemporaryMenuItems)
        self.temporaryMenu.itemClicked.connect(self.temporaryMenuConnection)
        self.temporaryMenuShowed.connect(self.showTemporaryMenu)

    def showTemporaryMenu(self, playlistName, url):
        self._playlist = playlistName
        self._url = url
        self.temporaryMenu.show()

    def createDownloadMenu(self):
        self.downloadMenu = DMenu(DownloadMenuItems)
        self.downloadMenu.itemClicked.connect(self.downloadMenuConnection)
        self.downloadMenuShowed.connect(self.showDownloadMenu)

    def showDownloadMenu(self, songId, downloaded):
        self._songId =songId
        self._downloaded = downloaded
        if self._downloaded:
            self.downloadMenu.setItemText('StartDownload', menuI18nWorker.pauseDownload)
        else:
            self.downloadMenu.setItemText('StartDownload', menuI18nWorker.startDownload)
        self.downloadMenu.show()

    def createSearchLocalMenu(self):
        self.searchLocalMenu = DMenu(SearchLocalMenuItems)
        self.searchLocalMenu.itemClicked.connect(self.searchLocalMenuConnection)
        self.searchLocalSongShowed.connect(self.showSearchLocalMenu)

    def showSearchLocalMenu(self, url):
        self._url = url
        self.addPlaylistMenuItemsForMusicManager(self.searchLocalMenu)
        self.searchLocalMenu.show()

    def createSystemTrayMenu(self):
        self.systemTrayMenu = DMenu(SystemTrayMenuItems)
        self.systemTrayMenu.itemClicked.connect(self.systemTrayMenuConnection)
        self.systemTrayMenuShowed.connect(self.showSystemTrayMenu)

    def showSystemTrayMenu(self):
        from mediaplayer import mediaPlayer
        from windowmanageworker import windowManageWorker
        from app import DeepinPlayer

        if mediaPlayer.playing:
            self.systemTrayMenu.getItemById('Play').text = menuI18nWorker.pause
        else:
            self.systemTrayMenu.getItemById('Play').text = menuI18nWorker.play

        if mediaPlayer.playbackMode == 4:
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:Random').checked = True
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:Loop').checked = False
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:CurrentItemInLoop').checked = False
        elif mediaPlayer.playbackMode == 3:
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:Random').checked = False
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:Loop').checked = True
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:CurrentItemInLoop').checked = False
        elif mediaPlayer.playbackMode == 1:
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:Random').checked = False
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:Loop').checked = False
            self.systemTrayMenu.getItemById('PlaybackMode_group:radio:CurrentItemInLoop').checked = True

        if windowManageWorker.windowMode == 'Full':
            self.systemTrayMenu.getItemById('WindowMode_group:radio:FullMode').checked = True
            self.systemTrayMenu.getItemById('WindowMode_group:radio:SimpleMode').checked = False
            self.systemTrayMenu.getItemById('WindowMode_group:radio:MiniMode').checked = False
        elif windowManageWorker.windowMode == 'Simple':
            self.systemTrayMenu.getItemById('WindowMode_group:radio:FullMode').checked = False
            self.systemTrayMenu.getItemById('WindowMode_group:radio:SimpleMode').checked = True
            self.systemTrayMenu.getItemById('WindowMode_group:radio:MiniMode').checked = False
        elif windowManageWorker.windowMode == 'Mini':
            self.systemTrayMenu.getItemById('WindowMode_group:radio:FullMode').checked = False
            self.systemTrayMenu.getItemById('WindowMode_group:radio:SimpleMode').checked = False
            self.systemTrayMenu.getItemById('WindowMode_group:radio:MiniMode').checked = True

        lrcWindowManager = DeepinPlayer.instance().lrcWindowManager
        if lrcWindowManager.isVisible:
            self.systemTrayMenu.getItemById('DesktopLrcVisible').text = menuI18nWorker.hideDesktopLrc
        else:
            self.systemTrayMenu.getItemById('DesktopLrcVisible').text = menuI18nWorker.showDesktopLrc

        if lrcWindowManager.state == 'Locked':
            self.systemTrayMenu.getItemById('DesktopLrcLockStatus').text = menuI18nWorker.unlockDesktopLrc
        else:
            self.systemTrayMenu.getItemById('DesktopLrcLockStatus').text = menuI18nWorker.lockDesktopLrc

        self.systemTrayMenu.show(style='normal')

    def settingMenuConnection(self, menuId, checked):
        if menuId == 'WindowMode':
            if self._windowMode == 'MainWindow':
                self.simpleTrigger.emit()
            elif self._windowMode == 'SimpleWindow':
                self.fullTrigger.emit()
        elif menuId == "MiniMode":
            self.miniTrigger.emit()
        elif menuId == "File":
            self.addSongFile.emit()
        elif menuId == "Folder":
            self.addSongFolder.emit()
        elif menuId == "Setting":
            self.settingTrigger.emit()
        elif menuId == "Exit":
            self.exitTrigger.emit()

    def artistMenuConnection(self, menuId, checked):
        if menuId == "Play":
            self.playArtist.emit(self._artist)
        elif menuId == "RemoveFromDatabase":
            self.removeFromDatabaseByArtistName.emit(self._artist)
        elif menuId == "RemoveFromDriver":
            self.removeFromDriverByArtistName.emit(self._artist)
        elif menuId == "NewPlaylist":
            signalManager.newPlaylistDialogShowed.emit()
        elif menuId == "AddToMutiPlaylist":
            signalManager.newMultiPlaylistDialogShowed.emit(self._artist, 'Artist')
        else:
            if menuId.startswith('playlist_group'):
                playlistName = menuId.split(':')[-1]
                if playlistName in PlaylistWorker._playlists:
                    self.addSongsToPlaylist.emit(self._artist, playlistName, 'Artist')

    def albumMenuConnection(self, menuId, checked):
        if menuId == "Play":
            self.playAlbum.emit(self._album)
        elif menuId == "RemoveFromDatabase":
            self.removeFromDatabaseByAlbumName.emit(self._album)
        elif menuId == "RemoveFromDriver":
            self.removeFromDriverByAlbumName.emit(self._album)
        elif menuId == "NewPlaylist":
            signalManager.newPlaylistDialogShowed.emit()
        elif menuId == "AddToMutiPlaylist":
            signalManager.newMultiPlaylistDialogShowed.emit(self._album, 'Album')
        else:
            if menuId.startswith('playlist_group'):
                playlistName = menuId.split(':')[-1]
                if playlistName in PlaylistWorker._playlists:
                    self.addSongsToPlaylist.emit(self._album, playlistName, 'Album')

    def songMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            signalManager.playMusicByLocalUrl.emit(self._url)
        elif menuId == 'OpenFolder':
            self.openSongFolder.emit(self._url)
        elif menuId == "Order_group:radio:OrderBySongName":
            self.orderByKey.emit(self._modelType, 'title')
        elif menuId == "Order_group:radio:OrderByArtist":
            self.orderByKey.emit(self._modelType, 'artist')
        elif menuId == "Order_group:radio:OrderByAlbum":
            self.orderByKey.emit(self._modelType, 'album')
        elif menuId == "Order_group:radio:OrderByDuration":
            self.orderByKey.emit(self._modelType, 'duration')
        elif menuId == "Order_group:radio:OrderByPlayCount":
            self.orderByKey.emit(self._modelType, 'playCount')
        elif menuId == "Order_group:radio:OrderByAddTime":
            self.orderByKey.emit(self._modelType, 'created_date')
        elif menuId == "Order_group:radio:OrderByFileSize":
            self.orderByKey.emit(self._modelType, 'size')
        elif menuId == "RemoveFromDatabase":
            self.removeFromDatabaseByUrl.emit(self._url)
        elif menuId == "RemoveFromDriver":
            self.removeFromDriveByUrl.emit(self._url)
        elif menuId == "NewPlaylist":
            signalManager.newPlaylistDialogShowed.emit()
        elif menuId == "AddToMutiPlaylist":
            signalManager.newMultiPlaylistDialogShowed.emit(self._url, 'Song')
        elif menuId == 'Information':
            signalManager.informationShow.emit(self._url)
        else:
            if menuId.startswith('playlist_group'):
                playlistName = menuId.split(':')[-1]
                if playlistName in PlaylistWorker._playlists:
                    self.addSongToPlaylist.emit(self._url, playlistName)

        if menuId.startswith('Order'):
            subMenuItems =  self.songMenu.getItemById('Order').subMenu.items
            self.updateCheckableItems(subMenuItems, menuId)

    def updateCheckableItems(self, subMenuItems, checkedID):
        for menuItem in subMenuItems:
            if menuItem.id == checkedID:
                menuItem.checked = True
            else:
                menuItem.checked = False

    def folderMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            self.playFolder.emit(self._folder)
        elif menuId == 'OpenFolder':
           openLocalUrl(self._folder)
        elif menuId == "RemoveFromDatabase":
            self.removeFromDatabaseByFolderName.emit(self._folder)
        elif menuId == "RemoveFromDriver":
            self.removeFromDriverByFolderName.emit(self._folder)
        elif menuId == "NewPlaylist":
            signalManager.newPlaylistDialogShowed.emit()
        elif menuId == "AddToMutiPlaylist":
            signalManager.newMultiPlaylistDialogShowed.emit(self._folder, 'Folder')
        else:
            if menuId.startswith('playlist_group'):
                playlistName = menuId.split(':')[-1]
                if playlistName in PlaylistWorker._playlists:
                    self.addSongsToPlaylist.emit(self._folder, playlistName, 'Folder')

    def playlistLocalSongMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            self.playMusicByUrl.emit(self._url)
        elif menuId == 'OpenFolder':
            self.openSongFolder.emit(self._url)
        elif menuId == 'RemoveFromPlaylist':
            self.removeFromPlaylist.emit(self._playlist, self._url)
        elif menuId == "NewPlaylist":
            signalManager.newPlaylistDialogShowed.emit()
        elif menuId == "AddToMutiPlaylist":
            signalManager.newMultiPlaylistDialogShowed.emit(self._url, 'Song')
        elif menuId == 'Information':
            signalManager.informationShow.emit(self._url)
        else:
            if menuId.startswith('playlist_group'):
                playlistName = menuId.split(':')[-1]
                if playlistName in PlaylistWorker._playlists:
                    self.addSongToPlaylist.emit(self._url, playlistName)

    def playlistOnlineSongMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            self.playMusicByUrl.emit(self._url)
        elif menuId == 'Download':
            signalManager.addtoDownloadlist.emit(self._songId)
        elif menuId == 'RemoveFromPlaylist':
            self.removeFromPlaylist.emit(self._playlist, self._url)
        elif menuId == "NewPlaylist":
            signalManager.newPlaylistDialogShowed.emit()
        elif menuId == "AddToMutiPlaylist":
            signalManager.newMultiPlaylistDialogShowed.emit(self._url, 'Song')
        else:
            if menuId.startswith('playlist_group'):
                    playlistName = menuId.split(':')[-1]
                    if playlistName in PlaylistWorker._playlists:
                        self.addSongToPlaylist.emit(self._url, playlistName)

    def addPlaylistMenuItemsForMusicManager(self, menu, menuId='AddToSinglePlaylist'):
        playlistSubMenuItems = []
        for key in PlaylistWorker._playlists:
            if key == 'temporary':
                subMenuID = menuI18nWorker.temporary
            elif key == 'favorite':
                subMenuID = menuI18nWorker.favorite
            else:
                subMenuID = key
            playlistSubMenuItems.append((key, subMenuID))
        playlistSubMenu = CheckboxMenu('playlist_group', playlistSubMenuItems)
        menu.getItemById(menuId).setSubMenu(playlistSubMenu)

    def addPlaylistMenuItemsForPlaylist(self, menu, menuId='AddToSinglePlaylist'):
        playlistSubMenuItems = []
        for key in PlaylistWorker._playlists:
            if key != self._playlist:
                if key == 'temporary':
                    subMenuID = menuI18nWorker.temporary
                elif key == 'favorite':
                    subMenuID = menuI18nWorker.favorite
                else:
                    subMenuID = key
                playlistSubMenuItems.append((key, subMenuID))
        playlistSubMenu = CheckboxMenu('playlist_group', playlistSubMenuItems)
        menu.getItemById(menuId).setSubMenu(playlistSubMenu)

    def playlistNavigationMenuConnection(self, menuId, checked):
        if menuId == 'PlayAll':
            self.playNavigationAllSongs.emit(self._playlist, self._playlistNavigationIndex)
        elif menuId == 'DownloadAll':
            signalManager.addAlltoDownloadlist.emit(self._playlist)
        elif menuId == 'DeletePlaylist':
            self.deletePlaylist.emit(self._playlist)

    def ftPlaylistNavigationMenuConnection(self, menuId, checked):
        if menuId == 'PlayAll':
            self.playFTAllSongs.emit(self._playlist)
        elif menuId == 'DownloadAll':
            signalManager.addAlltoDownloadlist.emit(self._playlist)

    def temporaryMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            self.playMusicInTemporary.emit(self._url)
        elif menuId == 'RemoveFromPlaylist':
            self.removeFromPlaylist.emit(self._playlist, self._url)

    def downloadMenuConnection(self, menuId, checked):
        if menuId == 'StartDownload':
            self.switchDownloadedStatus.emit(self._songId, not self._downloaded)
        elif menuId == 'Play':
            self.playMusicByIdSignal.emit(self._songId)
        elif menuId == 'DeleteDownload':
            self.removeFromDownloadList.emit(self._songId)

    def searchLocalMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            signalManager.playMusicByLocalUrl.emit(self._url)
        elif menuId == 'Information':
            signalManager.informationShow.emit(self._url)
        if menuId.startswith('playlist_group'):
                playlistName = menuId.split(':')[-1]
                if playlistName in PlaylistWorker._playlists:
                    self.addSongsToPlaylist.emit(self._album, playlistName, 'Album')

    def systemTrayMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            from mediaplayer import mediaPlayer
            signalManager.playToggle.emit(not mediaPlayer.playing)
        elif menuId == 'Previous':
            signalManager.previousSong.emit()
        elif menuId == 'Next':
            signalManager.nextSong.emit()
        elif menuId == 'PlaybackMode_group:radio:Random':
            signalManager.playbackModeChanged.emit(4)
        elif menuId == 'PlaybackMode_group:radio:Loop':
            signalManager.playbackModeChanged.emit(3)
        elif menuId == 'PlaybackMode_group:radio:CurrentItemInLoop':
            signalManager.playbackModeChanged.emit(1)
        elif menuId == 'WindowMode_group:radio:FullMode':
            signalManager.fullMode.emit()
        elif menuId == 'WindowMode_group:radio:SimpleMode':
            signalManager.simpleMode.emit()
        elif menuId == 'WindowMode_group:radio:MiniMode':
            signalManager.miniMode.emit()
        elif menuId == 'DesktopLrcVisible':
            signalManager.lrcToggleShow.emit()
        elif menuId == 'DesktopLrcLockStatus':
            signalManager.lrcToggleLock.emit()
        elif menuId == 'Exit':
            signalManager.exited.emit()


menuWorker = MenuWorker()

if __name__ == "__main__":
    import sys
    from PyQt5.QtCore import QCoreApplication

    app = QCoreApplication([])

    @pyqtSlot(str, bool)
    def invoked(s, c):
        logger.info("id: ", s, ", checked: ", c)
        # menu.setItemText("id_nonactive", "hello")
        # menu.setItemActivity("id_nonactive", True)

    @pyqtSlot()
    def dismissed():
        app.quit()

    # 1)
    # driver = MenuItem("id_driver", "Driver", "/usr/share/icons/Deepin/apps/16/preferences-driver.png")
    # display = MenuItem("id_display", "Display", "/usr/share/icons/Deepin/apps/16/preferences-display.png")
    # show = Menu()
    # show.addMenuItem(MenuItem("id_sub_display", "Display", "/usr/share/icons/Deepin/apps/16/preferences-display.png"))
    # display.setSubMenu(show)
    # menu = Menu(is_root=True)
    # menu.addMenuItems([driver, display])
    # menu.showMenu(200, 200)

    # 2)
    menu = Menu([("id_driver", "Driver", ("/usr/share/icons/Deepin/apps/16/preferences-display.png",)),
                 None,
                 ("id_display", "_Display", (), [
                  ("display_sub1", "Display One"), ("display_sub2", "Display Two"), ]),
                 ("id_radio", "RadioButtonMenu"),
                 ("id_checkbox", "_CheckBoxMenu"),
                 ("id_extra", "_ExtraTest", (), [], "Ctrl-X"),
                 MenuSeparator(),
                 CheckableMenuItem("radio_group_2:radio:radio2_sub1", "One"),
                 CheckableMenuItem("radio_group_2:radio:radio2_sub2", "Two"),
                 None,
                 CheckableMenuItem(
                     "checkbox_group_2:checkbox:checkbox2_sub1", "One"),
                 CheckableMenuItem(
                     "checkbox_group_2:checkbox:checkbox2_sub2", "Two"),
                 None,
                 MenuItem("id_nonactive", "NotActive", isActive=False),
                 CheckableMenuItem("id_check", "CheckMe", True)], is_root=True,)
    radio_sub = RadioButtonMenu(
        "radio_group_1", [("id_radio1", "Radio One"), ("id_radio2", "Radio Two"), ])
    checkbox_sub = CheckboxMenu("checkbox_group_1", [
                                ("id_checkbox1", "Checkbox One"), ("id_checkbox2", "Checkbox Two")])
    menu.getItemById("id_radio").setSubMenu(radio_sub)
    menu.getItemById("id_checkbox").setSubMenu(checkbox_sub)
    # menu.getItemById("id_radio2").showCheckmark = False
    menu.itemClicked.connect(invoked)
    menu.menuDismissed.connect(dismissed)
    menu.showRectMenu(1400, 300)
    #menu.showDockMenu(1366, 768)

    sys.exit(app.exec_())
