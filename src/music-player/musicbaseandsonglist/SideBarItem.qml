// Copyright (C) 2022 UnionTech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../dialogs"

ColumnLayout {
    property string title
    property string type
    property ListModel sideModel
    property alias view: sideListView
    property bool fillHeight: false
    property Component action
    signal itemClicked(string key, string text)
    signal itemRightClicked(string key, string text)
    property ButtonGroup group: ButtonGroup {}

    id: control
    spacing: 10
    Layout.leftMargin: 10

    Rectangle {
        id: siderTitle
        width: 200
        height: 20
        color: "transparent"
        Label {
            id: viewLabel
            width: 42; height: 20
            anchors.left: parent.left
            anchors.leftMargin: 10
            color: Qt.rgba(0.6,0.6,0.6, 0.8)
            text: title
            horizontalAlignment: Qt.AlignLeft
        }
        Loader {
            anchors.right: parent.right
            anchors.rightMargin: 10
            sourceComponent: control.action
        }
    }

    ListView {
        id: sideListView
        width: 200

        Layout.alignment: Qt.AlignLeft
        Layout.fillHeight: fillHeight
        Layout.leftMargin: 10
        model: sideModel
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        delegate: SideBarItemDelegate{
            id: sidebarItem
            width: 180; height: 36
            backgroundVisible: true
            normalBackgroundVisible: false
            ButtonGroup.group: group
            type: control.type
            font: DTK.fontManager.t6
        }

        Keys.onPressed: {
            switch (event.key){
            case Qt.Key_F2:
                sideListView.currentItem.rename();
                break;
            case Qt.Key_Delete:
                if(sideModel.get(currentIndex).editable){
                    removeSong.listHash = sideModel.get(currentIndex).uuid;
                    removeSong.show();
                }
                break
            default:
                break;
            }
            event.accepted = true;
        }

        DropArea {
            property int lastIndex: 0
            property int toIndex: 0
            property bool dragForSort: false
            property bool dragForExpand: false
            property int hoverIndex: 0
            property bool isScroll: false
            property int lastDragY: 0

            id: dropArea
            anchors.fill: control.type === "playlists" ? parent : null

            onEntered: {
                console.log("onEntered.............", dragForSort)
                for(var j = 0; j < drag.keys.length; j++) {
                    if (drag.keys[j] === "uuid") {
                        dragForSort = true
                        break
                    } else if (drag.keys[j] === "music-list/hash-list") {
                        dragForExpand = true
                        break
                    }
                }
            }
            onPositionChanged: {
//                console.log("onPositionChanged.........drag.y:", drag.y, "   sideListView.height:", sideListView.height)
                updateHoverIndex()

                var object = mapToItem(musicBaseScrollView, drag.x, drag.y)

//                console.log("drag.x,drag.y:", drag.x, drag.y, "object.x,object.y:", object.x, object.y)
//                console.log("musicBaseScrollView.atYBeginning:", musicBaseScrollView.contentItem.atYBeginning, "  musicBaseScrollView.atYEnd:", musicBaseScrollView.contentItem.atYEnd)

//                if (drag.y !== lastDragY)
//                    lastDragY = drag.y

                if (object.y < 30 && !musicBaseScrollView.contentItem.atYBeginning) {
                    scrollUpTimer.start()
                } else {
                    scrollUpTimer.stop()
                }

                if (object.y > musicBaseScrollView.height - 30 && !musicBaseScrollView.contentItem.atYEnd) {
                    scrollDownTimer.start()
                } else {
                    scrollDownTimer.stop()
                }
            }
            onDropped: {
                if (dragForSort) {
                    scrollDownTimer.stop()
                    scrollUpTimer.stop()
                    if (drop.getDataAsString("index") > toIndex) {
                        Presenter.movePlaylist(sideModel.get(drop.getDataAsString("index")).uuid, sideModel.get(toIndex + 1).uuid)
                        sideModel.move(drop.getDataAsString("index"), toIndex + 1, 1)
                        sideModel.setProperty(toIndex, "dragFlag", false)
                    } else {
                        if(toIndex + 1 >= sideModel.count)
                            Presenter.movePlaylist(sideModel.get(drop.getDataAsString("index")).uuid, "")
                        else
                            Presenter.movePlaylist(sideModel.get(drop.getDataAsString("index")).uuid, sideModel.get(toIndex + 1).uuid)
                        sideModel.move(drop.getDataAsString("index"), toIndex, 1)
                        sideModel.setProperty(toIndex - 1, "dragFlag", false)
                    }
                    dragForSort = false
                } else if (dragForExpand) {
                    console.log("dropForExpand:", drop.getDataAsString("music-list/hash-list"))
                    var list = drop.getDataAsString("music-list/hash-list").split(",")
                    console.log("list:", list)
                    Presenter.addMetasToPlayList(list, sideModel.get(toIndex).uuid);
                    dragForExpand = false
                } else {
                    console.log("dropForImport:", drop.urls)
                    var urlList = []
                    for (var i = 0; i < drop.urls.length; i++)
                        urlList.push(drop.urls[i])

                    Presenter.importMetas(urlList, sideModel.get(toIndex).uuid)
                }
            }
            onExited: {
                sideModel.setProperty(toIndex, "dragFlag", false)
                scrollDownTimer.stop()
                scrollUpTimer.stop()
            }
            function updateHoverIndex() {
//                console.log("updateHoverIndex................", musicBaseScrollView.contentItem.movingHorizontally)
//                console.log("updateHoverIndex.........drag.y:", drag.y, "   ", musicBaseScrollView.contentItem.contentY)
//                var obj = mapToGlobal(drag.x, drag.y)
//                console.log("updateHoverIndex.........drag.y:", drag.y, "  obj.y:", obj.y, "   ", sideListView.originY)

                if (drag.y == lastDragY && isScroll) {
                    hoverIndex = sideListView.indexAt(drag.x, drag.y + (drag.y - lastDragY) + musicBaseScrollView.contentItem.contentY)

                } else {
                    hoverIndex = sideListView.indexAt(drag.x, drag.y)
                }

                if (lastDragY != drag.y)
                    lastDragY = drag.y


//                console.log("drag.y:", drag.y, "  contentY:", musicBaseScrollView.contentItem.contentY, "  lastDragY:", lastDragY,
//                            "  hoverIndex:", hoverIndex)

                if (dragForSort) {
                    if (sideModel.get(1).uuid === "cdarole" && hoverIndex <= 1)
                        hoverIndex = 2
                    if (drag.y < hoverIndex * 36 + 36 / 2)
                        hoverIndex--
                    if (hoverIndex < 0)
                        hoverIndex = 0

                    sideModel.setProperty(lastIndex, "dragFlag", false)
                    sideModel.setProperty(hoverIndex, "dragFlag", true)
                    lastIndex = hoverIndex
                    toIndex = hoverIndex
                } else {
                    toIndex = hoverIndex
                }
            }
            Connections {
                target: musicBaseScrollView.contentItem
                onContentYChanged: {
//                    console.log("Connections........onContentYChanged   ", musicBaseScrollView.contentItem.contentY)
                    if (control.type === "playlists")
                        dropArea.updateHoverIndex()
                }
            }
        }

        Timer {
            id: scrollDownTimer
            interval: 40
            repeat: true
            running: false

            onTriggered: {
                if(!musicBaseScrollView.contentItem.atYEnd)
                    musicBaseScrollView.contentItem.contentY += 10
            }
            onRunningChanged: {
                dropArea.isScroll = running
            }
        }
        Timer {
            id: scrollUpTimer
            interval: 40
            repeat: true
            running: false

            onTriggered: {
                if(!musicBaseScrollView.contentItem.atYBeginning)
                    musicBaseScrollView.contentItem.contentY -= 10
            }
            onRunningChanged: {
                dropArea.isScroll = running
            }
        }

        function onRenameNewItem(){
            sideListView.currentIndex = sideListView.model.count - 1;
            sideListView.currentItem.enableRename();
        }
        function onRenamePlaylist(hash){
            for(var i = sideModel.count - 1; i >= 0; i--){
                if(hash === sideModel.get(i).uuid){
                    sideListView.currentIndex = i;
                    sideListView.currentItem.rename();
                    break;
                }
            }
        }
        function onSwitchToPreviousPlaylist(previousIndex){
            sideListView.currentIndex = previousIndex;
            sideListView.currentItem.switchToPrevious();
        }
    }

    DeleteSonglistDialog{
        id: removeSong;
        removeMusic: false
    }

    Component.onCompleted: {
        globalVariant.curListPageChanged.connect(function(){
            if (globalVariant !== undefined && globalVariant.curListPage === "search") {
                sideListView.currentItem.checked = false
            }
        })

//        musicBaseScrollView.contentItem.contentYChanged.connect(dropArea.updateHoverIndex())
    }

}
