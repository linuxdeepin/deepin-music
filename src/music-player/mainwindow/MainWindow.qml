// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import Qt.labs.platform 1.1
import audio.global 1.0
import "../playlist"
import "../dialogs"
import "../allItems"


ApplicationWindow {
    property Item globalVariant: GlobalVariant{} //全局变量，对此之后的对象都可见
    property bool isLyricShow : false
    property bool isPlaylistShow : false
    property bool playlistShowRequested: false
    property bool deferredUiLoaded: false
    property int windowMiniWidth: 1070
    property int windowMiniHeight: 680

    id: rootWindow
    visible: true
    minimumWidth: windowMiniWidth
    minimumHeight: windowMiniHeight
    width: windowMiniWidth
    height: windowMiniHeight
    title: globalVariant.currentMediaMeta && globalVariant.currentMediaMeta.localPath
           ? globalVariant.currentMediaMeta.localPath
           : qsTr("Music")
    DWindow.enabled: true
    DWindow.alphaBufferSize: 8
    DWindow.enableBlurWindow: true
    color: "transparent"
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | (Qt.platform.os === "windows" ? Qt.FramelessWindowHint : Qt.WindowTitleHint)
    header: WindowTitlebar { id: musicTitle }
    background: Rectangle {
        anchors.fill: parent
        color: "transparent"

        Row {
            anchors.fill: parent
            // 左侧导航栏毛玻璃效果
            D.StyledBehindWindowBlur {
                id: leftBgArea
                width: parent.width > 600 ? 220 : 0
                visible: parent.width > 600
                height: parent.height
                anchors.top: parent.top
                control: rootWindow
                blendColor: {
                    if (valid) {
                        return DS.Style.control.selectColor(control ? control.palette.window : undefined,
                            Qt.rgba(0.97, 0.97, 0.97, 0.73),
                            Qt.rgba(0.15, 0.15, 0.15, 0.87))
                    }
                    return DS.Style.control.selectColor(undefined,
                        DS.Style.behindWindowBlur.lightNoBlurColor,
                        DS.Style.behindWindowBlur.darkNoBlurColor)
                }
                Rectangle {
                    width: 1 / Screen.devicePixelRatio
                    height: parent.height
                    anchors.right: parent.right
                    color: DTK.themeType === ApplicationHelper.LightType
                           ? Qt.rgba(0, 0, 0, 0.08)
                           : Qt.rgba(0, 0, 0, 0.50)
                }
            }
            // 右侧区域纯色背景
            Rectangle {
                id: rightBgArea
                width: parent.width - leftBgArea.width
                height: parent.height
                anchors.top: parent.top
                color: DTK.themeType === ApplicationHelper.LightType ? "#f7f7f7" : "#252525"
            }
        }
    }

    function loadDeferredUi() {
        if (deferredUiLoaded)
            return

        deferredUiLoaded = true
        toolbox.restorePlaybackStatus()
    }

    Shortcuts {
        id: shortcuts
        enabled: rootWindow.active
    }

    MusicContentWindow {
        id: contentWindow
        visible: !isLyricShow
        y: -50
        width: parent.width
        height: parent.height/* - musicTitle.height - 20*/
        anchors.left: musicTitle.left/*; anchors.top: musicTitle.bottom*/
    }


//    Rectangle {
//        property string icon_name: "toolbar_playlist_checked"
//        property size icon_size: Qt.size(50, 50)
//        property int rectWidth: 50
//        property int rectHeight: 50
//        property var iconMode: [ColorSelector.NormalState, ColorSelector.HoveredState,
//            ColorSelector.PressedState, ColorSelector.DisabledState/*, ColorSelector.InactiveState*/]

//        id: control
//        width: 500
//        height: 300
//        anchors.centerIn: parent
////        visible: false

//        Row {
//            anchors.centerIn: parent
//            spacing: 10
//            Repeater {
//                model: control.iconMode.length
//                Rectangle {
//                    width: control.rectWidth
//                    height: control.rectHeight
//                    //border.color: "#0000ff"
//                    //color: "#aaaaaa"

//                    DciIcon {
//                        sourceSize: control.icon_size
//                        name: control.icon_name
//                        mode: control.iconMode[index]
//                        palette: DTK.makeIconPalette(parent.palette)
//                        theme: ApplicationHelper.LightType
//                    }
//                }
//            }
//        }
//    }

    /*Rectangle {
        width: 200
        height: 200
        anchors.centerIn: parent
        color: "transparent"
//        Image {
//            id: back
//            anchors.fill: parent
//            source: "qrc:/dsg/img/test1.jpg"
//        }
        InWindowBlur {
            id: blur
            anchors.fill: parent
            radius: 32
            offscreen: true
        }
        ItemViewport {
            id: roundBlur
            width: 100; height: 100
            anchors.centerIn: blur
            fixed: false
            sourceItem: blur
            radius: width / 2
            hideSource: false
        }
        Rectangle {
            radius: roundBlur.radius
            anchors.fill: roundBlur
            color: Qt.rgba(0, 0, 0, 0.1)
        }
    }*/

    Toolbar {
        id: toolbox
        width: parent.width
        z: 10
    }

    Loader {
        id: lrcWindowLoader

        onLoaded: {
            lrcWindowLoader.item.animationFinished.connect(onAnimationFinished)
            lrcWindowLoader.item.animationStart.connect(onAnimationStart)
        }

        function onAnimationFinished(isShow) {
            isLyricShow = !isShow
        }
        function onAnimationStart(show) {
            musicTitle.toggleLyrics(show)
        }
    }

    Loader {
        id: playlistLoader
        onLoaded: {
            rootWindow.configurePlaylist()
            if (rootWindow.playlistShowRequested) {
                item.playlistRaise()
                rootWindow.isPlaylistShow = true
            }
        }
        onStatusChanged: {
            if (status === Loader.Error) {
                rootWindow.playlistShowRequested = false
                rootWindow.isPlaylistShow = false
                toolbox.updatePlaylistBtnStatus(false)
                console.warn("Failed to load play queue")
                source = ""
            }
        }
    }

    Loader {
        id: systemTrayLoader
        active: rootWindow.deferredUiLoaded
        sourceComponent: Component {
            SystemTrayIcon{
            id: systemTray
            visible: true
            // TODO: temporar setting, wait dtk fix IconEngine. icon.name is fallback
            icon.name: "deepin-music"
            icon.source: "qrc:/dsg/img/deepin-music.svg"
            tooltip: qsTr("Music")

            onActivated: {
                if (rootWindow.visibility === Window.Minimized || !rootWindow.visible) {
                    rootWindow.show()
                    rootWindow.raise()
                    rootWindow.requestActivate()
                } else {
                    rootWindow.showMinimized()
                }
            }

            menu: Menu {
                MenuItem {
                    text: qsTr("Play/Pause")
                    onTriggered: Presenter.playPause()
                    enabled: globalVariant.playingCount > 0
                }
                MenuItem {
                    text: qsTr("Previous")
                    onTriggered: Presenter.playPre()
                    enabled: {
                        if ((globalVariant.playlistExist && globalVariant.playingCount <= 1) ||
                            (!Presenter.preMetaFromPlay(globalVariant.currentMediaMeta.hash) && globalVariant.curPlayMode === DmGlobal.RepeatNull))
                            return false
                        else
                            return true
                    }
                }
                MenuItem {
                    text: qsTr("Next")
                    onTriggered: Presenter.playNext()
                    enabled: {
                        if ((globalVariant.playlistExist && globalVariant.playingCount <= 1) ||
                            (!Presenter.nextMetaFromPlay(globalVariant.currentMediaMeta.hash) && globalVariant.curPlayMode === DmGlobal.RepeatNull))
                            return false
                        else
                            return true
                    }
                }
                MenuItem {
                    text: qsTr("Exit")
                    onTriggered: Presenter.forceExit();
                }
            }
        }

        }
    }

    Loader { id: importFailedDlgLoader }

    function configurePlaylist() {
        if (!playlistLoader.item)
            return

        playlistLoader.item.width = 320
        playlistLoader.item.height = rootWindow.height - 90 - 50
        playlistLoader.item.y = height - playlistLoader.item.height - 80 - 50
        playlistLoader.item.playlistHided.connect(function(){
            playlistShowRequested = false
            isPlaylistShow = false
            toolbox.updatePlaylistBtnStatus(false)
        })
        playlistLoader.item.playlistEmpty.connect(function(){
            if (isLyricShow)
                lrcWindowLoader.item.lyricWindowUp()
        })
        toolbox.updatePlayControlBtnStatus()
    }

    function showPlaylist() {
        playlistShowRequested = true
        if (playlistLoader.status === Loader.Null) {
            playlistLoader.setSource("qrc:/playlist/CurrentPlayList.qml")
            return
        }
        if (playlistLoader.status === Loader.Ready) {
            playlistLoader.item.playlistRaise()
            isPlaylistShow = true
        }
    }
    onWidthChanged: {
        if (playlistLoader.item)
            playlistLoader.item.x = width - playlistLoader.item.width - 10
    }
    onHeightChanged: {
//        playlistLoader.item.y = height - playlistLoader.item.height - 80 - 50
        if (playlistLoader.item)
            playlistLoader.item.height = rootWindow.height - 90 - 50
    }

    onClosing: {
        var closeAction = Number(Presenter.valueFromSettings("base.close.close_action"))
        if (isNaN(closeAction))
            closeAction = 2
        //console.log("closeAction: " + closeAction)

        // Do not instantiate the confirmation dialog when the configured action is direct exit.
        if (closeAction === 1) {
            Presenter.forceExit()
            return
        }

        if (globalVariant.closeConfirmDlgLoader.status === Loader.Null)
            globalVariant.closeConfirmDlgLoader.setSource("../dialogs/CloseConfirmDialog.qml")

        if (globalVariant.closeConfirmDlgLoader.status !== Loader.Ready) {
            close.accepted = false
            console.warn("Failed to load close confirmation dialog")
            return
        }

        if (globalVariant.closeConfirmDlgLoader.item.isMinimize) {
            //最小化
            globalVariant.closeConfirmDlgLoader.item.isMinimize = false
        } else {
            //询问
            globalVariant.closeConfirmDlgLoader.item.isClose = Presenter.valueFromSettings("base.close.is_close")
            if (closeAction !== 2)
                globalVariant.closeConfirmDlgLoader.item.closeAction = closeAction
            if (!globalVariant.closeConfirmDlgLoader.item.isClose) {
                close.accepted = false
                globalVariant.closeConfirmDlgLoader.item.show()
            }
        }
    }

    Connections {
        id: toolboxConnect
        target: toolbox
        function onLyricToggleClicked() {
            if (lrcWindowLoader.status === Loader.Null) {
                lrcWindowLoader.setSource("LyricWindow.qml")
                lrcWindowLoader.item.y = -50
            }
             if (lrcWindowLoader.status === Loader.Ready) {
                 lrcWindowLoader.item.lyricWindowUp()
             }
        }
        function onPlaylistBtnClicked() {
            rootWindow.showPlaylist()
        }
    }
    Connections {
        id: titleBarConnect
        target: musicTitle
        function onLrcHideBtnClicked() {
            if (lrcWindowLoader.status === Loader.Null) {
                lrcWindowLoader.setSource("LyricWindow.qml")
                lrcWindowLoader.item.y = -50
            }
             if (lrcWindowLoader.status === Loader.Ready) {
                 lrcWindowLoader.item.lyricWindowUp()
             }
        }
        function onSearchItemTriggered(text, type) {
            //console.log("maindow:onSearchItemTriggered:" + text)
            //searchResultDlgItemTriggered(text)
            contentWindow.onSearchResultItemChanged(text, type)
        }
    }
    Connections {
        target: globalVariant.closeConfirmDlgLoader.item
        function onMinimizeToSystemTray() {
            //console.log("onMinimizeToSystemTray......................")
            close()
        }
    }
    Connections {
        target: globalVariant
        function onSendFloatingMessageBox(pageName, msgType) {
            if (messageBoxLoader.status === Loader.Null) {
                messageBoxLoader.setSource("../allItems/FloatingMessageBox.qml")
            }

            if (messageBoxLoader.status === Loader.Ready) {
                showFloatingMessage(pageName, msgType)
            } else {
                messageBoxLoader.pendingPageName = pageName
                messageBoxLoader.pendingMsgType = msgType
            }
        }

        function onClickPlayAllBtn() {
            toolbox.startListBtnAnim()
        }
    }

    Loader {
        id: messageBoxLoader
        property string pendingPageName: ""
        property int pendingMsgType: -1

        onLoaded: {
            if (pendingMsgType >= 0) {
                showFloatingMessage(pendingPageName, pendingMsgType)
                pendingMsgType = -1
            }
        }
    }

    function showFloatingMessage(pageName, msgType) {
        messageBoxLoader.item.type = msgType;
        switch (messageBoxLoader.item.type){
        case 0:
            messageBoxLoader.item.message = qsTr("Already added to the playlist");
            break;
        case 1:
            messageBoxLoader.item.message = qsTr("Added to \"%1\"").arg(pageName);
            break;
        case 2:
            messageBoxLoader.item.message = qsTr("Removed from \"My Favorites\"");
            break;
        case 3:
            messageBoxLoader.item.message = qsTr("A disc is connected");
            break;
        case 4:
            messageBoxLoader.item.message = qsTr("Sound Effects Saved");
            break;
        default:
            break;
        }
        messageBoxLoader.item.show();
    }

    function onAddOneMeta(playlistHashs, meta) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] === "fav") {
                globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 1);
            } else {
                for (var j = 0; j < globalVariant.globalCustomPlaylistModel.count; j++) {
                    if (playlistHashs[i] ===  globalVariant.globalCustomPlaylistModel.get(j).uuid) {
                        var value = globalVariant.globalCustomPlaylistModel.get(j).displayName;
                        globalVariant.sendFloatingMessageBox(value, 1);
                        break
                    }
                }
            }
        }
    }
    function onImportFinished(playlistHashs, failCount, sucessCount, existCount) {
        if (sucessCount <= 0) {
            // 当所有文件都已存在时显示提示
            if (existCount > 0 && sucessCount === 0 && failCount === 0) {
                globalVariant.sendFloatingMessageBox("", 0);
                return;
            }
            
            // 有导入失败的情况
            if (importFailedDlgLoader.status === Loader.Null) {
                importFailedDlgLoader.setSource("../dialogs/ImportFailedDialog.qml");
            }
            if (importFailedDlgLoader.status === Loader.Ready) {
                importFailedDlgLoader.item.show();
            }
        } else if (globalVariant.curListPage === "all") {
            globalVariant.sendFloatingMessageBox(qsTr("All Songs"), 1);
        }
    }
    function onQuitRequested() {
        Presenter.forceExit();
    }
    function onRaiseRequested() {
        //console.log("onRaiseRequested......................", rootWindow.visibility, rootWindow.visible)
        rootWindow.show()
        rootWindow.raise()
        rootWindow.requestActivate()
    }

    function onKeyFiltered(key, modifier) {
        if (key === Qt.Key_A && modifier === Qt.ControlModifier) {
            if (!isPlaylistShow && !isLyricShow) {
                contentWindow.selectAll();
            }
        }
    }

    Component.onCompleted: {
        Presenter.addOneMeta.connect(onAddOneMeta)
        Presenter.importFinished.connect(onImportFinished)
        Presenter.quitRequested.connect(onQuitRequested)
        Presenter.raiseRequested.connect(onRaiseRequested)
        globalVariant.devicePixelRatio = Screen.devicePixelRatio
        EventsFilter.keyFiltered.connect(onKeyFiltered)
        // Center window on screen
        if (Screen.width > 0 && Screen.height > 0) {
            x = (Screen.width - width) / 2
            y = (Screen.height - height) / 2
        }
    }
}
