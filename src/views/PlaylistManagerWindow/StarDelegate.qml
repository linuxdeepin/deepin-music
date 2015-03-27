import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle{
    id: starDelegate

    property var name: playlistNameText.text

    width: 120
    height: 16
    color: "transparent"

    signal clicked()

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
                text: I18nWorker.favorite
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
            PropertyChanges { target: starDelegate; color: "transparent"}
            PropertyChanges { target: playlistNameText; color: "#868686"}
            PropertyChanges { target: waveBar; active: false ;}
            PropertyChanges { target: tipButton; visible: true ;}
        },
        State {
            name: "Checked"
            PropertyChanges { target: starDelegate; color: "#eeeeee"}
            PropertyChanges { target: playlistNameText; color: "black"}
            PropertyChanges { target: waveBar; active: false ;}
            PropertyChanges { target: tipButton; visible: true ;}
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
            if (starDelegate.state == 'Active'){

            }else{
                starDelegate.state = "Checked";
            }
            starDelegate.clicked()
        }
    }
}