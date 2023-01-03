import QtQuick 2.11
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
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
    property int curIndex: 0
    property Menu artistMoreMenu: ArtistMoreMenu{}
    property Menu albumMoreMenu: AlbumMoreMenu{}
    property ListModel songsModel: ListModel{}
    property ListModel albumsModel: ListModel{}
    property ListModel artistsModel: ListModel{}
    property int switchType: globalVariant.globalSwitchButtonStatus;
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

    StackView {
        id: myStackView
        width: parent.width
        height: parent.height
        clip: true
        visible: curIndex < 0 ? false : true
        initialItem: Rectangle {
            width: parent.width
            height: parent.height
            color: "transparent"
            Column {
                width: parent.width
                height: parent.height
                leftPadding: 20
                rightPadding: 20

                Rectangle {
                    id: headArea
                    width: parent.width - 40
                    height: 72
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

                TabView {
                    id: tabArea
                    width: parent.width - 40
                    height: parent.height - headArea.height
                    currentIndex: curIndex

                    Tab {
                        id: musicTab
                        title: qsTr("Music")
                        AllMusicListView {
                            width: tabArea.width
                            height: tabArea.height
                            mediaModel: songsModel
                            viewListHash: "musicResult"
                        }
                    }
                    Tab {
                        id: albumTab
                        title: qsTr("Album")
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
                        /*AlbumGridView {
                            albumModel: albumsModel
                        }*/
                    }
                    Tab {
                        id: artistTab
                        title: qsTr("Artist")
                        width: parent.width
                        height: parent.height
                        GridView {
                            id: singergridview
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
                    }
                    style: TabViewStyle {
                        frameOverlap: -10
                        tab: Rectangle {
                            implicitWidth: 82
                            implicitHeight: 36
                            color: "transparent"
                            Rectangle {
                                width: 76
                                height: 30
                                color: styleData.selected ? palette.highlight : "transparent"
                                anchors.centerIn: parent
                                radius: 5
                                Text {
                                    id: text
                                    anchors.centerIn: parent
                                    text: styleData.title
                                    color: styleData.selected ? "white" : "black"
                                }
                            }

                        }
                        tabBar: Rectangle {
                            implicitWidth: 246
                            implicitHeight: 46
                            color: "transparent"

                            /*FloatingPanel*/Rectangle {
                                width: 245
                                height: 36
                                radius: 5
                                color:  Qt.rgba(0, 0, 0, 0.05)
                            }

                            SortMenu{
                                id: musicSortMenu
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                visible: tabArea.currentIndex === 0 ? true : false
                                sortPageHash: "musicResult"
//                                pageSortType: 0
                            }
                            SortMenu{
                                id: albumSortMenu
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                visible: tabArea.currentIndex === 1 ? true : false
                                sortPageHash: "albumResult"
//                                pageSortType: 0
                            }
                            SortMenu{
                                id: artistSortMenu
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                visible: tabArea.currentIndex === 2 ? true : false
                                sortPageHash: "artistResult"
//                                pageSortType: 0
                            }

                            Component.onCompleted: {
                                var musicSortType = Presenter.playlistSortType("musicResult")
                                var artistSortType = Presenter.playlistSortType("artistResult")
                                var albumSortType = Presenter.playlistSortType("albumResult")

                                //console.log("search window onCompleted:musicSortType:" + musicSortType + "    artistSortType:" + artistSortType + "   albumSortType:" + albumSortType)

                                switch(musicSortType) {
                                case 10:
                                    musicSortMenu.pageSortType = 0
                                    break
                                case 11:
                                    musicSortMenu.pageSortType = 1
                                    break
                                case 12:
                                    musicSortMenu.pageSortType = 2
                                    break
                                case 13:
                                    musicSortMenu.pageSortType = 3
                                    break
                                default:
                                    musicSortMenu.pageSortType = -1
                                }

                                switch(albumSortType) {
                                case 10:
                                    albumSortMenu.pageSortType = 0
                                    break
                                case 13:
                                    albumSortMenu.pageSortType = 1
                                    break
                                default:
                                    albumSortMenu.pageSortType = -1
                                }

                                switch(albumSortType) {
                                case 10:
                                    artistSortMenu.pageSortType = 0
                                    break
                                case 12:
                                    artistSortMenu.pageSortType = 2
                                    break
                                default:
                                    artistSortMenu.pageSortType = -1
                                }
                            }
                        }
                        frame: Rectangle { color: "transparent" }
                    }
                }
            }
        }
    }

    onItemDoubleClicked: {
        searchRootRect.artistData = artistData
        globalVariant.globalSwitchButtonStatus = 2;
    }
    onSwitchTypeChanged: {
        if (switchType === 1) {
            myStackView.pop(myStackView.initialItem); //回到上一级页面
        } else if (switchType === 2){
            if (tabArea.currentIndex === 1) {
                var item = myStackView.find(function(item, index) { return item.objectName === "artistSublist"})
                if(item !== null){
                    myStackView.pop(item);
                    return;
                }
                myStackView.push(artistSublistView); //切换到下一级页面
            } else if (tabArea.currentIndex === 2) {
                var item = myStackView.find(function(item, index) { return item.objectName === "albumSublist"})
                if(item !== null){
                    myStackView.pop(item);
                    return;
                }
                myStackView.push(albumSublistView); //切换到下一级页面

            }
        }
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
        myStackView.pop(myStackView.initialItem)
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
