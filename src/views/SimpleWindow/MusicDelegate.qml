import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: mediaItem

    property var playButton: playButton

    width: parent.width
    height: 44
    color: "transparent"

    signal menuShowed(int index)

    Row {

        anchors.centerIn: parent

        Rectangle {
            width: 40
            height: 40

            color: "transparent"

            DWaveBar {
                id: waveBar
                itemHeight: 30
                itemWidth: 5
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
                width: 40
                height: 40
                visible: false
            }

        }

        Column {
            Text {
                id: musicText
                width: mediaItem.width - playButton.width
                height: mediaItem.height / 2
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model[index].title
                color: "#131313"
                font.pixelSize: 14
            }

            Text {
                id: artistText
                width: mediaItem.width - playButton.width
                height: mediaItem.height / 2
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model[index].artist
                color: "#919191"
                font.pixelSize: 13
            }
        }
    }


    states: [
        State {
            name: "Current"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: musicText; color: "#4ba3fb" ; font.pixelSize: 16}
            PropertyChanges { target: artistText; color: "#4ba3fb" ; font.pixelSize: 14}
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
            }else{
                playButton.visible = ! waveBar.visible;
            }
        }
        onExited: {
            playButton.visible = false
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                // mediaItem.ListView.view.currentIndex = index;
                mediaItem.ListView.view.changeIndex(index);
                playButton.visible = false;
            }
            else if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }
    }
}