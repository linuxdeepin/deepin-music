import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/PlaylistPage"

Rectangle{
    id: starDelegate

    property var name: playlistNameText.text

    width: 120
    height: 16
    color: "transparent"

    signal leftClicked()
    signal rightClicked()

    Row {
        spacing: 10

        Rectangle {
            width: 16
            height: 16

            color: "transparent"

            DStarButton {
                id: tipButton
                anchors.fill: parent
                visible: !waveBar.active
            }

            DWaveBar {
                id: waveBar
                anchors.centerIn: parent
                itemHeight: 12
                itemWidth: 3
                active: {
                    var playlist = MediaPlayer.playlist;
                    if (playlist){
                        if (playlist.name == 'favorite'){
                            if (MediaPlayer.playing){
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

        Rectangle {
            id: textBox
            width: 94
            height: 16

            color: "transparent"
            Text {
                id: playlistNameText
                anchors.fill: parent
                color: "#868686"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: I18nWorker.favorite
            }
        }
    }

    states: [
        State {
            name: "Active"
            when: waveBar.active
            PropertyChanges { target: playlistNameText; color: "#2ca7f8"}
        },
        State {
            name: "!Checked"
            PropertyChanges { target: starDelegate; color: "transparent"}
            PropertyChanges { target: playlistNameText; color: "#868686"}
        },
        State {
            name: "Checked"
            PropertyChanges { target: starDelegate; color: "#eeeeee"}
            PropertyChanges { target: playlistNameText; color: "black"}
        }
    ]

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        propagateComposedEvents: true
        onEntered: {
            if (starDelegate.state != "Checked"){
                starDelegate.color = "#f9f9f9";
            }
        }
        onExited: {
            if (starDelegate.state != "Checked"){
                starDelegate.color = 'transparent'
            }
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                if (starDelegate.state == 'Active'){

                }else{
                    starDelegate.state = "Checked";
                }
                starDelegate.leftClicked();
            }else if (mouse.button == Qt.RightButton){
                starDelegate.rightClicked();
            }
        }
    }
}