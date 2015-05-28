import QtQuick 2.3
import DMusic 1.0

Rectangle {
    color: "transparent"

    Column {
        anchors.fill: parent
        spacing: 20
        Rectangle {
            width: parent.width
            height: 160
            SongListView {
                id: songListView
                datamodel: SearchLocalSongListModel
            }
        }

        Rectangle {
            id: artistBox
            width: parent.width
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
                    height:108
                }
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

                AlbumSection{
                    id: albumSection
                    width: parent.width
                    height: 108
                }
            }
        }
    }

    Rectangle {
        id: noMusicTip
        anchors.fill: parent

        visible:{
            if (songListView.view.count > 0 || artistSection.view.count > 0 ||albumSection.view.count > 0){
                return false;
            }else{
                return true;
            }
        }

        Rectangle {
           anchors.topMargin: 160
           anchors.fill: parent
           Column {
                spacing: 5
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    id: linkTipText
                    color: "#535353"
                    linkColor: "#31a4fa"
                    font.pixelSize: 14
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: "没有找到与\" <a href=\"Online\" style=\"text-decoration:none;\">" + SearchWorker.keyword + "</a>\" 相关的音乐"
                }

                Text {
                    id: linkTipText1
                    color: "#535353"
                    linkColor: "#31a4fa"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: "1. 检查输入的关键字是否正确"
                }

                Text {
                    id: linkTipText2
                    color: "#535353"
                    linkColor: "#31a4fa"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: "2. 缩短关键字长度"
                }
            }
        }
    }

    LocalController {
        localSongsView: songListView.view
    }
}