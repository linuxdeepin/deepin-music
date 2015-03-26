import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle{
    id: starDelegate

    property var name: playlistName.text

    width: 120
    height: 16
    color: "transparent"

    signal clicked()

    Row {
        spacing: 10

        Rectangle {
            width: 16
            height: 16

            DStarButton {
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
            id: playlistName
            width: 94
            height: 16
            color: "#868686"
            font.pixelSize: 12
            elide: Text.ElideRight
            text: '我的收藏'
        }
    }

    states: [
        State {
            name: "Active"
            PropertyChanges { target: playlistName; color: "#2ca7f8"}
            PropertyChanges { target: waveBar; active: true ;}
            PropertyChanges { target: tipButton; visible: false ;}
            PropertyChanges { target: playButton; visible: false ;}
        },
        State {
            name: "!Active"
            PropertyChanges { target: playlistName; color: "#868686"}
            PropertyChanges { target: waveBar; active: false ;}
            PropertyChanges { target: tipButton; visible: true ;}
            PropertyChanges { target: playButton; visible: false ;}
        },
        State {
            name: "Checked"
            PropertyChanges { target: playlistName; color: "#2ca7f8"}
            PropertyChanges { target: waveBar; active: false ;}
            PropertyChanges { target: tipButton; visible: false ;}
            PropertyChanges { target: playButton; visible: true ;}
        }
    ]

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        propagateComposedEvents: true
        onEntered: {
            if (starDelegate.state != "Active"){
                tipButton.visible = false;
                playButton.visible = true;
            }else{
                tipButton.visible = false;
                playButton.visible = false;
            }
            
        }
        onExited: {
            if (starDelegate.state != "Active"){
                tipButton.visible = true;
                playButton.visible = false;
            }else{
                tipButton.visible = false;
                playButton.visible = false;
            }
        }
        onClicked: {
            starDelegate.state = "Checked";
            starDelegate.clicked()
        }
    }
}