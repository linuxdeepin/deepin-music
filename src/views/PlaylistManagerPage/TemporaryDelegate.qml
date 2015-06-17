import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/PlaylistPage"

Rectangle{
    id: temporaryDelegate

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

            DTemporaryButton {
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
                        if (playlist.name == 'temporary'){
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
                text: I18nWorker.temporary
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
            PropertyChanges { target: temporaryDelegate; color: "transparent"}
            PropertyChanges { target: playlistNameText; color: "#868686"}
        },
        State {
            name: "Checked"
            PropertyChanges { target: temporaryDelegate; color: "#eeeeee"}
            PropertyChanges { target: playlistNameText; color: "black"}
        }
    ]

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        propagateComposedEvents: true
        onEntered: {
            if (temporaryDelegate.state != "Checked"){
                temporaryDelegate.color = "#f9f9f9";
            }
            
        }
        onExited: {
            if (temporaryDelegate.state != "Checked"){
                temporaryDelegate.color = 'transparent'
            }
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                if (temporaryDelegate.state == 'Active'){
                }else{
                    temporaryDelegate.state = "Checked";
                }
                temporaryDelegate.leftClicked();
            }else if (mouse.button == Qt.RightButton){
                temporaryDelegate.rightClicked();
            }
        }
    }
}