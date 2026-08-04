// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import "../allItems"
import "../musicsublist"

Rectangle {
    property int switchType: globalVariant.globalSwitchButtonStatus;
    property ListModel albumModels: AlbumModel{}
    property int animtorTime: 200
    //采用名字作为索引，确保qml能够识别，
    // property var artistData: artistModels.get(0)qml可能无法识别
    property string albumName: albumModels.count > 0 ? albumModels.get(0).name : ""
    property point currentItemPos: [0, 0]
    signal itemDoubleClicked(var albumData)

    id: contenWindow
    color: "transparent"
    objectName: "album"

    Component {
        id: albumSublistView
        AlbumSublistView {
            scalePoint: currentItemPos
            albumData: {
                for(var i = 0; i < albumModels.count; i++){
                    if(albumName === albumModels.get(i).name){
                        return albumModels.get(i);
                    }
                }
            }
        }
    }

    StackView {
        id: stackView
        width: contenWindow.width; height: contenWindow.height
        anchors.left: contenWindow.left;
        clip: true
        initialItem: Rectangle {
            id: albumView
            color: "transparent"
            ToolButtonItem{
                id: toolButtonItem
                width: contenWindow.width - 40; height: 68;
                isPlayAll: contenWindow.albumModels.count > 0 ? true : false
                title: qsTr("Albums")
                musicinfo: qsTr("%1 albums - %2 songs").arg(contenWindow.albumModels.count).arg(contenWindow.albumModels.musicCount)
                gridAndlistViewModel: (contenWindow.albumModels.count === 0) ? false : true
                isDefault: (contenWindow.albumModels.count === 0) ? false : true
                pageHash: "album"
            }
            AlbumDefaultPage {
                id: defaultPage
                // Keep empty-state controls above the full-page DropArea below.
                z: 1
                width: toolButtonItem.width; height: contenWindow.height - toolButtonItem.height
                anchors.left: toolButtonItem.left
                anchors.top: toolButtonItem.bottom;
                visible: (contenWindow.albumModels.count === 0) ? true : false
            }

            AlbumListView {
                id: listview
                width: parent.width
                height: stackView.height - toolButtonItem.height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: toolButtonItem.bottom;
                visible: false;
                albumModels: contenWindow.albumModels
                onItemDoubleClicked:{
                    contenWindow.itemDoubleClicked(albumData);
                }
            }

            AlbumGridView {
                id: gridview
                width: parent.width; height: parent.height - toolButtonItem.height
                anchors.left: parent.left; /*anchors.leftMargin: 10*/
                anchors.top: toolButtonItem.bottom;
                visible: (contenWindow.albumModels.count === 0) ? false : true;
                albumModel: contenWindow.albumModels
                onItemDoubleClicked: function(albumData){
                    currentItemPos.x = gridview.view.currentItem.x + gridview.view.currentItem.width / 2
                    currentItemPos.y = gridview.view.currentItem.y + gridview.view.currentItem.height / 2 - 20
                    contenWindow.itemDoubleClicked(albumData);
                }
            }

            DropArea {
                anchors.fill: parent
                onDropped: {
                    var list = []
                    for (var i = 0; i < drop.urls.length; i++)
                        list.push(drop.urls[i])
                    Presenter.importMetas(list, globalVariant.curListPage)
                }
            }

            Connections {
                target: toolButtonItem
                function onViewChanged(type) {
                    if (type === 0) {
                        gridview.visible = true;
                        toggleGridViewAnimation.start()
                    } else {
                        listview.visible = true;
                        toggleListViewAnimation.start()
                    }
                }
            }
        }
        popEnter: Transition {
            NumberAnimation { property: "scale"; from: 0.8; to: 1; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 300; easing.type: Easing.InOutQuad }
        }

        popExit: Transition {
            NumberAnimation { property: "scale"; from: 1; to: 0.8; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.OutExpo }
        }

        pushEnter: Transition {
            NumberAnimation { property: "scale"; from: 1.2; to: 1; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 300; easing.type: Easing.InOutQuad }
        }

        pushExit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.OutExpo }
        }
    }
    onItemDoubleClicked: function(albumData) {
        contenWindow.albumName = albumData.name
        globalVariant.globalSwitchButtonStatus = 2; //使能上一页按钮，失能下一页按钮
    }
    onSwitchTypeChanged: {
        if(switchType === 1){
            stackView.pop(stackView.initialItem); //回到上一级页面
            globalVariant.globalSwitchButtonStatus = 1; //使能下一页按钮，失能上一页按钮
        }else if(switchType === 2){
            var item = stackView.find(function(item, index) { return item.objectName === "albumSublist"})
            if(item !== null){
                stackView.pop(item);
                return;
            }
            stackView.push(albumSublistView); //切换到下一级页面
        }
    }

    function returnUpperlevelView(){
        globalVariant.globalSwitchButtonStatus = 0; //初始化切换按钮状态
        stackView.pop(stackView.initialItem); //回到上一级页面
    }

    Connections {
        target: globalVariant
        function onReturnUpperlevelView() {
            contenWindow.returnUpperlevelView();
        }
    }
    Connections {
        target: albumModels
        function onMetaCodecChanged(name) {
            contenWindow.albumName = name;
        }
    }

    Component.onCompleted: {
        var sortType = Presenter.playlistSortType("album")
        if (sortType === 10)
            toolButtonItem.sortType = 0
        else if (sortType === 13)
            toolButtonItem.sortType = 1

        restoreTimer.start()
    }

    Timer {
        id: restoreTimer
        interval: 50
        onTriggered: {
            var mode = globalVariant.albumDisplayMode
            // 静默恢复按钮选中态
            toolButtonItem.restoreViewMode(mode)

            toggleGridViewAnimation.stop()
            toggleListViewAnimation.stop()
            listFirstScale.stop()
            // 恢复期间不播放动画：直接把两个 view 设为终态
            if (mode === 1) {
                // 列表模式：隐藏 grid、显示 list，scale/opacity 均置为 1
                gridview.visible = false
                gridview.scale = 1
                gridview.opacity = 1
                listview.visible = true
                listview.scale = 1
                listview.opacity = 1
            } else {
                // 平铺模式：隐藏 list、显示 grid
                listview.visible = false
                listview.scale = 1
                listview.opacity = 1
                gridview.visible = true
                gridview.scale = 1
                gridview.opacity = 1
            }
        }
    }

    SequentialAnimation {
        id: toggleGridViewAnimation

        ParallelAnimation {
            ScaleAnimator {
                target: listview
                from: 1
                to: 0
                duration: animtorTime
                easing.type: Easing.InOutQuad
            }
            OpacityAnimator {
                target: listview
                from: 1
                to: 0
                duration: animtorTime
                easing.type: Easing.InQuint
            }
        }

        ParallelAnimation {
            ScaleAnimator {
                target: gridview
                from: 0
                to: 1
                duration: animtorTime
                easing.type: Easing.InOutQuad
            }
            OpacityAnimator {
                target: gridview
                from: 0
                to: 1
                duration: animtorTime
                easing.type: Easing.InQuint
            }
        }
    }

    SequentialAnimation {
        id: toggleListViewAnimation

        ParallelAnimation {
            id: gridHideAnimation
            ScaleAnimator {
                target: gridview
                from: 1
                to: 0
                duration: animtorTime
                easing.type: Easing.InOutQuad
            }
            OpacityAnimator {
                target: gridview
                from: 1
                to: 0
                duration: animtorTime
                easing.type: Easing.InQuint
            }
        }

        ParallelAnimation {
            ScaleAnimator {
                target: listview
                from: 0
                to: 1
                duration: animtorTime
                easing.type: Easing.InOutQuad
            }
            OpacityAnimator {
                target: listview
                from: 0
                to: 1
                duration: animtorTime
                easing.type: Easing.InQuint
            }
        }

        Component.onCompleted: {
            listFirstScale.start()
        }
    }
    ScaleAnimator {
        id: listFirstScale
        target: listview
        from: 1
        to: 0
        duration: animtorTime
        easing.type: Easing.InOutQuad
    }
    Connections {
        target: toggleGridViewAnimation
        function onFinished() {
            listview.visible = false
        }
    }
    Connections {
        target: toggleListViewAnimation
        function onFinished() {
            gridview.visible = false
        }
    }
}
