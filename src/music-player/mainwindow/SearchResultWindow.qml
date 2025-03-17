// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import audio.global 1.0

import "../musicsublist"
import "../musicList"
import "../singerlist"
import "../albumlist"
import "../musicmousemenu"
import "../allItems"

Rectangle {
    property string pattern: "a"
    property Menu artistMoreMenu: ArtistMoreMenu{}
    property Menu albumMoreMenu: AlbumMoreMenu{}
    property ListModel songsModel: ListModel{}
    property ListModel albumsModel: ListModel{}
    property ListModel artistsModel: ListModel{}
    property int curIndex: tabArea.currentIndex
    signal itemDoubleClicked(var artistData)
    property var artistData

    Component {
        id: artistSublistView
        ArtistSublistView {
            artistData:searchRootRect.artistData
        }
    }
    Component {
        id: albumSublistView
        AlbumSublistView {
            albumData:searchRootRect.artistData
        }
    }

    id: searchRootRect
    objectName: "search"
    color: "transparent"

    Rectangle {
        id: noResultView
        width: parent.width
        height: parent.height
        visible: curIndex < 0 ? true : false
        color: "transparent"

        Label {
            anchors.centerIn: parent
            text: qsTr("No search results")
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height
        color: "transparent"
        visible: curIndex < 0 ? false : true
        ColumnLayout {
            width: parent.width
            height: parent.height

            Rectangle {
                id: headArea
                width: parent.width - 40
                height: 72
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                color: "transparent"

                Row {
                    anchors.fill: parent
                    topPadding: 17
                    spacing: 10
                    DciIcon {
                        id: playall
                        name: "headline_play_bottom";
                        sourceSize: Qt.size(28, 28)
                        ActionButton {
                            anchors.fill: playall
                            icon.name: "list_play";
                            width: 20; height: 20
                            hoverEnabled: false;
                            onClicked: {
                                Presenter.playPlaylist("musicResult");
                            }
                        }
                    }
                    Label {
                        id: buttonLable
                        text: qsTr("Search Results")
                        font: DTK.fontManager.t5
                    }
                    Label {
                        font: DTK.fontManager.t8
                        text: albumsModel.count !== 1 ? qsTr("%1 albums - %2 songs").arg(albumsModel.count).arg(songsModel.count) :
                                                        (songsModel.count === 1 ? qsTr("1 album - 1 song") : qsTr("%1 album - %2 songs").arg(albumsModel.count).arg(songsModel.count))

                        anchors.verticalCenter: buttonLable.verticalCenter
                    }
                }
            }

            RowLayout {
                id: headerLayout
                Layout.rightMargin: 40
                TabBar {
                    id: tabArea
                    width: parent.width - 40
                    height: parent.height - headArea.height
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    currentIndex: 0

                    TabButton {
                        text: qsTr("Music")
                        width: 82
                    }
                    TabButton {
                        text: qsTr("Album")
                        width: 82
                    }
                    TabButton {
                        text: qsTr("Artist")
                        width: 82
                    }
                }
                Item { Layout.fillWidth: true }
                SortMenu {
                    id: allMusicSortMenu
                    visible: tabArea.currentIndex === 0
                    sortPageHash: "musicResult"
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    pageSortType: 0
                }
                SortMenu {
                    id: albumMusicSortMenu
                    visible: tabArea.currentIndex === 1
                    sortPageHash: "albumResult"
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    pageSortType: 0
                }
                SortMenu {
                    id: artistMusicSortMenu
                    visible: tabArea.currentIndex === 2
                    sortPageHash: "artistResult"
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    pageSortType: 0
                }

                Component.onCompleted: {
                    var musicSortType = Presenter.playlistSortType("musicResult")
                    var artistSortType = Presenter.playlistSortType("artistResult")
                    var albumSortType = Presenter.playlistSortType("albumResult")

                    switch(musicSortType) {
                    case 10:
                        allMusicSortMenu.pageSortType = 0
                        break
                    case 11:
                        allMusicSortMenu.pageSortType = 1
                        break
                    case 12:
                        allMusicSortMenu.pageSortType = 2
                        break
                    case 13:
                        allMusicSortMenu.pageSortType = 3
                        break
                    default:
                        allMusicSortMenu.pageSortType = -1
                    }

                    switch(albumSortType) {
                    case 10:
                        albumMusicSortMenu.pageSortType = 0
                        break
                    case 13:
                        albumMusicSortMenu.pageSortType = 1
                        break
                    default:
                        albumMusicSortMenu.pageSortType = -1
                    }

                    switch(artistSortType) {
                    case 10:
                        artistMusicSortMenu.pageSortType = 0
                        break
                    case 12:
                        artistMusicSortMenu.pageSortType = 2
                        break
                    default:
                        artistMusicSortMenu.pageSortType = -1
                    }
                }
            }

            StackLayout {
                id: myStackView
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                clip: true
                visible: curIndex < 0 ? false : true
                currentIndex: tabArea.currentIndex

                AllMusicListView {
                    id: allMusicList
                    width: parent.width
                    height: parent.height
                    mediaModel: songsModel
                    viewListHash: "musicResult"
                }
                GridView {
                    id: artistView
                    width: Math.floor(parent.width / 196) * 196;
                    height: parent.height
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top; anchors.topMargin: 34
                    anchors.leftMargin: (parent.width - width) / 2
                    anchors.rightMargin: (parent.width - width) / 2
                    cellWidth: 196; cellHeight: 196
                    ScrollBar.vertical: ScrollBar {}
                    clip: true
                    model: artistsModel
                    delegate: ArtistGridDelegate{
                        id: musicSingerGridItem
                        onItemDoubleClicked: {
                            searchRootRect.itemDoubleClicked(artistData);
                        }
                    }
                    onWidthChanged: {
                        var w =  Math.floor(tabArea.width / 196) * 196;
                        var m = (tabArea.width - w) / 2;
                        anchors.leftMargin = m;
                        anchors.rightMargin = m;
                    }
                }
                GridView {
                    id: albumGridView
                    width: Math.floor(parent.width / 201) * 201;
                    height: parent.height
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top; anchors.topMargin: 5
                    anchors.leftMargin: (parent.width - width) / 2
                    anchors.rightMargin: (parent.width - width) / 2
                    cellWidth: 201; cellHeight: 228
                    ScrollBar.vertical: ScrollBar {}
                    clip: true
                    model: albumsModel
                    delegate: AlbumGridDelegate{
                        id: itemDelegate
                        playing: (globalVariant.curPlayingStatus === DmGlobal.Playing) ? true : false
                        onItemDoubleClicked: {
                            console.log("onItemDoubleClicked: " + albumData);
                            searchRootRect.itemDoubleClicked(albumData);
                        }
                    }
                    onWidthChanged: {
                        var w =  Math.floor(tabArea.width / 201) * 201;
                        var m = (tabArea.width - w) / 2;
                        anchors.leftMargin = m;
                        anchors.rightMargin = m;
                    }
                }
            }
        }
    }

    onItemDoubleClicked: {
        searchRootRect.artistData = artistData
        globalVariant.globalSwitchButtonStatus = 2;
    }

    function updateSearchResultInfo(text, type) {
        songsModel.clear()
        albumsModel.clear()
        artistsModel.clear()

        var result = Presenter.searchText(text)
        var songs = result["metas"]
        var albums = result["albums"]
        var artists = result["artists"]

        for (var i = 0; i < songs.length; i++) {
            songs[i].inMulitSelect = false;
            songs[i].dragFlag = false;
            songsModel.append(songs[i])
        }

        for (var j = 0; j < albums.length; j++) {
            var albumData = albums[j]
            var curCount = 0;

            for (var key in albumData.musicinfos) {
                curCount++
                albumData.coverUrl = albumData.musicinfos[key].coverUrl
                albumData.musicCount = curCount;
            }
            albumsModel.append(albumData)
        }

        for (var k = 0; k < artists.length; k++) {
            var artistData = artists[k]
            var curCount = 0;

            for (var key in artistData.musicinfos) {
                curCount++
                artistData.coverUrl = artistData.musicinfos[key].coverUrl
                artistData.musicCount = curCount;
            }
            artistsModel.append(artistData)
        }

        curIndex = type
        tabArea.currentIndex = curIndex
        globalVariant.globalSwitchButtonStatus = 0
    }
    function onDeleteOneMeta(playlistHashs, hash){
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] == "all") {
                for (var j = 0; j < songsModel.count; j++) {
                    if (songsModel.get(j)["hash"] === hash) {
                        songsModel.remove(j)
                        break
                    }
                }
            } else if (playlistHashs[i] === "fav"){
                for(var k = 0; k < songsModel.count; k++){
                    if(hash == songsModel.get(k).hash){
                        songsModel.setProperty(k,"favourite",false)
                        break;
                    }
                }
            }
        }
    }
    function onAddOneMeta(playlistHashs, meta) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] == "fav") {
                for (var j = 0; j < songsModel.count; j++) {
                    if (songsModel.get(j)["hash"] == meta.hash) {
                        songsModel.setProperty(j,"favourite",true)
                        break
                    }
                }
            }
        }
    }
    function onUpdatedMetaCodec(meta, preAlbum, preArtist) {
        for (var j = 0; j < songsModel.count; j++) {
            if (songsModel.get(j)["hash"] === meta.hash) {
                songsModel.set(j, meta)
                break
            }
        }
    }
    function playlistSortChanged(playlistHash) {
        console.log("playlistSortChanged..................." + playlistHash)
        switch (playlistHash) {
        case "musicResult":
            songsModel.clear()
            var songs = Presenter.getPlaylistMetas("musicResult")
            for (var i = 0; i < songs.length; i++) {
                songs[i].inMulitSelect = false;
                songsModel.append(songs[i])
            }
            break
        case "artistResult":
            artistsModel.clear()
            var artists = Presenter.searchedArtistInfos()
            for (var k = 0; k < artists.length; k++) {
                var artistData = artists[k]
                var curCount = 0;

                for (var key in artistData.musicinfos) {
                    curCount++
                    artistData.coverUrl = artistData.musicinfos[key].coverUrl
                    artistData.musicCount = curCount;
                }
                artistsModel.append(artistData)
            }
            break
        case "albumResult":
            albumsModel.clear()
            var albums = Presenter.searchedAlbumInfos()
            for (var j = 0; j < albums.length; j++) {
                var albumData = albums[j]
                var curCount = 0;

                console.log("albumResult.........name:" + albumData.name + "    artist:" + albumData.artist)
                for (var key in albumData.musicinfos) {
                    curCount++
                    albumData.coverUrl = albumData.musicinfos[key].coverUrl
                    albumData.musicCount = curCount;
                }
                albumsModel.append(albumData)
            }
            break
        }
    }

    Connections {
        target: artistMoreMenu
        onViewArtistDatails: { itemDoubleClicked(artistData);}
    }
    Connections {
        target: albumMoreMenu
        onViewAlbumDatails: { itemDoubleClicked(artistData);}
    }
    Component.onCompleted: {
        updateSearchResultInfo(pattern, curIndex)
        Presenter.deleteOneMeta.connect(onDeleteOneMeta);
        Presenter.addOneMeta.connect(onAddOneMeta);
        Presenter.updatedMetaCodec.connect(onUpdatedMetaCodec)
        Presenter.playlistSortChanged.connect(playlistSortChanged)
    }
}
