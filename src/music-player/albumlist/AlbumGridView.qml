// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../allItems"
import "../musicsublist"
import "../musicmousemenu"
import audio.global 1.0

Rectangle {
    property ListModel albumModel
    signal itemDoubleClicked(var albumData)
    property alias view: albumGridView
    property Menu albumMoreMenu: AlbumMoreMenu{}
    id: albumGrid;
    color: Qt.rgba(0, 0, 0, 0)
    clip: true
    GridView {
        id: albumGridView
        width: Math.floor((albumGrid.width - 10) / 208) * 208;
        height: albumGrid.height
        anchors.horizontalCenter: albumGrid.horizontalCenter
        anchors.top: albumGrid.top;
        anchors.topMargin: 5
        anchors.leftMargin: (albumGrid.width - width) / 2
        anchors.rightMargin: (albumGrid.width - width) / 2
        cellWidth: 208; cellHeight: 242
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}
        model: albumModel
        delegate: AlbumGridDelegate{
            id: itemDelegate
            playing: (globalVariant.curPlayingStatus === DmGlobal.Playing) ? true : false
            onItemDoubleClicked: {
                albumGridView.currentIndex = index
                albumGrid.itemDoubleClicked(albumData);
            }
        }
        onWidthChanged: {
            var w =  Math.floor(albumGrid.width / 208) * 208
            var m = (albumGrid.width - w)
            anchors.leftMargin = m + 10;
            anchors.rightMargin = m;
        }
    }
    Connections {
        target: albumMoreMenu
        onViewAlbumDatails:{ albumGrid.itemDoubleClicked(albumData);}
    }
}
