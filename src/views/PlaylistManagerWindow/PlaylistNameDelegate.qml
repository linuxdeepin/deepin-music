import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle{
    id: playlistNameDelegate

    property var playlistName

    width: 120
    height: 16
    color: "transparent"
    Row {
        spacing: 10

        Rectangle {
            width: 16
            height: 16

            DPlaylistButton {
                id: tipButton
                anchors.fill: parent
            }

            DPlayButton {
                id: playButton
                anchors.fill: parent
                visible: false
            }

            DWaveBar {
                id: waveBar
                anchors.centerIn: parent
                itemHeight: 12
                itemWidth: 3
                active: {
                    if (MediaPlayer.state == 1){
                        return true;
                    }else{
                        return false;
                    }
                }
            }
        }

        Text {
            id: playlistNameText
            width: 94
            height: 16
            color: "#868686"
            font.pixelSize: 12
            elide: Text.ElideRight
            text: name
        }
    }

    states: [
        State {
            name: "Active"
            // when: playlistNameDelegate.ListView.isCurrentItem
            PropertyChanges { target: playlistNameText; color: "#2ca7f8"}
            PropertyChanges { target: waveBar; active: true ;}
            PropertyChanges { target: tipButton; visible: false ;}
            PropertyChanges { target: playButton; visible: false ;}
        },
        State {
            name: "!Active"
            when: !playlistNameDelegate.ListView.isCurrentItem
            PropertyChanges { target: playlistNameText; color: "#868686"}
            PropertyChanges { target: waveBar; active: false ;}
            PropertyChanges { target: tipButton; visible: true ;}
            PropertyChanges { target: playButton; visible: false ;}
        }
    ]

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        propagateComposedEvents: true
        onEntered: {
            if ( playlistNameDelegate.ListView.view.currentIndex != index){
                tipButton.visible = false
                playButton.visible = true;
            }
        }
        onExited: {
            if ( playlistNameDelegate.ListView.view.currentIndex != index){
                tipButton.visible = true
                playButton.visible = false;
            }
        }
        onClicked: {
            playlistName = playlistNameText.text
            playlistNameDelegate.ListView.view.currentIndex = index;
            playlistNameDelegate.state = "Active";
        }
    }
}