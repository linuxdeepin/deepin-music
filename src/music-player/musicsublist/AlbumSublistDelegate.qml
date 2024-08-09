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

    id: sublistDelegate
    hoverEnabled: true
    MouseArea {
        id: mouseArea
        anchors.fill: sublistDelegate
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        onDoubleClicked: {
            Presenter.playAlbum(album, hash);
        }
        onClicked: {
            if(mouse.button ===  Qt.LeftButton) {
                listview.forceActiveFocus();
                switch(mouse.modifiers){
                case Qt.ControlModifier:
                    var inMulitSelect = mediaListModels.get(index).inMulitSelect;
                    mediaListModels.setProperty(index, "inMulitSelect", (!inMulitSelect));
                    listview.delegateModelGroup.push(index);
                    break;
                case Qt.ShiftModifier:
                    listview.checkMulti(index);
                    break;
                default:
                    listview.checkOne(index);
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
}
