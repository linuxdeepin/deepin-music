// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../allItems"

ItemDelegate {
    property var m_mediaMeta
    id: sublistDelegate
    MouseArea {
        anchors.fill: sublistDelegate
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        onDoubleClicked: {
            Presenter.playArtist(artist, hash);
            imagecell.setplayActionButtonIcon("list_pussed");
        }
        onClicked: {
            if(mouse.button ===  Qt.LeftButton){
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
                    musicMoreMenu.mediaData = model
                    musicMoreMenu.itemIndex = index
                    musicMoreMenu.popup()
                } else {
                    selectMenu.musicHashList = listview.getSelectGroupHashList()
                    selectMenu.popup()
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
                    var tmpHash = []
                    tmpHash.push(model.hash)
                    importMenu.mediaHashList = tmpHash;
                    importMenu.itemIndex = index
                    importMenu.popup()
                }
            }
            ActionButton {
                id: moreButton
                icon.name: sublistDelegate.checked ? "list_more_checked" : "list_more"
                icon.width: 20; icon.height: 20
                anchors.verticalCenter: addButton.verticalCenter
                onClicked: {
                    musicMoreMenu.mediaData = model
                    musicMoreMenu.itemIndex = index
                    musicMoreMenu.popup()
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
                width: 26
                height: 56
                color: Qt.rgba(0, 0, 0, 0)
                ActionButton {
                    id: heartbutton
                    anchors.left: parent.left; anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    icon.width: 20; icon.height: 20
                    icon.name: favourite ? "heart_check" : "heart"
                    palette.windowText: (favourite & !sublistDelegate.checked)  ? "#F75B5B" : undefined
                    onClicked: {
                        if(favourite === false) {
                            Presenter.addMetasToPlayList(hash, "fav")
                        } else {
                            Presenter.removeFromPlayList(hash, "fav")
                            globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2);
                        }
                    }
                }
            }
            Rectangle {
                id: columnMusic
                width: sublistDelegate.width - 368; height: 56
                color: Qt.rgba(0, 0, 0, 0)
                ImageCell {
                    id: imagecell
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    source: "file:///" + coverUrl
                    pageHash: "artistSublist"
                    isCurPlay: (globalVariant.curPlayingHash === hash) ? true : false
                    isCurHover: sublistDelegate.hovered
                    curMediaData: model
                    width: 40; height: 40
                }
                Label {
                    id: musicNameLabel;
                    anchors.left: imagecell.right; anchors.leftMargin: 10
                    width: columnMusic.width - 150; height: 17
                    elide: Text.ElideRight
                    text: title
                    verticalAlignment: Qt.AlignVCenter
                    anchors.verticalCenter: imagecell.verticalCenter
                    palette.text: DTK.themeType === ApplicationHelper.DarkType ? "#B2F7F7F7" : "#000000"
                    color: checked ? palette.highlightedText :
                                     (imagecell.isCurPlay ? palette.highlight : palette.text)
                    font: DTK.fontManager.t7
                }
                Loader {
                    id: buttonsLoader;
                    anchors.right: columnMusic.right; anchors.rightMargin: 10
                    anchors.verticalCenter: musicNameLabel.verticalCenter
                    sourceComponent: hoverbuttons
                    visible: sublistDelegate.hovered || (importMenu.visible && importMenu.itemIndex === index) ||
                             (musicMoreMenu.visible && musicMoreMenu.itemIndex === index)
                }
            }
            Label {
                id: ablumLabel
                width: 200; height: 36
                elide: Text.ElideRight
                leftPadding: 10
                text: (album === "") ? "undefind": album
                verticalAlignment: Qt.AlignVCenter
                anchors.verticalCenter: columnMusic.verticalCenter
            }
            Label {
                id: musictimeLabel
                width: 142; height: 36
                elide: Text.ElideRight
                text:{
                    var sec = Math.floor((length/1000)%60);
                    if (sec < 10) {
                        return Math.floor(length / 1000 / 60) + ":0" + sec
                    } else {
                        return Math.floor(length / 1000 / 60) + ":" + sec
                    }
                }
                verticalAlignment: Qt.AlignVCenter
                anchors.verticalCenter: ablumLabel.verticalCenter
            }

        }
    }
    hoverEnabled: true
    onHoveredChanged: {
        imagecell.itemHoveredChanged(sublistDelegate.hovered);
    }
}
