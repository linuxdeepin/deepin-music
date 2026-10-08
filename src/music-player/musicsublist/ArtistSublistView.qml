// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.0
import "../musicList"

Rectangle {
    id: rootrectangle

    property var artistData
    property ListModel mediaListModels: MusicSublistModel {
        meidaDataMap: artistData.musicinfos
    }

    objectName: "artistSublist"
    color: "transparent"

    MusicSublistTitle {
        id: musicSublistTitle
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        titleWidth: rootrectangle.width
        titleHeight: 244
        currentData: rootrectangle.artistData
        pageHash: "artist"
    }

    AllMusicListView {
        id: artistMusicList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: musicSublistTitle.bottom
        anchors.bottom: parent.bottom
        mediaModel: rootrectangle.mediaListModels
        viewListHash: "artist"
        headerHeight: 56
        showTrackNumber: false
        showArtistColumn: false
        playbackByArtist: true

        onScrollStateChanged: function(scrolled) {
            musicSublistTitle.titleHeight = scrolled ? 80 : 244
            musicSublistTitle.suspensionTitle(scrolled)
        }
    }

    onArtistDataChanged: {
        if (artistData && artistData.musicinfos !== undefined)
            mediaListModels.meidaDataMap = artistData.musicinfos
    }
}
