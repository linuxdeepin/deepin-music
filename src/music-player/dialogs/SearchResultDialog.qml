// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0

Popup {
    property var songList: []
    property ListModel artistModel: ListModel{}
    property ListModel albumModel: ListModel{}
    property string pattern: ""

    property int itemHeight: 40

    signal searchItemTriggered(string value, int type)

    id: searchResultRect
    width: 360
    height: (songList == null ? 0 : songList.length + artistModel.count + albumModel.count) * itemHeight + 110
    leftPadding: 8
    rightPadding: 8
    topPadding: 8
    bottomPadding: 8

    Column {
        width: parent.width
        height:parent.height

        MenuSeparator {
            text: qsTr("Songs")
            anchors.left: parent.left
            anchors.leftMargin: 12
            font.pixelSize: 14
            font.bold: true
            visible: (songList == null || songList.length == 0) ? false : true
        }

        Repeater {
            model: songList
            delegate: resultItemTextDelegate
        }

        MenuSeparator {
            text: qsTr("Artists")
            anchors.left: parent.left
            anchors.leftMargin: 12
            font.pixelSize: 14
            font.bold: true
            visible: artistModel.count === 0 ? false : true
        }
        Repeater {
           model: artistModel
           delegate: resultItemImgDelegate
        }

        MenuSeparator {
            text: qsTr("Albums")
            anchors.left: parent.left
            anchors.leftMargin: 12
            font.pixelSize: 14
            font.bold: true
            visible: albumModel.count === 0 ? false : true
        }
        Repeater {
            model: albumModel
            delegate: resultItemImgDelegate
        }
    }

    Component {
        id: resultItemTextDelegate
        Rectangle {
            id: itemRect
            width: parent.width
            height: itemHeight
            radius: 6
            color: "#00000000"
            Row {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10
                leftPadding: 30
                Label {
                    width: parent.width - 30 * 2
                    anchors.verticalCenter: parent.verticalCenter
                    text: songList[index]
                    elide: Text.ElideMiddle
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    itemRect.color = palette.highlight
                }
                onExited: {
                    itemRect.color = "#00000000"
                }
                onClicked: {
                    searchItemTriggered(songList[index], 0)
                    searchResultRect.close()
                }
            }
        }
    }

    Component {
        id: resultItemImgDelegate
        Rectangle {
            id: itemRect
            width: parent.width
            height: itemHeight
            radius: 6
            color: "#00000000"
            Row {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10
                leftPadding: 30
                Image {
                    width: 34
                    height: 34
                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize: Qt.size(34, 34)
                    source: "file:///" + coverUrl
                    cache: false
                }
                Label {
                    width: parent.width - 44 - 30 * 2
                    anchors.verticalCenter: parent.verticalCenter
                    text: name
                    elide: Text.ElideMiddle
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    itemRect.color = palette.highlight
                }
                onExited: {
                    itemRect.color = "#00000000"
                }
                onClicked: {
                    var type = 0

                    if (model.type === "artist")
                        type = 1
                    else if (model.type === "album")
                        type = 2

                    searchItemTriggered(model.name, type)
                    searchResultRect.close()
                }
            }
        }
    }
}
