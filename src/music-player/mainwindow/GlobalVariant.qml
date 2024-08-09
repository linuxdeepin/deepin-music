// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.0
import "../dialogs"
import "../musicbaseandsonglist"
import audio.global 1.0
import org.deepin.dtk 1.0

Item {
    // 标题栏左右切换按钮状态,控制一、二级页面切换和按钮使能
    // 0: 为初始化, 1: 使能下一页按钮，失能上一页按钮，并切换页面到一级页面
    // 2：使能上一页按钮，失能下一页按钮，并切换到二级页面
    property int globalSwitchButtonStatus: 0;
    property var currentMediaMeta  //当前歌曲
    property string curPlayingHash: Presenter.valueFromSettings("base.play.last_meta")
    property int curPlayingStatus: DmGlobal.Idle
    property string curPlayingAlbum: ""
    property string curPlayingArtist: ""
    property string curListPage: "all"
    property string playingIconName: "music_play1" //播放中动效图标
    property bool playlistExist: false
    property int playingCount: 0
    property ListModel globalCustomPlaylistModel: CustomPlaylistModel {} //全局的自定义歌单model
    property var currentSelectMediaMeta: null //当前选择歌曲
    property int curPlayMode: Presenter.getPlaybackMode()
    property string appIconName: "deepin-music"
    property real devicePixelRatio: 1

    signal switchToPreviousPlaylist(int previousIndex);  //删除歌单后，自动切换到上一个歌单列表
    signal renamePlaylist(string hash); //重命名
    signal renameNewItem(); //新建歌单时重命名
    signal returnUpperlevelView()  //返回上一级页面信号
    signal updateCurrentPlaylistTitleName(string name, string curHash);  //重命名刷新信号
    signal clearSelectGroup();    //删除后，清除选中数据的index
    signal sendFloatingMessageBox(string pageName, int msgType);  // 消息框提示信号
    signal clickPlayAllBtn();    // 播放全部按钮点击，开始列表按钮动画

    Loader { id: globalFileDlgLoader }
    property Loader closeConfirmDlgLoader: Loader {}
    Loader { id: cdRemovedDlgLoader }
    Loader { id: musicInfoDlgLoader }

    function onMetaChanged(){
        currentMediaMeta = Presenter.getActivateMeta();
        curPlayingHash = currentMediaMeta.hash
        curPlayingAlbum = currentMediaMeta.album
        curPlayingArtist = currentMediaMeta.artist
    }
    function onPlaybackStatusChanged(status){ curPlayingStatus = status;}
    function onUpdatePlayingIcon(iconPath){
        playingIconName = iconPath;
    }
    function globalFileDlgOpen() {
        if (globalFileDlgLoader.status === Loader.Null)
            globalFileDlgLoader.setSource("../dialogs/FileDialog.qml")
        if (globalFileDlgLoader.status === Loader.Ready )
            globalFileDlgLoader.item.open()
    }
    function cdRemovedDlgShow() {
        if (cdRemovedDlgLoader.status === Loader.Null)
            cdRemovedDlgLoader.setSource("../dialogs/CDRemovedDialog.qml")
        if (cdRemovedDlgLoader.status === Loader.Ready )
            cdRemovedDlgLoader.item.show()
    }
    function musicInfoDlgShow() {
        if (musicInfoDlgLoader.status === Loader.Null)
            musicInfoDlgLoader.setSource("../dialogs/MusicInfoDialog.qml")
        if (musicInfoDlgLoader.status === Loader.Ready && currentSelectMediaMeta !== null) {
            musicInfoDlgLoader.item.musicData = currentSelectMediaMeta
            musicInfoDlgLoader.item.show()
        }
    }

    Component.onCompleted: {
        currentMediaMeta = Presenter.getActivateMeta();
        Presenter.metaChanged.connect(onMetaChanged);
        Presenter.playbackStatusChanged.connect(onPlaybackStatusChanged);
        Presenter.updatePlayingIcon.connect(onUpdatePlayingIcon);
    }
}
