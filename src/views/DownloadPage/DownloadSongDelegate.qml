import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: mediaItem

    property var mouseArea: mouseArea

    width: parent.width
    height: 28
    color: "transparent"

    signal menuShowed(int index)

    Row {

        anchors.fill: parent
        spacing: 0

        Rectangle{
            id: tipRect
            width: 24
            height: mediaItem.height

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
            height: mediaItem.height
            spacing: 20
            Rectangle{
                width: 266
                height: mediaItem.height
                color: "transparent"
                Text {
                    id: titleText
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: "#3a3a3a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: name
                }
            }

            Rectangle{
                width: 160
                height: mediaItem.height
                color: "transparent"
                Text {
                    id: artistText
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: "#8a8a8a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: singerName
                }
            }
            
            Rectangle{
                width: 134
                height: mediaItem.height
                color: "transparent"
                Text {
                    id: sizeText
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    color: "#8a8a8a"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: UtilWorker.size_to_string(size)
                }
            }
            
            Rectangle{
                width: 114
                height: mediaItem.height
                color: "transparent"

                DCircleProgressBar {
                    x: 12
                    width: mediaItem.height
                    height: mediaItem.height
                    color: "transparent"
                    dprogress: progress
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
                var url = mediaItem.ListView.view.model.get(index).url
                mediaItem.ListView.view.playMusicByUrl(url);
                waveBar.active = true
            }
            else if (mouse.button == Qt.RightButton){
                mediaItem.menuShowed(index);
            }
        }
    }
}