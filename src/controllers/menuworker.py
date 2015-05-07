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



class MenuI18nWorker(QObject):

    __metaclass__ = ModelMetaclass

    __Fields__ = (
        ('addMusic', 'QString', u'添加歌曲'),
        ('easyMode', 'QString', u'简洁模式'),
        ('file', 'QString', u'文件'),
        ('folder', 'QString', u'文件夹'),
        ('miniMode', 'QString', u'迷你模式'),
        ('checkUpdate', 'QString', u'查看新特性'),
        ('setting', 'QString', u'设置'),
        ('exit', 'QString', u'退出'),
        ('play', 'QString', u'播放'),
        ('addToSinglePlaylist', 'QString', u'添加到歌单'),
        ('addToMutiPlaylist', 'QString', u'添加到多个歌单'),
        ('newPlaylist', 'QString', u'新建歌单'),
        ('removeFromDatabase', 'QString', u'从歌库中移除'),
        ('removeFromDriver', 'QString', u'从硬盘中移除'),
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
    )

    __contextName__ = "MenuI18nWorker"

    @registerContext
    def initialize(self, *agrs, **kwargs):
        pass

menuI18nWorker = MenuI18nWorker()


SettingMenuItems = [
    ('AddMusic', menuI18nWorker.addMusic, (), [("File", menuI18nWorker.file), ("Folder", menuI18nWorker.folder)]),
    None,
    ('EasyMode', menuI18nWorker.easyMode),
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



class DMenu(Menu):
    """docstring for DMenu"""
    def __init__(self, items):
        super(DMenu, self).__init__(items)

    def show(self):
        self.showRectMenu(QCursor.pos().x(), QCursor.pos().y())


class MenuWorker(QObject):

    __contextName__ = 'MenuWorker'

    settingMenuShow = pyqtSignal()
    artistMenuShow = pyqtSignal('QString')
    albumMenuShow = pyqtSignal('QString')
    songMenuShow = pyqtSignal('QString', 'QString')
    folderMenuShow = pyqtSignal('QString')

    #setting Menu
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
    playSong = pyqtSignal('QString')
    orderByKey = pyqtSignal('QString', 'QString')
    openSongFolder = pyqtSignal('QString')
    removeFromDatabaseByUrl = pyqtSignal('QString')
    removeFromDriveByUrl = pyqtSignal('QString')

    #Folder Menu
    playFolder = pyqtSignal('QString')
    removeFromDatabaseByFolderName = pyqtSignal('QString')
    removeFromDriverByFolderName = pyqtSignal('QString')

    @registerContext
    def __init__(self):
        super(MenuWorker, self).__init__()
        self._artist = ''
        self._album = ''
        self._url = ''
        self._folder = ''
        self._modelType = ''
        self.createSettingMenu()
        self.createArtistMenu()
        self.createAlbumMenu()
        self.createSongMenu()
        self.createFolderMenu()

    def createSettingMenu(self):
        self.settingMenu = DMenu(SettingMenuItems)
        self.settingMenu.itemClicked.connect(self.settingMenuConnection)
        self.settingMenuShow.connect(self.settingMenu.show)

    def createArtistMenu(self):
        self.artistMenu = DMenu(ArtistMenuItems)
        self.artistMenu.itemClicked.connect(self.artistMenuConnection)
        self.artistMenuShow.connect(self.showArtistMenu)

    def showArtistMenu(self, artist):
        self._artist = artist
        self.artistMenu.show()

    def createAlbumMenu(self):
        self.albumMenu = DMenu(AlbumMenuItems)
        self.albumMenu.itemClicked.connect(self.albumMenuConnection)
        self.albumMenuShow.connect(self.showAlbumMenu)

    def showAlbumMenu(self, album):
        self._album = album
        self.albumMenu.show()

    def createSongMenu(self):
        self.songMenu = DMenu(SongMenuItems)
        self.songMenu.itemClicked.connect(self.songMenuConnection)
        self.songMenuShow.connect(self.showSongMenu)

    def showSongMenu(self, modelType, url):
        self._modelType = modelType
        self._url = url
        self.songMenu.itemClicked.emit('Order_group:radio:OrderBySongName', True)
        self.songMenu.show()

    def createFolderMenu(self):
        self.folderMenu = DMenu(FolderMenuItems)
        self.folderMenu.itemClicked.connect(self.folderMenuConnection)
        self.folderMenuShow.connect(self.showFolderMenu)

    def showFolderMenu(self, folder):
        self._folder = folder
        self.folderMenu.show()

    def settingMenuConnection(self, menuId, checked):
        if menuId == "MiniMode":
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

    def albumMenuConnection(self, menuId, checked):
        if menuId == "Play":
            self.playAlbum.emit(self._album)
        elif menuId == "RemoveFromDatabase":
            self.removeFromDatabaseByAlbumName.emit(self._album)
        elif menuId == "RemoveFromDriver":
            self.removeFromDriverByAlbumName.emit(self._album)

    def songMenuConnection(self, menuId, checked):
        if menuId == 'Play':
            self.playSong.emit(self._url)
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
