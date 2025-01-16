// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import Qt.labs.platform 1.1
import audio.global 1.0
import "../playlist"
import "../dialogs"
import "../allItems"


ApplicationWindow {
    property Item globalVariant: GlobalVariant{} //全局变量，对此之后的对象都可见
    property bool isLyricShow : false
    property bool isPlaylistShow : false
    property int windowMiniWidth: 1070
    property int windowMiniHeight: 680

    id: rootWindow
    visible: true
    minimumWidth: windowMiniWidth
    minimumHeight: windowMiniHeight
    width: windowMiniWidth
    height: windowMiniHeight
    DWindow.enabled: true
    DWindow.alphaBufferSize: 8
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    header: WindowTitlebar { id: musicTitle }
    background: Rectangle {
        anchors.fill: parent
        color: "transparent"

        Row {
            anchors.fill: parent
            BehindWindowBlur {
                id: leftBgArea
                width: 220
                height: parent.height
                anchors.top: parent.top
                blendColor: DTK.themeType === ApplicationHelper.LightType ? "#bbf7f7f7"
                                                                          : "#dd252525"
                Rectangle {
                    width: 1
                    height: parent.height
                    anchors.right: parent.right
                    color: DTK.themeType === ApplicationHelper.LightType ? "#eee7e7e7"
                                                                         : "#ee252525"
                }
            }
            Rectangle {
                id: rightBgArea
                width: parent.width - leftBgArea.width
                height: 50
                anchors.top: parent.top
                color: Qt.rgba(0, 0, 0, 0.01)
                BoxShadow {
                    anchors.fill: rightBgArea
                    shadowOffsetX: 0
                    shadowOffsetY: 4
                    shadowColor: Qt.rgba(0, 0, 0, 0.05)
                    shadowBlur: 10
                    cornerRadius: rightBgArea.radius
                    spread: 0
                    hollow: true
                }
            }
        }
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

    Loader { id: playlistLoader }

    SystemTrayIcon{
        id: systemTray
        visible: true
        // TODO: temporar setting, wait dtk fix IconEngine. icon.name is fallback
        icon.name: "deepin-music"
        icon.source: "qrc:/dsg/img/deepin-music.svg"
        tooltip: qsTr("Music")

        onActivated: {
            //console.log("..........................", rootWindow.visibility, rootWindow.visible)
            if (rootWindow.visibility === 2) {
                rootWindow.showMinimized()
            } else if (rootWindow.visibility === 3 || rootWindow.visibility === 0) {
                rootWindow.show()
                rootWindow.raise()
                rootWindow.requestActivate()
            }
        }

        menu: Menu {
            MenuItem {
                text: qsTr("Play/Pause")
                onTriggered: Presenter.playPause()
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

    Loader { id: messageBoxLoader }
    Loader { id: importFailedDlgLoader }

    onActiveChanged: {
        //窗口显示完成后加载播放列表
        //console.log("onActiveChanged................", active, "  visibility:", visibility)
        if (active && playlistLoader.status === Loader.Null) {
            playlistLoader.setSource("qrc:/playlist/CurrentPlayList.qml")
            playlistLoader.item.width = 320
            playlistLoader.item.height = rootWindow.height - 90 - 50
            playlistLoader.item.y = height - playlistLoader.item.height - 80 - 50
            playlistLoader.item.playlistHided.connect(function(){
                toolbox.updatePlaylistBtnStatus(false)
            })
            playlistLoader.item.playlistEmpty.connect(function(){
                if (isLyricShow)
                    lrcWindowLoader.item.lyricWindowUp()
            })
            toolbox.updatePlayControlBtnStatus()
        }
    }
    onWidthChanged: {
        playlistLoader.item.x = width - playlistLoader.item.width - 10
    }
    onHeightChanged: {
//        playlistLoader.item.y = height - playlistLoader.item.height - 80 - 50
        playlistLoader.item.height = rootWindow.height - 90 - 50
    }

    onClosing: {
        var closeAction = Presenter.valueFromSettings("base.close.close_action")
        //console.log("closeAction: " + closeAction)

        if (globalVariant.closeConfirmDlgLoader.status === Loader.Null)
            globalVariant.closeConfirmDlgLoader.setSource("../dialogs/CloseConfirmDialog.qml")

        if (globalVariant.closeConfirmDlgLoader.item.isMinimize) {
            //最小化
            globalVariant.closeConfirmDlgLoader.item.isMinimize = false
        } else if (closeAction == 1) {
            //退出
            Presenter.forceExit();
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
        onLyricToggleClicked: {
            if (lrcWindowLoader.status === Loader.Null) {
                lrcWindowLoader.setSource("LyricWindow.qml")
                lrcWindowLoader.item.y = -50
            }
             if (lrcWindowLoader.status === Loader.Ready) {
                 lrcWindowLoader.item.lyricWindowUp()
             }
        }
        onPlaylistBtnClicked: {
            if (playlistLoader.status === Loader.Ready) {
                playlistLoader.item.playlistRaise()
                isPlaylistShow = true
            }
        }
    }
    Connections {
        id: titleBarConnect
        target: musicTitle
        onLrcHideBtnClicked: {
            if (lrcWindowLoader.status === Loader.Null) {
                lrcWindowLoader.setSource("LyricWindow.qml")
                lrcWindowLoader.item.y = -50
            }
             if (lrcWindowLoader.status === Loader.Ready) {
                 lrcWindowLoader.item.lyricWindowUp()
             }
        }
        onSearchItemTriggered: {
            //console.log("maindow:onSearchItemTriggered:" + text)
            //searchResultDlgItemTriggered(text)
            contentWindow.onSearchResultItemChanged(text, type)
        }
    }
    Connections {
        target: globalVariant.closeConfirmDlgLoader.item
        onMinimizeToSystemTray: {
            //console.log("onMinimizeToSystemTray......................")
            close()
        }
    }
    Connections {
        target: globalVariant
        onSendFloatingMessageBox: {
            if (messageBoxLoader.status === Loader.Null) {
                messageBoxLoader.setSource("../allItems/FloatingMessageBox.qml")
            }

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
            default:
                break;
            }
            messageBoxLoader.item.show();
        }

        onClickPlayAllBtn: {
            toolbox.startListBtnAnim()
        }
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
        if (sucessCount <= 0 && failCount > 0) {
            if (importFailedDlgLoader.status === Loader.Null)
                importFailedDlgLoader.setSource("../dialogs/ImportFailedDialog.qml")
            if (importFailedDlgLoader.status === Loader.Ready)
                importFailedDlgLoader.item.show()
        } else if (sucessCount <= 0 && existCount > 0) {
            globalVariant.sendFloatingMessageBox("", 0)
        } else {
            if (globalVariant.curListPage === "all") {
                globalVariant.sendFloatingMessageBox(qsTr("All Songs"), 1)
            }
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

    Component.onCompleted: {
        Presenter.addOneMeta.connect(onAddOneMeta)
        Presenter.importFinished.connect(onImportFinished)
        Presenter.quitRequested.connect(onQuitRequested)
        Presenter.raiseRequested.connect(onRaiseRequested)
        globalVariant.devicePixelRatio = Screen.devicePixelRatio
    }
}
