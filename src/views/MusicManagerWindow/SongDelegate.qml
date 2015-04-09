import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: mediaItem

    property var mouseArea: mouseArea

    width: parent.width
    height: 24
    color: "transparent"

    signal menuShowed(int index)

    Row {

        anchors.fill: parent
        spacing: 0

        Rectangle{
            id: tipRect
            width: 24
            height: 24

            color: "transparent"

            Text {
                id: indexTip
                anchors.centerIn: parent
                color: "#8a8a8a"
                font.pixelSize: 10
                text: index + 1
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

        Row {
            height: 24
            spacing: 20
            Rectangle{
                width: 238
                height: 24
                color: "transparent"
                Text {
                    id: titleTetx
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    color: "#3a3a3a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: mediaItem.ListView.view.model[index].title
                }
            }

            Rectangle{
                width: 160
                height: 24
                color: "transparent"
                Text {
                    id: artistTetx
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: "#8a8a8a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: mediaItem.ListView.view.model[index].artist
                }
            }
            
            Rectangle{
                width: 160
                height: 24
                color: "transparent"
                Text {
                    id: albumTetx
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: "#8a8a8a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: mediaItem.ListView.view.model[index].album
                }
            }
            
            Rectangle{
                width: 58
                height: 24
                color: "transparent"
                Text {
                    id: durationTetx
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: "#8a8a8a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: UtilWorker.duration_to_string(mediaItem.ListView.view.model[index].duration)
                }
            }
        }
    }

    states: [
        State {
            name: "Active"
            PropertyChanges { target: titleTetx; color: "#2ca7f8"}
            PropertyChanges { target: waveBar; active: true ;}
        },
        State {
            name: "Current"
            when: mediaItem.ListView.isCurrentItem
            PropertyChanges { target: titleTetx; color: "#2ca7f8";}
        },
        State {
            name: "!Current"
            when: !mediaItem.ListView.isCurrentItem
            PropertyChanges { target: waveBar; active: false ;}
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onEntered: {
            mediaItem.color = "lightgray"
        }
        onExited: {
            mediaItem.color = "transparent"
        }
        onClicked: {
            if (mouse.button == Qt.LeftButton){
                mediaItem.ListView.view.currentIndex = index;
                var url = mediaItem.ListView.view.model[index].url
                mediaItem.ListView.view.playMusicByUrl(url);
                waveBar.active = true
            }
            else if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }
    }
}