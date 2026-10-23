// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import "../musicList"

Rectangle {
    id: rootrectangle

    property var albumData
    property ListModel mediaListModels: MusicSublistModel {
        meidaDataMap: albumData.musicinfos
    }

    objectName: "albumSublist"
    color: "transparent"

    MusicSublistTitle {
        id: musicSublistTitle
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        titleWidth: rootrectangle.width
        titleHeight: 244
        currentData: rootrectangle.albumData
        pageHash: "album"
        Accessible.name: "AlbumSublistView_MusicSublistTitle"
        Accessible.role: Accessible.Pane
    }

    AllMusicListView {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: musicSublistTitle.bottom
        anchors.bottom: parent.bottom
        mediaModel: rootrectangle.mediaListModels
        viewListHash: "album"
        headerHeight: 48
        playbackAlbumName: rootrectangle.albumData ? rootrectangle.albumData.name : ""
        showCoverImage: false
        indexPlaybackState: true
        showAlbumColumn: false
        acceptExternalDrops: true
        allowLocalReorder: true
        Accessible.name: "AlbumSublistView_AllMusicListView"
        Accessible.role: Accessible.Pane

        onScrollStateChanged: function(scrolled) {
            musicSublistTitle.titleHeight = scrolled ? 80 : 244
            musicSublistTitle.suspensionTitle(scrolled)
        }
    }

    onAlbumDataChanged: {
        if (albumData && albumData.musicinfos !== undefined)
            mediaListModels.meidaDataMap = albumData.musicinfos
    }
}
