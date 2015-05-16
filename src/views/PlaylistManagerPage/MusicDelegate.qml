import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import "../DMusicWidgets/PlaylistPage"

Rectangle {
    id: mediaItem

    property var mouseArea: mouseArea

    width: parent.width
    height: 24
    color: "transparent"
    Row {

        anchors.fill: parent
        spacing: 10

        Rectangle{
            id: tipRect
            width: 20
            height: 24

            color: "transparent"

            Text {
                id: indexTip
                anchors.centerIn: parent
                color: "#8a8a8a"
                font.pixelSize: 10
                text: index + 1
                visible: !waveBar.active
            }

            DPlayTipButton {
                id: playButton
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: false
            }

            DWaveBar {
                id: waveBar
                anchors.centerIn: parent
                itemHeight: 12
                itemWidth: 3
                active: {
                    var playlist = MediaPlayer.playlist;
                    if (playlist){
                        if (playlist.name == mediaItem.ListView.view.currentPlaylistName){
                            if (MediaPlayer.currentIndex == index && MediaPlayer.playing){
                                return true
                            }else{
                                return false
                            }
                        }
                        else{
                            return false
                        }
                    }
                    else{
                        return false
                    }
                }
            }
        }

        Row {
            height: 24
            spacing: 38

            Text {
                id: titleText
                width: 250
                height: 24
                color: waveBar.active? "#2ca7f8": "#3a3a3a"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: title
            }

            Text {
                id: artistText
                width: 156
                height: 24
                color: "#8a8a8a"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: artist
            }

            Text {
                id: durationText
                width: 50
                height: 24
                color: "#8a8a8a"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: UtilWorker.duration_to_string(duration)
            }
        }
    }

    states: [

        State {
            name: "Entered"
            PropertyChanges { target: mediaItem; color: "lightgray";}
            PropertyChanges { target: playButton; visible: !waveBar.active;}
            PropertyChanges { target: indexTip; visible: false;}
        },
        State {
            name: "Exited"
            PropertyChanges { target: mediaItem; color: "transparent";}
            PropertyChanges { target: playButton; visible: false;}
            PropertyChanges { target: indexTip; visible: !waveBar.active;}
        },

        State {
            name: "DoubleClicked"
            PropertyChanges { target: indexTip; visible: false ;}
            PropertyChanges { target: playButton; visible: !waveBar.active ;}
        },

        State {
            name: "current"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: indexTip; visible: false ;}
            PropertyChanges { target: playButton; visible: false ;}
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onEntered: {
            mediaItem.state = 'Entered';
        }
        onExited: {
            mediaItem.state = 'Exited';
        }
        onClicked:{
            if (mouse.button == Qt.RightButton){
                mediaItem.ListView.view.menuShowed(url);
            }
        }
        onDoubleClicked: {
            mediaItem.state = 'DoubleClicked';
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                mediaItem.ListView.view.playMusicByUrl(url);
            }
        }
    }
}