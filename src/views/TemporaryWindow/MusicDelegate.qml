import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: mediaItem

    property var playButton: playButton

    width: parent.width
    height: 24
    color: "transparent"

    signal menuShowed(int index)

    Row {

        anchors.fill: parent

        spacing: 0

        Rectangle {
            width: 28
            height: 24

            color: "transparent"

            DWaveBar {
                id: waveBar
                itemHeight: 16
                itemWidth: 3
                active: false
            }

            DPlayButton {
                id: playButton
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: false
            }
        }

        
        Row {
            spacing: 16

            Image {
                width: 24
                height: 24
                asynchronous: true
                source: cover
            }

            Text {
                id: musicText
                width: 164
                height: mediaItem.height
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: title
                color: "#a0a0a0"
                font.pixelSize: 11
            }

            Text {
                id: durationText
                width: 34
                height: mediaItem.height
                verticalAlignment: Text.AlignVCenter
                text: UtilWorker.duration_to_string(duration)
                color: "#a0a0a0"
                font.pixelSize: 11
            }

            Rectangle{
                width: 4
                height: mediaItem.height
            }
        }
    }


    states: [
        State {
            name: "Current"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: musicText; color: "#2ca7f8";}
            PropertyChanges { target: durationText; color: "#2ca7f8";}
            PropertyChanges { target: waveBar; active: true ;}
        },
        State {
            name: "!Current"
            when: !mediaItem.ListView.isCurrentItem
            PropertyChanges { target: waveBar; active: false ;}
        }
    ]

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onEntered: {
            if (mediaItem.ListView.view.currentIndex != index){
                playButton.visible = true;
                mediaItem.color = "lightgray"
            }else{
                playButton.visible = ! waveBar.visible;
                mediaItem.color = "transparent"
            }
        }
        onExited: {
            playButton.visible = false
            mediaItem.color = "transparent"
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                if (playButton.visible){
                    mediaItem.ListView.view.playMusicByUrl(url);
                }
                playButton.visible = false;
            }
            else if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }
    }
}