import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: mediaItem

    // property var playButton: playButton

    width: parent.width
    height: 24
    color: "transparent"

    signal menuShowed(int index)

    Row {

        anchors.fill: parent
        spacing: 10

        Rectangle{
            id: tipRect
            width: 20
            height: 24

            Text {
                anchors.centerIn: parent
                text: index
            }
        }

        Row {
            height: 24
            spacing: 38

            Text {
                id: titleTetx
                width: 250
                height: 24
                color: "#3a3a3a"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model[index].title
            }

            Text {
                id: artistText
                width: 156
                height: 24
                color: "#8a8a8a"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model[index].artist
            }

            Text {
                id: durationText
                width: 50
                height: 24
                color: "#8a8a8a"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: mediaItem.ListView.view.model[index].duration
            }
        }
    }

    states: [
        State {
            name: "Current"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: titleTetx; color: "#2ca7f8";}
        },
        State {
            name: "!Current"
            when: !mediaItem.ListView.isCurrentItem
            // PropertyChanges { target: waveBar; active: false ;}
        }
    ]

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onEntered: {
            if (mediaItem.ListView.view.currentIndex != index){
                // playButton.visible = true;
                // mediaItem.color = "lightgray"
            }else{
                // playButton.visible = ! waveBar.visible;
                // mediaItem.color = "transparent"
            }
        }
        onExited: {
            // playButton.visible = false
            // mediaItem.color = "transparent"
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                // if (playButton.visible){
                    // var url = mediaItem.ListView.view.model[index].url
                    // mediaItem.ListView.view.playMusicByUrl(url);
                // }
                // playButton.visible = false;
            }
            else if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }
    }
}