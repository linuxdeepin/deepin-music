// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../allItems"


ItemDelegate{
    id: itemDelegate
    anchors.horizontalCenter: parent.horizontalCenter
    hoverEnabled: true

    Component {
        id: hoverbuttons
        ActionButton {
            id: moreButton
            icon.name: itemDelegate.checked ? "list_more_checked" : "list_more"
            icon.width: 20; icon.height: 20
            onClicked: {
                albumMoreMenu.albumData = model
                albumMoreMenu.popup();
            }
        }
    }
    MouseArea {
        anchors.fill: itemDelegate;
        acceptedButtons: Qt.RightButton
        onClicked: {
            if(mouse.button === Qt.RightButton){
                albumMoreMenu.albumData = model
                albumMoreMenu.popup();
            }
        }
    }
    Rectangle {
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        radius: 8
        color: itemDelegate.hovered ? Qt.rgba(0, 0, 0, 0.08) : Qt.rgba(0, 0, 0, 0)
    Row {
        id: row
        anchors.fill: itemDelegate
//        anchors.centerIn: itemDelegate
        Label {
            id: numLable
            width: 26; height: 17
            anchors.verticalCenter: row.verticalCenter
            elide: Text.ElideRight
            text: (index+1 < 10) ? "0%1".arg(index + 1) : index+1
            horizontalAlignment: Qt.AlignHCenter
        }
        Rectangle {
            id: albumCell
            width: itemDelegate.width - 438; height: itemDelegate.height
            color: Qt.rgba(0,0,0,0)
            Row {
                spacing: 10
                leftPadding: 10
                anchors.fill: parent
                ImageCell {
                    id: imagecell
                    anchors.verticalCenter: parent.verticalCenter
                    source: "file:///" + coverUrl
                    pageHash: "album"
                    isCurPlay: (globalVariant.curPlayingAlbum === name) ? true : false
                    isCurHover: itemDelegate.hovered
                    curMediaData: model
                    width: 40; height: 40
                }
                Label {
                    id: albumNameLabel
                    width: albumCell.width - 110; height: 17
                    elide: Text.ElideRight
                    text: (name === "") ? "undefind": name
                    verticalAlignment: Qt.AlignVCenter
                    anchors.verticalCenter: parent.verticalCenter
                    palette.text: DTK.themeType === ApplicationHelper.DarkType ? "#B2F7F7F7" : "#000000"
                    color: checked ? palette.highlightedText :
                                     (imagecell.isCurPlay ? palette.highlight : palette.text)
                    font: DTK.fontManager.t7
                }
                Loader {
                    id: buttonsLoader
                    anchors.verticalCenter: parent.verticalCenter
                    sourceComponent: hoverbuttons
                    visible: itemDelegate.hovered
                }
            }
        }
        Label {
            id: artistLabel
            width: 200; height: itemDelegate.height
            elide: Text.ElideRight
            text: (artist === "") ? "undefind": artist
            verticalAlignment: Qt.AlignVCenter
            anchors.verticalCenter: row.verticalCenter
        }
        Label {
            id: musicNumbler
            width: 110; height: itemDelegate.height
            elide: Text.ElideRight
            text: qsTr("%1").arg(musicCount);
            verticalAlignment: Qt.AlignVCenter
            anchors.verticalCenter: artistLabel.verticalCenter
        }
        Label {
            id: dateTime
            width: 102; height: itemDelegate.height
            elide: Text.ElideRight
            text: new Date(timestamp/1000).toLocaleString(Qt.locale("de_DE"), "yyyy-MM-dd")
            verticalAlignment: Qt.AlignVCenter
            anchors.verticalCenter: musicNumbler.verticalCenter
        }
    }
    }

    onHoveredChanged: {
        imagecell.itemHoveredChanged(itemDelegate.hovered);
    }
}
