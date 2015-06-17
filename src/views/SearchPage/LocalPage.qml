import QtQuick 2.3
import QtQml.Models 2.1
import DMusic 1.0

Rectangle {
    id: localPage
    color: "transparent"



    ObjectModel {
        id: itemModel

        Rectangle {
            width: localPage.width
            height: 160
            SongListView {
                id: songListView
                datamodel: SearchLocalSongListModel
            }
        }

        Rectangle {
            id: artistBox
            width: localPage.width
            height: artistSection.height + 10 + artistTitle.height
            color: "transparent"
            visible: {
                if(artistSection.view.count > 0){
                    return true
                }else{
                    return false
                }
            }

            Column {
                anchors.fill: parent
                spacing: 10
                Rectangle {
                    id: artistTitle
                    width: 200
                    height: 20
                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 26
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 12
                        color: "#8a8a8a"
                        text: I18nWorker.artist + '  (' + artistSection.view.count + ')'
                    }
                }

                ArtistSection {
                    id: artistSection
                    width: parent.width
                }
            }
        }

        Rectangle {
            id: albumBox
            width: localPage.width
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

                AlbumSection{
                    id: albumSection
                    width: parent.width
                }
            }
        }
        Item {
            width: localPage.width
            height: 10
        }
    }

    DListPage {
        anchors.fill: parent
        model: itemModel
        spacing: 30
    }

    NoMusicTip {
        id: noMusicTip
        anchors.fill: parent

        visible:{
            if (songListView.view.count > 0 || artistSection.view.count > 0 ||albumSection.view.count > 0){
                return false;
            }else{
                return true;
            }
        }
    }

    LocalController {
        localSongsView: songListView.view
    }
}