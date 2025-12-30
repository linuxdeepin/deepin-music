// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import audio.global 1.0
import org.deepin.dtk 1.0
import "../allItems"

ItemDelegate {
    property bool playing: (globalVariant.curPlayingStatus === DmGlobal.Playing) ? true : false
    property bool activeMeta:(globalVariant.curPlayingHash === hash) ? true : false
    property bool isDragged: false

    id: sublistDelegate
    hoverEnabled: true

    // 拖拽支持
    Drag.active: mouseArea.drag.active
    Drag.supportedActions: Qt.MoveAction
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "music-list/index-list": listview.delegateModelGroup
    }
    Drag.hotSpot.x: -15
    Drag.hotSpot.y: -15
    Drag.onActiveChanged: {
        console.warn("[AlbumSublistDelegate] Drag.active changed:", Drag.active, "index:", index)
    }
    Drag.onDragStarted: {
        console.warn("[AlbumSublistDelegate] Drag started, index:", index, "delegateModelGroup:", listview.delegateModelGroup)
    }
    Drag.onDragFinished: {
        console.warn("[AlbumSublistDelegate] Drag finished, index:", index)
        isDragged = true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: sublistDelegate
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        drag.target: sublistDelegate

        onDoubleClicked: {
            Presenter.playAlbum(album, hash);
        }
        onPressed: function(mouse) {
            if(mouse.button ===  Qt.LeftButton) {
                listview.forceActiveFocus();
                var inMulitSelect = mediaListModels.get(index).inMulitSelect;
                switch(mouse.modifiers){
                case Qt.ControlModifier:
                    mediaListModels.setProperty(index, "inMulitSelect", (!inMulitSelect));
                    listview.delegateModelGroup.push(index);
                    break;
                case Qt.ShiftModifier:
                    listview.checkMulti(index);
                    break;
                default:
                    if (!inMulitSelect)
                        listview.checkOne(index);
                    // 生成拖拽图像
                    dragDelegate.grabToImage(function(result) {
                        parent.Drag.imageSource = result.url
                    });
                    break;
                }
            } else if (mouse.button ===  Qt.RightButton) {
                if (listview.delegateModelGroup.length <= 1) {
                    musicMoreMenu.mediaData = model;
                    musicMoreMenu.itemIndex = index
                    musicMoreMenu.popup();
                } else {
                    selectMenu.musicHashList = listview.getSelectGroupHashList();
                    selectMenu.popup();
                }
            }
        }
        onReleased: function(mouse) {
            sublistDelegate.x = 0
            if ((mouse.modifiers !== Qt.ShiftModifier && mouse.modifiers !== Qt.ControlModifier)
                    && mouse.button === Qt.LeftButton && !isDragged) {
                listview.checkOne(index);
                isDragged = false
            }
            if (parent.Drag.supportedActions === Qt.MoveAction && isDragged) {
                if (listview.dragToIndex < 0)
                    sublistDelegate.y = 0 + listview.originY;
                else
                    sublistDelegate.y = listview.dragToIndex * 56 + listview.originY;

                if (index > listview.dragToIndex)
                    sublistDelegate.y = index * 56 + listview.originY;

                mediaListModels.setProperty(index, "dragFlag", false)
            }
            listview.dragToIndex = 0
        }
    }
    Component {
        id: hoverbuttons
        Row {
            spacing: 10
            ActionButton {
                id: addButton
                icon.name: sublistDelegate.checked ? "list_add_checked" : "list_add"
                icon.width: 20; icon.height: 20
                onClicked: {
                    var tmpHash = [];
                    tmpHash.push(model.hash);
                    importMenu.mediaHashList = tmpHash
                    importMenu.itemIndex = index
                    importMenu.popup();
                }
            }
            ActionButton {
                id: moreButton
                icon.name: sublistDelegate.checked ? "list_more_checked" : "list_more"
                icon.width: 20; icon.height: 20
                anchors.verticalCenter: addButton.verticalCenter
                onClicked: {
                    musicMoreMenu.mediaData = model;
                    musicMoreMenu.itemIndex = index
                    musicMoreMenu.popup();
                }
            }
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        radius: 8
        color: sublistDelegate.hovered ? Qt.rgba(0, 0, 0, 0.08) : Qt.rgba(0, 0, 0, 0)
        Row {
            Rectangle {
                id: columnNumber
                width: 56; height: 56
                color: Qt.rgba(0, 0, 0, 0)
                Row {
                    anchors.centerIn: parent
                    Rectangle {
                        width: 36
                        height: 56
                        color: Qt.rgba(0, 0, 0, 0)
                        Label {
                            id: numLable
                            anchors.verticalCenter: parent.verticalCenter
                            verticalAlignment: Qt.AlignVCenter
                            horizontalAlignment: Qt.AlignLeft
                            leftPadding: 10

                            elide: Text.ElideRight
                            visible: (!activeMeta) && !sublistDelegate.hovered
                            text: (index+1 < 10) ? "0%1".arg(index + 1) : index+1
                        }
                        ActionButton {
                            id: playActionButton
                            icon.name:  sublistDelegate.hovered ? (playing && activeMeta ? "list_pussed" : "list_play")
                                                                : (sublistDelegate.checked ? globalVariant.playingIconName : globalVariant.playingIconName + "_blue") //"list_playing"
                            icon.width: 20
                            icon.height: 20
                            anchors.centerIn: parent
                            visible: sublistDelegate.hovered || activeMeta
                            ColorSelector.hovered: !sublistDelegate.checked
                            ColorSelector.pressed: false
                            palette.windowText: sublistDelegate.hovered ? Qt.rgba(247, 247, 247, 1) : undefined
                            onClicked:{
                                if(playing && activeMeta) {
                                    Presenter.pause();
                                } else {
                                    Presenter.playAlbum(album, hash);
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: 20
                        height: 56
                        color: Qt.rgba(0, 0, 0, 0)
                        ActionButton {
                            id: heartbutton
                            anchors.verticalCenter: parent.verticalCenter
                            icon.width: 20; icon.height: 20
                            icon.name: model.favourite ? "heart_check" : "heart"
                            palette.windowText: (favourite & !sublistDelegate.checked)  ? "#F75B5B" : undefined
                            onClicked: {
                                if(favourite === false){
                                    Presenter.addMetasToPlayList(hash, "fav")
                                }else {
                                    Presenter.removeFromPlayList(hash, "fav")
                                    globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2);
                                }
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: columnMusic
                width: sublistDelegate.width - 342 - 56; height: 56
                color: Qt.rgba(0, 0, 0, 0)
                Row {
                    spacing: 10
                    leftPadding: 6
                    anchors.fill: columnMusic
                    Label {
                        id: musicNameLabel;
                        width: columnMusic.width - 76; height: 17
                        elide: Text.ElideRight
                        text: title
                        verticalAlignment: Qt.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        palette.text: DTK.themeType === ApplicationHelper.DarkType ? "#B2F7F7F7" : "#000000"
                        color: checked ? palette.highlightedText :
                                         (activeMeta ? palette.highlight : palette.text)
                        font: DTK.fontManager.t7

                    }
                    Loader {
                        id: buttonsLoader;
                        anchors.verticalCenter: musicNameLabel.verticalCenter
                        sourceComponent: hoverbuttons
                        visible: sublistDelegate.hovered || (importMenu.visible && importMenu.itemIndex === index) ||
                              (musicMoreMenu.visible && musicMoreMenu.itemIndex === index)
                    }

                }
            }
            Label {
                id: ablumLabel
                width: 200; height: 36
                leftPadding: 10
                elide: Text.ElideRight
                text: (artist === "") ? "undefind": artist
                verticalAlignment: Qt.AlignVCenter
                anchors.verticalCenter: columnMusic.verticalCenter
            }

            Label {
                id: musictimeLabel
                width: 142; height: 36
                elide: Text.ElideRight
                text:{
                    var sec = Math.floor((length/1000)%60);
                    if (sec < 10){
                        return Math.floor(length / 1000 / 60) + ":0" + sec
                    }else{
                        return Math.floor(length / 1000 / 60) + ":" + sec
                    }
                }
                verticalAlignment: Qt.AlignVCenter
                anchors.verticalCenter: ablumLabel.verticalCenter
            }
        }
    }

    // 拖拽指示线（显示在顶部）
    Rectangle {
        id: topDivider
        width: parent.width
        height: 1
        y: 0
        color: palette.highlight
        visible: index === 0 && listview.dragToIndex === -1
    }

    // 拖拽指示线（显示在底部）
    Rectangle {
        id: bottomDivider
        width: parent.width
        height: 1
        y: parent.height - 1
        color: palette.highlight
        visible: dragFlag
    }

    // 拖拽时显示的图像组件
    Rectangle {
        id: dragDelegate
        width: txt.width + 20
        height: txt.height + 10
        radius: 5
        color: palette.highlight
        visible: false

        Text {
            id: txt
            anchors.centerIn: parent
            font: DTK.fontManager.t8
            color: palette.highlightedText
            text: title
        }
    }
}
