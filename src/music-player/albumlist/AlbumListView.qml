// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../allItems"
import "../musicmousemenu"

Rectangle {
    property double scalingRatio: 238 / 810   //计算宽度占比
    property ListModel albumModels
    signal itemDoubleClicked(var albumData)
    property Menu albumMoreMenu: AlbumMoreMenu{}
    id: rootRectangle
    color: Qt.rgba(0, 0, 0, 0)
    //标题栏
    Row {
        id: headerView
        width: rootRectangle.width; height: 36
        leftPadding: 20
        Rectangle {
            width: 26; height: 36
            color: "transparent"
        }
        Label {
            width: parent.width - 412 - 26 - 40
            height: 36
            leftPadding: 10
            text: qsTr("Album")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: 200; height: 36
            text: qsTr("Artist")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: 110; height: 36
            text: qsTr("Tracks")
            verticalAlignment: Qt.AlignVCenter
        }
        Label {
            width: 102; height: 36
            text: qsTr("Date added")
            verticalAlignment: Qt.AlignVCenter
        }
    }

    ListView{
        id: listview
        width: rootRectangle.width; height: rootRectangle.height - 36
        anchors.top: headerView.bottom
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}
        model: albumModels
        clip: true
        focus: true
        delegate: AlbumListDelegate{
            width: listview.width - 40; height: 56
            backgroundVisible: true
            normalBackgroundVisible: index % 2 === 0
            onDoubleClicked: {
                //console.log(" listview onItemDoubleClicked: ", albumModels.get(index));
                rootRectangle.itemDoubleClicked(albumModels.get(index));
            }
        }
    }
    Connections {
        target: albumMoreMenu
        onViewAlbumDatails:{ rootRectangle.itemDoubleClicked(albumData);}
    }

}
