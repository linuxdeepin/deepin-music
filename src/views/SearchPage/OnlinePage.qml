import QtQuick 2.3
import DMusic 1.0

Rectangle {
    color: "transparent"

    Column {
        anchors.fill: parent
        spacing: 20
        Rectangle {
            width: parent.width
            height: 290
            SongListView {
                id: songListView
                isLocal: false
                datamodel: SearchOnlineSongListModel
            }
        }

        Rectangle {
            id: albumBox
            width: parent.width
            height: albumSection.height + 10 + albumTitle.height
            color: "transparent"
            visible: {
                if(albumSection.view.count > 0){
                    return true
                }else{
                    return false
                }
            }
            Column {
                anchors.fill: parent
                spacing:10
                Rectangle {
                    id: albumTitle
                    width: 200
                    height: 20
                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 26
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 12
                        color: "#8a8a8a"
                        text: I18nWorker.album + '  (' + albumSection.view.count + ')'
                    }
                }

                OnlineAlbumSection{
                    id: albumSection
                    width: parent.width
                    height: 108
                }
            }
        }
    }

    NoMusicTip {
        id: noMusicTip
        anchors.fill: parent
        topMargin: 122
        visible:{
            if (songListView.view.count > 0 ||albumSection.view.count > 0){
                return false;
            }else{
                return true;
            }
        }
    }


    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 45
        visible: noMusicTip.visible
        spacing:10
        Rectangle {
            id: playlistTitle
            width: 200
            height: 20
            Text {
                anchors.fill: parent
                anchors.leftMargin: 26
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 12
                color: "#8a8a8a"
                text: I18nWorker.suggestPlaylist
            }
        }
            
        OnlinePlaylistSection{
            id: playlistSection
            height: 130
            width: parent.width
        }
    }


    OnlineController {
        onlineSongsView: songListView.view
        albumSection: albumSection
        playlistSection: playlistSection
    }
}