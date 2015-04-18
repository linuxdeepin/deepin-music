import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle{
    id: playlistNameDelegate

    property var mouseArea: mouseArea
    property var playlistName

    width: 120
    height: 16
    color: "transparent"

    function active() {
        waveBar.active = true
        playlistNameText.color = "#2ca7f8"
    }

    function normal() {
        waveBar.active = false
        playlistNameText.color = "#868686"
    }

    Row {
        spacing: 10

        Rectangle {
            width: 16
            height: 16

            color: "transparent"

            DPlaylistButton {
                id: tipButton
                anchors.fill: parent
                visible: !waveBar.active
            }

            DWaveBar {
                id: waveBar
                anchors.centerIn: parent
                itemHeight: 12
                itemWidth: 3
                active: false
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
                text: playlistNameDelegate.ListView.view.model[index].name
            }
        }
    }

    states: [
        State {
            name: "Active"
            PropertyChanges { target: playlistNameText; color: "#2ca7f8"}
            PropertyChanges { target: waveBar; active: true ;}
            PropertyChanges { target: tipButton; visible: false ;}
        },
        State {
            name: "!Checked"
            when: !playlistNameDelegate.ListView.isCurrentItem
            PropertyChanges {target: playlistNameDelegate; color: "transparent"}
            PropertyChanges { target: playlistNameText; color: "#868686"}
        },
        State {
            name: "Checked"
            PropertyChanges {target: playlistNameDelegate; color: "#eeeeee"}
            PropertyChanges { target: playlistNameText; color: "black"}
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        propagateComposedEvents: true
        onEntered: {
            if ( playlistNameDelegate.ListView.view.currentIndex != index){
                playlistNameDelegate.color = "#f9f9f9";
            }
        }
        onExited: {
            if ( playlistNameDelegate.ListView.view.currentIndex != index){
                playlistNameDelegate.color = "transparent";
            }else{
                playlistNameDelegate.color = "#eeeeee";
            }
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                playlistName = playlistNameText.text
                playlistNameDelegate.ListView.view.currentIndex = index;
                if (playlistNameDelegate.state == 'Active'){

                }else{
                    playlistNameDelegate.state = "Checked";
                }
                var name = playlistNameDelegate.ListView.view.model[index].name;
                playlistNameDelegate.ListView.view.itemClicked(name);
            }else if (mouse.button == Qt.RightButton){

            }
        }
    }
}