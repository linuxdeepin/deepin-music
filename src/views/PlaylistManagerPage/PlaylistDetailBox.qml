import QtQuick 2.3
import DMusic 1.0

Rectangle {
    id: playlistDetailBox

    property var playlistView: playlistView
    property var titleText: titleText
    property var noMusicTip: noMusicTip
    property var linkTipText: linkTipText
    property var songListModel: songListModel
    property var currentPlaylistName

    function getModelByPlaylistName(name){
        try {
            if (name){
                var model = eval('Playlist_' + Qt.md5(name));
                if (model){
                    return model
                }else{
                    return EmptyModel
                }
            }else{
                return EmptyModel
            }
        }catch(e){
            print(e)
            return EmptyModel
        }
    }

    Rectangle {
        id: detailBox
        anchors.fill: parent
        anchors.leftMargin: 25
        anchors.rightMargin: 35
        anchors.bottomMargin: 10

        Column {
            anchors.fill: detailBox

            spacing: 5

            Rectangle {
                id: header

                width: parent.width
                height: 24
                color: "#eeeeee"

                Row {

                    anchors.fill: parent
                    spacing: 10

                    Rectangle{
                        id: tipRect
                        width: 20
                        height: 24

                        color: "transparent"
                    }

                    Row {
                        height: 24
                        spacing: 38

                        Text {
                            id: titleText
                            width: 250
                            height: 24
                            color:  "#8a8a8a"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: I18nWorker.song + '   (' + playlistView.count +')'

                            Rectangle {
                                x: 270
                                y: 4
                                width: 1
                                height: parent.height - 8
                                color: "lightgray"
                            }
                        }

                        Text {
                            id: artistText
                            width: 156
                            height: 24
                            color: "#8a8a8a"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: I18nWorker.artist

                            Rectangle {
                                x: 174
                                y: 4
                                width: 1
                                height: parent.height - 8
                                color: "lightgray"
                            }
                        }

                        Text {
                            id: durationText
                            width: 100
                            height: 24
                            color: "#8a8a8a"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: I18nWorker.duration

                            
                        }
                    }
                }
            }
            Rectangle {
                width: parent.width
                height: detailBox.height - header.height - 29

                ListView {
                    id: playlistView

                    property var currentPlaylistName: playlistDetailBox.currentPlaylistName

                    anchors.fill: parent
                    clip: true
                    highlightMoveDuration: 1
                    delegate: MusicDelegate {}
                    model: songListModel
                    focus: true
                    spacing: 14
                    snapMode:ListView.SnapToItem

                    signal playMusicByUrl(string songUrl)
                    signal localMenuShowed(string songUrl)
                    signal onlineMenuShowed(string songUrl, int songId)

                    DScrollBar {
                        flickable: parent
                        inactiveColor: 'black'
                    }

                    Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Center)
                }

                DListModel {
                    id: songListModel
                    pymodel: getModelByPlaylistName(currentPlaylistName)
                }

                Rectangle {
                    id: noMusicTip
                    anchors.fill: parent
                    Text {
                        id: linkTipText
                        anchors.centerIn: parent
                        color: "#535353"
                        linkColor: "#31a4fa"
                        font.pixelSize: 14
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: "到 <a href=\"Online\" style=\"text-decoration:none;\">网络音乐</a> 和<a href=\"Local\" style=\"text-decoration:none;\">本地乐库</a>添加你喜欢的音乐吧"
                    }
                }
            }
        }
    }

    PlaylistDetailBoxController {
        id: playlistDetailBoxController
        playlistDetailBox: playlistDetailBox
        playlistView: playlistView
        noMusicTip: noMusicTip
        titleText: titleText
        linkTipText: linkTipText
    }
}