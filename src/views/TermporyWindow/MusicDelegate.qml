import QtQuick 2.4
import DMuisc 1.0
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

        spacing: 16

        Rectangle {
            width: 24
            height: 24

            color: "transparent"

            DWaveBar {
                id: waveBar
                itemHeight: 16
                itemWidth: 3
                active: {
                    if (MediaPlayer.state == 1){
                        return true;
                    }else{
                        return false;
                    }
                }
            }

            DPlayButton {
                id: playButton
                anchors.fill: parent
                width: 24
                height: 24
                visible: false
            }

        }

        
        Text {
            id: musicText
            width: mediaItem.width - playButton.width - 16
            height: mediaItem.height
            verticalAlignment: Text.AlignVCenter
            text: mediaItem.ListView.view.model[index].title
            color: "#7d7d7d"
            font.pixelSize: 12
        }

    }


    states: [
        State {
            name: "Current"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: musicText; color: "#2ca7f8";}
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
                mediaItem.ListView.view.changeIndex(index);
                playButton.visible = false;
            }
            else if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }
    }
}